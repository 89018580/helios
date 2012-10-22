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

#include <limits>

#include "Cell.hpp"
#include "Universe.hpp"
#include "Surface.hpp"
#include "Geometry.hpp"
#include <boost/tokenizer.hpp>

using namespace std;
using namespace boost;

namespace Helios {

Cell::Cell(const CellObject* definition, const std::vector<SenseSurface>& surfaces) :
	surfaces(surfaces),
	user_id(definition->getUserCellId()),
	flag(definition->getFlags()),
	fill(0),
	material(0),
	parent(0),
	internal_id(0) {
    /* Set the new cell on surfaces neighbor container */
    vector<Cell::SenseSurface>::const_iterator it_sur = surfaces.begin();
	for(; it_sur != surfaces.end() ; ++it_sur)
		(*it_sur).first->addNeighborCell((*it_sur).second,this);
}

void Cell::setFill(Universe* universe) {
	/* Link the universe filling this cell */
	fill = universe;
	fill->setParent(this);
}

std::ostream& operator<<(std::ostream& out, const Cell& q) {
	out << Log::ident(1) << "cell = " << q.getUserId() << " (internal = " << q.getInternalId() << ")" ;

	/* Print universe where this cell is */
	out << " ; universe = " << q.parent->getUserId();

	/* If this cell is filled with another universe, print the universe ID */
	if(q.fill) out << " ; fill = " << q.fill->getUserId();

	/* Print material */
	if(q.material) out << " ; material = " << q.material->getUserId();

	/* Print flags */
	out << " ; flags = " << q.getFlag(); out << endl;

	/* Print surfaces */
	vector<Cell::SenseSurface>::const_iterator it_sur = q.surfaces.begin();
	while(it_sur != q.surfaces.end()) {
		out << Log::ident(2);
		if((*it_sur).second) out << "(+) " << (*(*it_sur).first);
		else out << "(-) " << (*(*it_sur).first);
		out << endl;
		++it_sur;
	}

	return out;
}

const Cell* Cell::findCell(const Coordinate& position, const Surface* skip) const {
	vector<SenseSurface>::const_iterator it;
	for (it = surfaces.begin(); it != surfaces.end(); ++it) {
		if (it->first != skip) {
			if (it->first->sense(position) != it->second)
			/* The sense of the point isn't the same the same sense as we know this cell is defined... */
			return 0;
		}
	}
    /* If we get here, we are inside the cell :-) */
	if(fill) return fill->findCell(position,skip);
	else return this;
}

void Cell::intersect(const Coordinate& position, const Direction& direction, Surface*& surface, bool& sense, double& distance) const {

    /* If we have a parent "cell" we should check first on upper levels first */
    const Cell* parent_cell = parent->getParent();
    if(parent_cell){
    	parent_cell->intersect(position,direction,surface,sense,distance);
    } else {
        surface = 0;
        sense = false;
        distance = std::numeric_limits<double>::infinity();
    }

    /* Loop over surfaces */
	vector<SenseSurface>::const_iterator it;
	for (it = surfaces.begin() ; it != surfaces.end(); ++it) {
		/* Distance to the surface */
        double newDistance;
        /* Check the intersection with each surface */
		if(it->first->intersect(position,direction,it->second,newDistance)) {
			if (newDistance < distance) {
				/* Update data */
				distance = newDistance;
				surface = it->first;
				sense = it->second;
			}
		}
	}

}

static inline bool getSign(const SurfaceId& value) {
	if(value.find("-") != string::npos) return false;
	else return true;
}

static inline SurfaceId getAbsId(const SurfaceId& sense_id) {
	char_separator<char> sep("- ");
	/* Get user ID */
	tokenizer<char_separator<char> > tok(sense_id,sep);
	return (*tok.begin());
}

static inline vector<string> getUniqueTokens(const char_separator<char>& sep, const string& expresion) {
	tokenizer<char_separator<char> > tok(expresion, sep);
	vector<string> tokens;
	tokens.insert(tokens.begin(),tok.begin(),tok.end());
	sort(tokens.begin(),tokens.end());
	vector<string>::const_iterator it = unique(tokens.begin(),tokens.end());
	tokens.resize(it - tokens.begin());
	return tokens;
}

std::vector<SurfaceId> CellFactory::getSurfacesIds(const string& surface_expresion) {
	return getUniqueTokens(char_separator<char>("():- "),surface_expresion);
}

Cell* CellFactory::createCell(const CellObject* definition, std::map<SurfaceId,Surface*>& cell_surfaces) const {
	/* Get surface expression */
	string surface_expresion = definition->getSurfacesExpression();
	vector<string> tokens = getUniqueTokens(char_separator<char>("() "),surface_expresion);;

	/* Now get the tokens and craft a container with surfaces and senses */
	vector<Cell::SenseSurface> sense_surfaces_container;
	for(vector<string>::const_iterator it = tokens.begin() ; it != tokens.end() ; ++it) {
		Cell::SenseSurface sense_surface(cell_surfaces[getAbsId(*it)],getSign(*it));
		sense_surfaces_container.push_back(sense_surface);
	}

	return new Cell(definition,sense_surfaces_container);
}

} /* namespace Helios */
