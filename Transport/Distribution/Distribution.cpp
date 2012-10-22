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

#include "Distribution.hpp"
#include "Spatial.hpp"
#include "Angular.hpp"

using namespace std;

namespace Helios {

DistributionCustomObject::DistributionCustomObject(const std::string& type, const DistributionId& distid,
		   const std::vector<DistributionId>& samplersIds, const std::vector<double>& weights) :
	DistributionBaseObject(type,distid) , samplersIds(samplersIds) , weights(weights) {
	/* Check the weight input */
	if(this->weights.size() == 0) {
		this->weights.resize(this->samplersIds.size());
		/* Equal probability for all samplers */
		double prob = 1/(double)this->samplersIds.size();
		for(size_t i = 0 ; i < this->samplersIds.size() ; ++i)
			this->weights[i] = prob;
	}
}

/* Constructor from definition */
DistributionCustom::DistributionCustom(const DistributionBaseObject* definition) : DistributionBase(definition) {
	const DistributionCustomObject* distObject = static_cast<const DistributionCustomObject*>(definition);
	/* Weights of each sampler */
	std::vector<double> weights = distObject->getWeights();
	/* Samplers */
	std::vector<DistributionBase*> samplers = distObject->getDistributions();
	/* Create sampler */
	distribution_sampler = new Sampler<DistributionBase*>(samplers,weights);
};

DistributionFactory::DistributionFactory() {
	/* Distribution registering */
	registerDistribution(Box1D<xaxis>());
	registerDistribution(Box1D<yaxis>());
	registerDistribution(Box1D<zaxis>());
	registerDistribution(Box2D<xaxis>());
	registerDistribution(Box2D<yaxis>());
	registerDistribution(Box2D<zaxis>());
	registerDistribution(Box3D());
	registerDistribution(Cyl2D<xaxis>());
	registerDistribution(Cyl2D<yaxis>());
	registerDistribution(Cyl2D<zaxis>());
	registerDistribution(Isotropic());
	registerDistribution(DistributionCustom());
}

DistributionBase* DistributionFactory::createDistribution(const DistributionBaseObject* definition) const {
	map<string,DistributionBase::Constructor>::const_iterator it_type = constructor_table.find(definition->getType());
	if(it_type != constructor_table.end())
		return (*it_type).second(definition);
	else
		throw DistributionBase::BadDistributionCreation(definition->getUserId(),"Distribution type " + definition->getType() + " is not defined");
}

void DistributionFactory::registerDistribution(const DistributionBase& distribution) {
	constructor_table[distribution.getName()] = distribution.constructor();
}

DistributionBase::DistributionBase(const DistributionBaseObject* definition) : user_id(definition->getUserId()) {/* */};

} /* namespace Helios */
