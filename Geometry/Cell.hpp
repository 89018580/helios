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

#ifndef CELL_HPP_
#define CELL_HPP_

#include <iostream>
#include <fstream>
#include <vector>

#include "Surface.hpp"

namespace Helios {

class Cell {

public:
    /* Pair of surface and sense */
    typedef std::pair<Surface*, bool> CellSurface;

	/* Friendly printer */
	friend std::ostream& operator<<(std::ostream& out, const Cell& q);

	/* Hold extra information about the cell */
	enum CellInfo {
        NONE     = 0, /* No special cell attributes */
        DEADCELL = 1, /* Particles should be killed when entering us */
        NEGATED  = 2, /* We are "everything but" what is inside our bounds */
        VOID     = 3  /* No material inside this cell */
    };

	Cell(const CellId& cellid, std::vector<CellSurface>& surfaces, const CellInfo flags = NONE);

    /* Get container of bounding surfaces. */
    const std::vector<CellSurface>& getBoundingSurfaces() const { return surfaces;}

    /* Return the cell ID. */
    const CellId& getCellId() const {return cellid;}

    /* Return information about this cell */
    CellInfo getFlag() const {return flag;}
	/* Set different options for the cell */
	void setFlags(CellInfo new_flag) {flag = new_flag;}

    /*
     * Check if the cell contains the point.
     * Optionally skip checking one surface if we know we've crossed it.
     */
    bool checkPoint(const Coordinate& position, const Surface* skip = 0) const;

    /* Get the nearest surface to a point in a given direction */
    void intersect(const Coordinate& position, const Direction& direction, Surface*& surface, bool& sense, double& distance) const;

	virtual ~Cell() {/* */};

private:

    /* A vector of surfaces and senses that define this cell */
    std::vector<CellSurface> surfaces;
    /* Cell id choose by the user */
    CellId cellid;
    /* Other information about this cell */
    CellInfo flag;
};

} /* namespace Helios */
#endif /* CELL_HPP_ */
