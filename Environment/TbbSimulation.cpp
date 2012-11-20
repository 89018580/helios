/*
Copyright (c) 2012, Esteban Pellegrino
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#include "Simulation.hpp"

using namespace std;
using namespace Helios;
using namespace IntelTbb;


KeffSimulation::SourceSimulator::SourceSimulator(const McEnvironment* environment, const Random& base, vector<CellParticle>& particles,
			const size_t& max_rng) : base(base), particles(particles), max_rng(max_rng),
			source(environment->getModule<Source>()), geometry(environment->getModule<Geometry>()) {/* */}

void KeffSimulation::SourceSimulator::operator() (const tbb::blocked_range<size_t>& range) const {
	for(size_t i = range.begin() ; i < range.end() ; ++i) {
		Random r_local(base);
		r_local.jump(i * max_rng);
		particles[i] = source->sample(r_local);
	}
}

KeffSimulation::PowerStepSimulator::PowerStepSimulator(const McEnvironment* environment, const Random& base,
		const size_t& max_rng,vector<CellParticle>& current_bank,vector<vector<CellParticle> >& after_bank) :
		base(base), max_rng(max_rng), current_bank(current_bank),
		after_bank(after_bank), geometry(environment->getModule<Geometry>()), local_population(0.0)
		{/* */}

KeffSimulation::PowerStepSimulator::PowerStepSimulator(PowerStepSimulator& right, tbb::split) : base(right.base),
		max_rng(right.max_rng), current_bank(right.current_bank), after_bank(right.after_bank), geometry(right.geometry),
		local_population(0.0) {/* */}

void KeffSimulation::PowerStepSimulator::join(PowerStepSimulator& right) {
	local_population += right.local_population;
}

void KeffSimulation::PowerStepSimulator::operator() (const tbb::blocked_range<size_t>& range) {

	/* Get population */
	double population = local_population;

	/* Local bank */
	vector<vector<CellParticle> >& local_bank = after_bank;

	/* Bank to be simulated */
	vector<CellParticle>& fission_bank = current_bank;

	/* Geometry stuff */
	Surface* surface(0);  /* Surface pointer */
	bool sense(true);     /* Sense of the surface we are crossing */
	double distance(0.0); /* Distance to closest surface */

	for(size_t i = range.begin() ; i < range.end() ; ++i) {

		/* Random number stream for this particle */
		Random r(base);
		r.jump(i * max_rng);

		/* Flag if particle is out of the system */
		bool outside = false;

		/* 1. ---- Initialize particle from source (get particle from the bank) */
		CellParticle pc = fission_bank[i];
		const Cell* cell = pc.first;
		Particle particle = pc.second;

		while(true) {

			/* 2. ---- Get material and mean free path */
			const Material* material = cell->getMaterial();
			double mfp = material->getMeanFreePath(particle.erg());

			/* 3. ---- Get next surface's distance */
			cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

			/* 4. ---- Get collision distance (using mean free path) */
			double collision_distance = -log(r.uniform())*mfp;

			/* 5. ---- Check sampled distance against closest surface distance */
			while(collision_distance > distance) {

				/* 5.1 ---- Transport the particle to the surface */
				particle.pos() = particle.pos() + distance * particle.dir();

				/* 5.2 ---- Cross the surface (checking boundary conditions) */
				outside = not surface->cross(particle,sense,cell);
				assert(cell != 0);
				if(outside) break;

				/* 5.3 ---- Get material of the current cell (after crossing the surface) */
				material = cell->getMaterial();
				/* Mean free path (the particle didn't change the energy) */
				mfp = material->getMeanFreePath(particle.erg());

				/* 5.4 ---- Get next surface's distance */
				cell->intersect(particle.pos(),particle.dir(),surface,sense,distance);

				/* 5.5 ---- Get collision distance */
				collision_distance = -log(r.uniform())*mfp;
			}

			if(outside) break;

			/* 6. ---- Move the particle to the collision point */
			particle.pos() = particle.pos() + collision_distance * particle.dir();

			/* 7. ---- Sample isotope */
			const Isotope* isotope = material->getIsotope(particle.erg(),r);

			/* 8. ---- Sample reaction with the isotope */

			/* 8.1 ---- Check the type of reaction reaction */
			double absorption = isotope->getAbsorptionProb(particle.erg());
			double prob = r.uniform();
			if(prob < absorption) {
				/* Absorption reaction , we should check if this is a fission reaction */
				if(isotope->isFissile()) {
					double fission = isotope->getFissionProb(particle.erg());
					if(prob > (absorption - fission)) {
						/* We should bank the particle state after simulating the fission reaction */
						Reaction* fission_reaction = isotope->fission();
						(*fission_reaction)(particle, r);
						particle.sta() = Particle::BANK;
						population += particle.wgt();
						local_bank[i] = CellParticle(cell,particle);
					}
				}
				/* Kill the particle, this is an analog simulation */
				break;
			} else {
				/* Get elastic probability */
				double elastic = isotope->getElasticProb(particle.erg());
				/* 8.2 ---- Sample between inelastic and elastic scattering */
				if((prob - absorption) <= elastic) {
					/* Elastic reaction */
					Reaction* elastic_reaction = isotope->elastic();
					/* Apply the reaction */
					(*elastic_reaction)(particle,r);
				} else {
					/* Scatter with isotope sampling an inelastic reaction*/
					Reaction* inelastic_reaction = isotope->inelastic(particle.erg(),r);
					/* Apply the reaction */
					(*inelastic_reaction)(particle,r);
				}
			}
		}
	}

	/* Save new population */
	local_population = population;

}

KeffSimulation::KeffSimulation(const Random& _random, McEnvironment* _environment, double _keff, size_t _particles_number) :
		CriticalitySimulation(_random,_environment,_keff,_particles_number) {

	/* Get geometry from the environment */
	geometry = environment->getModule<Geometry>();

	/* Reserve space for the particle bank */
	fission_bank.resize(particles_number);

	/* Populate the particle bank with the initial source */
	tbb::parallel_for(tbb::blocked_range<size_t>(0,particles_number),SourceSimulator(environment,base,fission_bank,Source::max_samples));

	/* Jump on base stream of RNGs */
	base.jump(particles_number * Source::max_samples);
}

void KeffSimulation::launch() {

	/* --- Local particle bank for for this simulation */
	vector<vector<CellParticle> > local_bank(fission_bank.size());

	PowerStepSimulator power_step(environment,base,max_rng_per_history,fission_bank,local_bank);

	/* Simulate power step */
	tbb::parallel_reduce(tbb::blocked_range<size_t>(0, fission_bank.size()), power_step);

	/* Jump on random number generation */
	base.jump(fission_bank.size() * max_rng_per_history);

	/* Calculate multiplication factor for this cycle */
	keff = power_step.local_population / (double) particles_number;

	/* Clear particle bank*/
	fission_bank.clear();

	/* --- Re-populate the particle bank with the new source */
	for(vector<vector<CellParticle> >::const_iterator it = local_bank.begin() ; it != local_bank.end() ; ++it) {
		/* Loop over local bank */
		for(vector<CellParticle>::const_iterator it_particle = (*it).begin() ; it_particle != (*it).end() ; ++it_particle) {
			/* Get banked particle */
			CellParticle& banked_particle = (*it_particle);
			/* Split particle */
			double amp = banked_particle.second.wgt() / keff;
			int split = std::max(1,(int)(amp));
			/* New weight of the particle */
			banked_particle.second.wgt() = amp/(double)split;
			/* Put the split particle into the "simulation" list */
			for(int i = 0 ; i < split ; i++)
				fission_bank.push_back(banked_particle);
		}
	}

}




