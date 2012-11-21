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

#ifndef ENERGYLAW44_HPP_
#define ENERGYLAW44_HPP_

#include "AceEnergyLaw.hpp"
#include "EnergyTabular.hpp"

namespace Helios {
namespace AceReaction {

	/* ---------- Kalbach-87 Formalism (law 44) ---------- */

	/* Sample outgoing energy using a tabular distribution */
	class KalbachTabular : public TabularDistribution /* defined on AceReactionCommon.hpp */ {
		/* Precompound fraction */
		std::vector<double> r;
		/* Angular distribution slope */
		std::vector<double> a;
	public:

		KalbachTabular(const Ace::EnergyDistribution::Law44::EnergyData& ace_energy) :
			TabularDistribution(ace_energy.intt, ace_energy.eout, ace_energy.pdf, ace_energy.cdf),
			r(ace_energy.r), a(ace_energy.a)
		{/* */}

		void operator()(Random& random, double& energy, double& mu) const {
			/* Index on the outgoing grid */
			size_t idx;
			/* Set energy */
			energy = TabularDistribution::operator()(random, idx);
			/* Calculate Kalbach parameters */
			double rk, ak;
			/* Histogram interpolation */
			if(iflag == 1) {
				rk = r[idx];
				ak = a[idx];
			/* Linear-Linear interpolation */
			} else {
				rk = r[idx] + (r[idx + 1] - r[idx]) * (energy - out[idx]) * (out[idx + 1] - out[idx]);
				ak = a[idx] + (a[idx + 1] - a[idx]) * (energy - out[idx]) * (out[idx + 1] - out[idx]);
			}
			/* Random numbers */
			double chi = random.uniform();
			double rho = random.uniform();
			/* Sample scattering cosine */
			if(chi > rk) {
				/* Auxiliar factor */
				double t = (2 * rho - 1) * sinh(ak);
				/* Calculate MU */
				mu = log(t + sqrt(t*t + 1.0))/ak;
			} else {
				mu = log(rho*exp(ak) + (1.0 - rho)*exp(-ak))/ak;
			}
		}

		void print(std::ostream& out) const {
			out << " * Energy Tabular Distribution " << endl;
			TabularDistribution::print(out);
		}

		~KalbachTabular() {/* */}
	};

	class EnergyLaw44 : public EnergyOutgoingTabular<KalbachTabular> {
		typedef Ace::EnergyDistribution::Law44 Law44;
	public:
		EnergyLaw44(const Law* ace_data);
		~EnergyLaw44() {/* */}
	};

} /* namespace AceReaction */
} /* namespace Helios */
#endif /* ENERGYLAW44_HPP_ */
