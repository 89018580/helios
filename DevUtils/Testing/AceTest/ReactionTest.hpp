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

#ifndef REACTIONTEST_HPP_
#define REACTIONTEST_HPP_

#include <string>
#include <algorithm>

#include "../../../Common/Common.hpp"
#include "../../../Material/AceTable/AceModule.hpp"
#include "../../../Material/AceTable/AceMaterial.hpp"
#include "../../../Material/AceTable/AceReader/Ace.hpp"
#include "../../../Material/AceTable/AceReader/AceUtils.hpp"
#include "../../../Material/AceTable/AceReader/Conf.hpp"
#include "../../../Material/AceTable/AceReaction/ElasticScattering.hpp"
#include "../../Utils.hpp"
#include "../TestCommon.hpp"

#include "gtest/gtest.h"

class SimpleReactionTest : public ::testing::Test {
protected:
	SimpleReactionTest() {/* */}
	virtual ~SimpleReactionTest() {/* */}
	void SetUp() {/* */}
	void TearDown() {/* */}
};

class Histogram {
	std::vector<double> bins;
	std::vector<double> limits;
	double min, max, total, delta;
public:
	Histogram(size_t nbins, double min, double max) : bins(nbins,0.0), limits(nbins + 1), min(min), max(max), total(0.0) {
		delta = (max - min) / (double)(bins.size());
		for(size_t i = 0 ; i < limits.size() ; ++i)
			limits[i] = min + (double)i * delta;
	}
	void put(double value) {
		if(value < limits[0]) bins[0]++;
		else if(value > limits[limits.size() - 1]) bins[bins.size()]++;
		else {
			size_t idx = std::upper_bound(limits.begin(), limits.end(), value) - limits.begin() - 1;
			bins[idx]++;
		}
	}
	void print() {
		for(size_t i = 0 ; i < bins.size() ; ++i) {
			std::cout << std::scientific << limits[i] << " " << bins[i] << std::endl;
		}
	}
	~Histogram() {/* */}
};

TEST_F(SimpleReactionTest, CheckReaction) {
	using namespace std;
	using namespace Helios;
	using namespace Ace;
	using namespace AceReaction;

	NeutronTable* ace_table = dynamic_cast<NeutronTable*>(AceReader::getTable("1001.03c"));

	ReactionContainer reactions = ace_table->getReactions();
	NeutronReaction& elastic_reaction = *reactions.get_mt(1);
	ElasticScattering<MuIsotropic> elastic(reactions.awr(), reactions.temp(), elastic_reaction.getAngular());

	Histogram histo(20,5E-13,5E-6);
	Particle particle;
	Random random(1);
	for(size_t i = 0 ; i < 100000000 ; ++i) {
		elastic(particle,random);
		histo.put(particle.erg().second);
	}

	histo.print();

	delete ace_table;
}


#endif /* REACTIONTEST_HPP_ */
