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

#include "Parser.hpp"

using namespace std;

namespace Helios {

void Parser::parseGeometry(const std::string& geo_file) const {
	/* Cell and surfaces definitions */
	vector<Geometry::SurfaceDefinition> sur_def;
	vector<Geometry::CellDefinition> cell_def;

	/* Get geometry information */
	getGeometryInformation(geo_file,sur_def,cell_def);

	/* Add surfaces */
	vector<Geometry::SurfaceDefinition>::iterator it_sur = sur_def.begin();
	for(; it_sur != sur_def.end() ; ++it_sur) {
		try {
			/* Add surface into the geometry */
			Geometry::access().addSurface((*it_sur));
		} catch (Surface::BadSurfaceCreation& exception) {
			/* Catch exception */
			cerr << "[@] Exception caught : " << exception.what() << endl;
		}
	}

	/* Add cells */
	vector<Geometry::CellDefinition>::iterator it_cell = cell_def.begin();
	for(; it_cell != cell_def.end() ; ++it_cell) {
		try {
			/* Add surface into the geometry */
			Geometry::access().addCell((*it_cell));
		} catch (Surface::BadSurfaceCreation& exception) {
			/* Catch exception */
			cerr << "[@] Exception caught : " << exception.what() << endl;
		}
	}

}

} /* namespace Helios */
