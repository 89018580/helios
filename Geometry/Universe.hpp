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

#ifndef UNIVERSE_HPP_
#define UNIVERSE_HPP_

#include <vector>

#include "Cell.hpp"
#include "../Common.hpp"

namespace Helios {

	class Transformation {

		/* A translation transformation */
		Direction translation;
		/* A rotation (degrees around each of the 3 axis) */
		Direction rotation;

	public:
		Transformation(const Direction& translation = Direction(0,0,0), const Direction& rotation = Direction(0,0,0))
						: translation(translation), rotation(rotation) {/* */}

		/* Returns a new instance of a cloned transformed surface */
		Surface* operator()(const Surface* surface) const { return surface->transformate(translation); }

		/* Sum transformations */
		const Transformation operator+(const Transformation& right) const {
			return Transformation(right.translation + translation, right.rotation + rotation);
		}

		~Transformation() {/* */}
	};

	class Universe {

		friend class UniverseFactory;
		/* Friendly printer */
		friend std::ostream& operator<<(std::ostream& out, const Universe& q);

		/* Internal identification of this universe */
		InternalUniverseId int_univid;
		/* A vector of cells */
		std::vector<Cell*> cells;
		/* Universe id choose by the user */
		UniverseId univid;
		/*
		 * Parent cell : Each universe has ONLY one parent cell. If more than one cell
		 * is filled with the same universe, the universe is cloned. The base universe
		 * has a NULL parent.
		 */
		Cell* parent;

	protected:

		Universe(const UniverseId& univid, Cell* parent = 0);
		/* Prevent copy */
		Universe(const Universe& uni);
		Universe& operator=(const Universe& uni);

	public:

		/* Add a cell */
		void addCell(Cell* cell);
		/* Get cells of this universe */
		const std::vector<Cell*>& getCells() const {return cells;};

		/* Find cell inside the universe */
		const Cell* findCell(const Coordinate& position, const Surface* skip = 0) const;

		/* Set a parent for this universe */
		void setParent(Cell* cell) {parent = cell;};
		/* Get parent cell */
		const Cell* getParent() const {return parent;}

		/* Return the user ID associated with the universe. */
		const UniverseId& getUserId() const {return univid;}
		/* Set internal / unique identifier for the cell */
		void setInternalId(const InternalCellId& internal) {int_univid = internal;}
		/* Return the internal ID associated with the universe. */
		const InternalUniverseId& getInternalId() const {return int_univid;}
		/* Get number of cells */
		size_t getCellCount() const {return cells.size();}

		virtual ~Universe() {/* */};
	};

	class UniverseFactory {

		/* Static instance of the factory */
		static UniverseFactory factory;

		/* Prevent construction or copy */
		UniverseFactory() {/* */};
		UniverseFactory& operator= (const UniverseFactory& other);
		UniverseFactory(const UniverseFactory&);
		virtual ~UniverseFactory() {/* */}

	public:
		/* Access the factory, reference to the static singleton */
		static UniverseFactory& access() {return factory;}

		/* Create a new surface */
		Universe* createUniverse(const UniverseId& univid, Cell* parent = 0) const {
			return new Universe(univid, parent);
		}

	};

	std::ostream& operator<<(std::ostream& out, const Universe& q);

} /* namespace Helios */
#endif /* UNIVERSE_HPP_ */
