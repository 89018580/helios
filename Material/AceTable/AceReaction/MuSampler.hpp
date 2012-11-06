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

#ifndef MUSAMPLER_HPP_
#define MUSAMPLER_HPP_

#include <vector>
#include <algorithm>
#include <cassert>

namespace Helios {

namespace AceReaction {

	/* Tables for sampling scattering cosine obtained from an ACE cross section library */
	typedef Ace::AngularDistribution::AngularArray AceAngular;
	typedef Ace::AngularDistribution::Isotropic AceIsotropic;
	typedef Ace::AngularDistribution::EquiBins AceEquiBins;
	typedef Ace::AngularDistribution::Tabular AceTabular;

	/* Base sampling table */
	class CosineTable {
	public:
		CosineTable() {/* */}
		virtual double operator()(Random& random) = 0;
		virtual ~CosineTable() {/* */}
	};

	/* Sample isotropic scattering cosine */
	class Isotropic : public CosineTable {
	public:
		Isotropic(const AceAngular* ace_angular) {/* */}

		double operator()(Random& random) const {
			/* Return value */
			return (1.0 - 2.0 * random.uniform());
		}

		~Isotropic() {/* */}
	};

	/* Sample scattering cosine using 32 equiprobable bins */
	class EquiBins : public CosineTable {
		/* Cosine bins */
		std::vector<double> bins;
	public:
		EquiBins(const AceEquiBins* ace_angular) :
			bins(ace_angular->bins)
		{
			/* Sanity check */
			assert(bins.size() == 33);
		}

		double operator()(Random& random) const {
			/* Sample random number */
			double chi = random.uniform();
			/* Sample bin */
			size_t pos = (size_t) (chi * 32);
			/* Get interpolated cosine */
			return bins[pos] + (chi - pos) * (bins[pos + 1] - bins[pos]);
		}

		~EquiBins() {/* */}
	};

	/* Sample scattering cosine using a tabular distribution */
	class Tabular : public CosineTable {
		int iflag;                 /* 1 = histogram, 2 = lin-lin */
		std::vector<double> csout; /* Cosine scattering angular grid */
		std::vector<double> pdf;   /* Probability density function */
		std::vector<double> cdf;   /* Cumulative density function */
	public:
		Tabular(const AceTabular* ace_angular) :
			 iflag(ace_angular->iflag) ,csout(ace_angular->csout)
			,pdf(ace_angular->pdf), cdf(ace_angular->cdf)
		{
			/* Sanity check */
			assert(csout.size() == pdf.size());
			assert(cdf.size() == pdf.size());
		}

		double operator()(Random& random) const {
			/* Get random number */
			double chi = random.uniform();
			/* Sample the bin on the cumulative */
			size_t idx = std::upper_bound(cdf.begin(), cdf.end(), chi) - cdf.begin();

			/* Histogram interpolation */
			if(iflag == 1) {
				return csout[idx] + (chi - cdf[idx]) / pdf[idx];

			/* Linear-Linear interpolation */
			} else if(iflag == 2) {
				/* Auxiliary variables */
				double g = (pdf[idx + 1] - pdf[idx]) / (csout[idx + 1] - csout[idx]);
				double h = sqrt(pdf[idx] * pdf[idx] + 2*g*(chi - cdf[idx]));
				/* Solve for cosine */
				return csout[idx] + (1/g) * (h - pdf[idx]);
			}

			return 0.0;
		}

		virtual ~Tabular() {/* */}
	};

	/*
	 * Generic cosine sampler.
	 *
	 * This class have a sampler table for each incident energy tabulated.
	 * Before sampling a scattering cosine, the class samples the cosine table
	 * using the incident particle energy.
	 */
	class MuSampler {
		/* Tabulated incident energies */
		std::vector<double> energies;
		/* Cosine table for each tabulated energy */
		std::vector<CosineTable*> cosine_table;

		/* Cosine table builder */
		static CosineTable* tableBuilder(const AceAngular* ace_array);
	public:
		MuSampler(const Ace::AngularDistribution& ace_data);

		/* Sample cosine */
		double operator()(double energy, Random& random) {

		}

		~MuSampler() {/* */}
	};
}

}


#endif /* MUSAMPLER_HPP_ */