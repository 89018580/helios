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
 DISCLAIMED. IN NO EVENT SHALL ESTEBAN PELLEGRINO BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <cstdlib>

#include "TyrDistribution.hpp"
#include "AceUtils.hpp"

using namespace std;
using namespace Ace;

TyrDistribution::TyrDistribution(const TyrDistribution& tyrr)
         : type(tyrr.type) {

	if(type == distribution) {
		nr = tyrr.nr;
		nbt = tyrr.nbt;
		aint = tyrr.aint;
		ne = tyrr.ne;
		energies = tyrr.energies;
		nu = tyrr.nu;
	}

	tyr = tyrr.tyr;

	for(vector<NUBlock::NuData*>::const_iterator it = tyrr.fission_data.begin() ; it != tyrr.fission_data.end() ; ++it)
		fission_data.push_back((*it)->clone());

}

void TyrDistribution::setFission(const std::vector<NUBlock::NuData*>& nu_data) {
	for(vector<NUBlock::NuData*>::const_iterator it = nu_data.begin() ; it != nu_data.end() ; ++it)
		fission_data.push_back((*it)->clone());
	type = fission;
}

TyrDistribution::TyrDistribution(int tyr, std::vector<double>::const_iterator it) : tyr(tyr){
	type = distribution;
	getXSS(nr,it);
	getXSS(nbt,nr,it);
	getXSS(aint,nr,it);
	getXSS(ne,it);
	getXSS(energies,ne,it);
	getXSS(nu,ne,it);
}

void TyrDistribution::dump(std::ostream& xss) const {
	if(type == distribution) {
		putXSS(nr,xss);
		putXSS(nbt,xss);
		putXSS(aint,xss);
		putXSS(ne,xss);
		putXSS(energies,xss);
		putXSS(nu,xss);
	} else if(type == number)
		putXSS(tyr,xss);
}

TyrDistribution::~TyrDistribution() {
	for(vector<NUBlock::NuData*>::const_iterator it = fission_data.begin() ; it != fission_data.end() ; ++it)
		delete (*it);
};

