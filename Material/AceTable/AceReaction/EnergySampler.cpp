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

#include "EnergySampler.hpp"

namespace Helios {

using namespace AceReaction;

EnergySampler* EnergySamplerFactory::createSampler(const Ace::EnergyDistribution& ace_data) {
	if(ace_data.laws.size() != 1)
		throw(EnergySampler::BadEnergySamplerCreation("More than 1 energy law distribution in is not supported"));

	/* Get law */
	int law = ace_data.laws[0]->getLaw();

	if(law == 4) {
		/* Continuous Tabular Distribution */
		return new EnergyLaw4(dynamic_cast<Ace::EnergyDistribution::Law4*>(ace_data.laws[0]));
	}
	/* No law */
	throw(EnergySampler::BadEnergySamplerCreation("Energy law " + toString(law) + " is not supported"));
}

/* Constructor */
EnergyLaw4::EnergyLaw4(const Ace::EnergyDistribution::Law4* ace_data) {
	typedef Ace::EnergyDistribution::Law4 AceLaw;
	energies = ace_data->ein;
	/* Sanity check */
	assert(ace_data->eout_dist.size() == energies.size());
	/* Create the tables */
	for(vector<AceLaw::EnergyData>::const_iterator it = ace_data->eout_dist.begin() ; it != ace_data->eout_dist.end() ; ++it)
		tables.push_back(new EnergyTabular(*it));
}

void EnergyLaw4::print(std::ostream& out) const {
	out << " - Cosine Table Sampler " << endl;
	for(size_t i = 0 ; i < tables.size() ; ++i) {
		out << "energy = " << scientific << energies[i] << endl;
		tables[i]->print(out);
	}
}

EnergyLaw4::~EnergyLaw4() {
	for(size_t i = 0 ; i < tables.size() ; ++i) {
		delete tables[i];
	}
}

}



