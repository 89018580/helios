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

#ifndef ACEREACTIONBASE_HPP_
#define ACEREACTIONBASE_HPP_

#include "../../Isotope.hpp"
#include "MuSampler.hpp"
#include "EnergySampler.hpp"

namespace Helios {

namespace AceReaction {

	/*
	 * Base class to deal with Ace reactions. Basically an ACE reactions contains
	 * an interface for derived class to sample the scattering cosine or the energy
	 * distribution
	 */
	class AceReactionBase : public Reaction {
		/*
		 * Pointer to a cosine sampler. If the reaction does not contain a MU sampler (because
		 * is included on the energy sampler) this pointer is NULL
		 */
		MuSampler* mu_sampler;

		/* Pointer to the energy sampler. If the reaction does not contain one (for example,
		 * elastic scattering) this pointer is NULL. The energy sampler could sample the scattering
		 * cosine too.
		 */
		EnergySampler* energy_sampler;

		/* -- Sampler Builders */

		/* Build MU Sampler */
		static MuSampler* buildMuSampler(const Ace::AngularDistribution& ace_angular);

		/* Build Energy Sampler */
		static EnergySampler* buildEnergySampler(const Ace::EnergyDistribution& ace_energy);

	protected:
		/*
		 * Function to sample phase space coordinates of the particle
		 */

		/* Sample scattering cosine */
		void sampleCosine(const Particle& particle, Random& random, double& mu) {
			/* Sample MU */
			(*mu_sampler)(particle, random, mu);
		}

		/* Sample energy distribution (and MU if is available on the energy distribution) */
		void sampleEnergy(const Particle& particle, Random& random, double& energy, double& mu) {
			/* Sample energy */
			(*energy_sampler)(particle, random, energy, mu);
		}

	public:
		/* Constructor, from ACE isotope and the reaction parsed from the ACE library */
		AceReactionBase(const AceIsotope* isotope, const Ace::NeutronReaction& ace_reaction);

		virtual ~AceReactionBase();
	};

}

}


#endif /* ACEREACTIONBASE_HPP_ */
