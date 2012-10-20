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

#include <cstdlib>
#include <set>
#include<boost/tokenizer.hpp>

#include "Surface.hpp"
#include "Cell.hpp"
#include "Universe.hpp"
#include "GeometricFeature.hpp"
#include "../Environment/McEnvironment.hpp"
#include "Geometry.hpp"

using namespace std;
using namespace boost;

namespace Helios {

static inline bool getSign(const SurfaceId& value) {
	if(value.find("-") != string::npos)
		return false;
	else
		return true;
}

McModule* GeometryFactory::create(const std::vector<McObject*>& objects) const  {
	/* Try to get the materials */
	const Materials* materials = 0;
	try {
		materials = getEnvironment()->getModule<Materials>();
	} catch (exception& error) {
		Log::warn() << error.what() << endl;
	}
	return new Geometry(objects,materials);
}

template<class T>
static void pushObject(McObject* geo, std::vector<T*>& definition) {
	definition.push_back(dynamic_cast<T*>(geo));
}

Geometry::Geometry(const std::vector<McObject*>& definitions, const Materials* materials) : McModule(name()) {
	/* Objects */
	vector<SurfaceObject*> surObjects;
	vector<CellObject*> cellObjects;
	vector<FeatureObject*> featureObjects;

	/* Dispatch each definition to the corresponding container */
	vector<McObject*>::const_iterator it_def = definitions.begin();

	/* Get the type and put the object on the correct container */
	for(; it_def != definitions.end() ; ++it_def) {
		string type = (*it_def)->getObjectName();
		if (type == Cell::name())
			pushObject(*it_def,cellObjects);
		else if (type == Surface::name())
			pushObject(*it_def,surObjects);
		else if (type == GeometricFeature::name())
			pushObject(*it_def,featureObjects);
	}

	/* Keep the new objects added by the features */
	vector<CellObject*> cellFeatureObject;
	vector<SurfaceObject*> surFeatureObject;

	/* Create geometric features */
	if(featureObjects.size() != 0) {
		/* Now lets move on into the lattices */
		for(vector<FeatureObject*>::const_iterator it = featureObjects.begin() ; it != featureObjects.end() ; ++it) {
			/* Create a lattice factory */
			GeometricFeature* feature = feature_factory.createFeature(*it);
			feature->createFeature((*it),surFeatureObject,cellFeatureObject);
			delete feature;
		}
	}

	/* Reserve... */
	surObjects.reserve(surObjects.size() + surFeatureObject.size());
	cellObjects.reserve(cellObjects.size() + cellFeatureObject.size());
	/* ...and concatenate this new definitions to the current ones */
	surObjects.insert( surObjects.end(), surFeatureObject.begin(), surFeatureObject.end() );
	cellObjects.insert( cellObjects.end(), cellFeatureObject.begin(), cellFeatureObject.end() );

	/* First we create all the surfaces defined by the user. Ultimately, we'll have to clone and transform this ones */
	map<SurfaceId,Surface*> user_surfaces;
	vector<SurfaceObject*>::const_iterator it_sur = surObjects.begin();
	for(; it_sur != surObjects.end() ; ++it_sur) {
		/* Surface information */
		SurfaceId userSurfaceId((*it_sur)->getUserSurfaceId());
		/* Check duplicated IDs */
		map<SurfaceId,Surface*>::const_iterator it_id = user_surfaces.find(userSurfaceId);
		if(it_id != user_surfaces.end())
			throw Surface::BadSurfaceCreation(userSurfaceId,"Duplicated id");

		/* Create surface */
		Surface* new_surface = surface_factory.createSurface((*it_sur));
		/* Update surface map */
		user_surfaces[new_surface->getUserId()] = new_surface;
	}

	/* Check for duplicated cells */
	set<CellId> user_cell_ids;
	vector<CellObject*>::const_iterator it_cell = cellObjects.begin();
	for(; it_cell != cellObjects.end() ; ++it_cell) {
		CellId userCellId = (*it_cell)->getUserCellId();
		/* Check duplicated IDs */
		set<CellId>::const_iterator it_id = user_cell_ids.find(userCellId);
		if(it_id != user_cell_ids .end())
			throw Cell::BadCellCreation(userCellId,"Duplicated id");
		/* Check other stuff */
		if((*it_cell)->getFill() != Universe::BASE && ((*it_cell)->getFill() == (*it_cell)->getUniverse()))
			throw Cell::BadCellCreation(userCellId,"What are you trying to do? You can't fill a cell with the same universe in which is contained");

		user_cell_ids.insert(userCellId);
	}

	/* Map cell with universes */
	map<UniverseId,vector<CellObject*> > u_cells;  /* Universe definition */
	for(it_cell = cellObjects.begin() ; it_cell != cellObjects.end() ; ++it_cell) {
		UniverseId universe = (*it_cell)->getUniverse();
		u_cells[universe].push_back(*it_cell);
	}

	addUniverse((*u_cells.begin()).first,u_cells,user_surfaces);

	/* Now, if we have a Materials pointer, we should update the materials on each cell */
	if(materials)
		setupMaterials(*materials);

	/* Clean surfaces */
	map<SurfaceId,Surface*>::iterator it_user = user_surfaces.begin();
	for(; it_user != user_surfaces.end() ; ++it_user)
		delete (*it_user).second;

	/* Finally we should purge the extra definitions added by the geometry feature */
	purgePointers(cellFeatureObject);
	purgePointers(surFeatureObject);
};

CellId Geometry::getPath(const Cell* cell) const {
	/* Get the internal ID */
	InternalCellId internal = cell->getInternalId();
	map<InternalCellId,CellId>::const_iterator it = cell_path_map.find(internal);
	/* This is the full path of this cell */
	return (*it).second;
}

CellId Geometry::getUserId(const Cell* cell) const {
	/* This is the full path of this cell */
	CellId full_path = getPath(cell);
	/* Get the original ID of the cell */
	char_separator<char> sep("< ");
	tokenizer<char_separator<char> > tok(full_path,sep);
	return *tok.begin();
}

std::vector<Cell*> Geometry::getCells(const std::string& pathOrig) {
	/* Erase spaces */
	string path(pathOrig);
	path.erase(std::remove_if(path.begin(), path.end(),::isspace), path.end());
	/* Detect if is a full path (only one cell) or a group of cells */
	if(path.find("<") != string::npos) {
		/* One specific cell */
		map<CellId,InternalCellId>::iterator it = cell_reverse_map.find(path);
		if(it != cell_reverse_map.end()) {
			std::vector<Cell*> ptr;
			ptr.push_back(cells[(*it).second]);
			return ptr;
		} else
			throw GeometryError("Could not find any cell on path " + path);
	} else {
		/* Group of cells (or a cell on top level) */
		map<CellId,vector<InternalCellId> >::iterator it = cell_internal_map.find(path);
		if(it != cell_internal_map.end()) {
			std::vector<Cell*> ptrs;
			vector<InternalCellId> internal = (*it).second;
			for(vector<InternalCellId>::const_iterator it_internal = internal.begin() ; it_internal != internal.end() ; ++it_internal)
				ptrs.push_back(cells[*it_internal]);
			return ptrs;
		} else
			throw GeometryError("Cell " + path + " does not exist");
	}
}

std::vector<Surface*> Geometry::getSurfaces(const std::string& pathOrig) {
	/* Erase spaces */
	string path(pathOrig);
	path.erase(std::remove_if(path.begin(), path.end(),::isspace), path.end());
	/* Detect if is a full path (only one surface) or a group of surfaces */
	if(path.find("<") != string::npos) {
		/* One specific cell */
		map<SurfaceId,InternalSurfaceId>::iterator it = surface_reverse_map.find(path);
		if(it != surface_reverse_map.end()) {
			std::vector<Surface*> ptr;
			ptr.push_back(surfaces[(*it).second]);
			return ptr;
		} else
			throw GeometryError("Could not find any surface on path " + path);
	} else {
		/* Group of surfaces (or a surface on top level) */
		map<SurfaceId,vector<InternalSurfaceId> >::iterator it = surface_internal_map.find(path);
		if(it != surface_internal_map.end()) {
			std::vector<Surface*> ptrs;
			vector<InternalSurfaceId> internal = (*it).second;
			for(vector<InternalSurfaceId>::const_iterator it_internal = internal.begin() ; it_internal != internal.end() ; ++it_internal)
				ptrs.push_back(surfaces[*it_internal]);
			return ptrs;
		} else
			throw GeometryError("Surface " + path + " does not exist");
	}
}

SurfaceId Geometry::getPath(const Surface* surf) const {
	/* Get the internal ID */
	InternalSurfaceId internal = surf->getInternalId();
	map<InternalSurfaceId,SurfaceId>::const_iterator it = surface_path_map.find(internal);
	/* This is the full path of this cell */
	return (*it).second;
}

SurfaceId Geometry::getUserId(const Surface* surf) const {
	/* This is the full path of this cell */
	SurfaceId full_path = getPath(surf);
	/* Get the original ID of the cell */
	char_separator<char> sep("< ");
	tokenizer<char_separator<char> > tok(full_path,sep);
	return *tok.begin();
}

Surface* Geometry::addSurface(const Surface* surface, const ParentCell& parent_cell, const std::string& surf_id) {
	/* Create the new duplicated surface */
	Surface* new_surface = parent_cell.getTransformation()(surface);

	/* Check if the surface is not duplicated */
	vector<Cell::SenseSurface>::const_iterator it_sur = parent_cell.getSurfaces().begin();
	for(; it_sur != parent_cell.getSurfaces().end() ; ++it_sur) {
		if(*new_surface == *((*it_sur).first)) {
			delete new_surface;
			return (*it_sur).first;
		}
	}

	/* Set internal / unique index */
	new_surface->setInternalId(surfaces.size());
	/* Update surface map */
    SurfaceId new_surf_id;
    if(parent_cell.getId().size() == 0) new_surf_id = surf_id;
    else new_surf_id = surf_id + "<" + parent_cell.getId();

    /* Update path map */
    surface_path_map[new_surface->getInternalId()] = new_surf_id;
    /* Update internal map */
    surface_internal_map[surf_id].push_back(new_surface->getInternalId());
    /* Update reverse map */
    surface_reverse_map[new_surf_id] = new_surface->getInternalId();

	/* Push the surface into the container */
	surfaces.push_back(new_surface);

	/* Return the new surface */
	return new_surface;
}

Universe* Geometry::addUniverse(const UniverseId& uni_def, const map<UniverseId,vector<CellObject*> >& u_cells,
		                        const map<SurfaceId,Surface*>& user_surfaces, const ParentCell& parent_cell) {

	/* Create universe */
	Universe* new_universe = new Universe(uni_def);
	/* Set internal / unique index */
	new_universe->setInternalId(universes.size());
	/* Push the universe into the container */
	universes.push_back(new_universe);
	/* Update universe map */
	universe_map[new_universe->getUserId()].push_back(new_universe->getInternalId());

	map<UniverseId,vector<CellObject*> >::const_iterator it_uni_cells = u_cells.find(uni_def);
	if(it_uni_cells == u_cells.end()) return 0;

	/* Get the cell of this level */
	vector<CellObject*> cell_def = (*it_uni_cells).second;

	/* Add each cell of this universe */
	vector<CellObject*>::iterator it_cell = cell_def.begin();
    map<SurfaceId,Surface*> temp_sur_map;

    /* Separator to get user ID */
	char_separator<char> sep("- ");

	for(; it_cell != cell_def.end() ; ++it_cell) {

		/* Cell information */
		CellId user_cell_id((*it_cell)->getUserCellId());
		vector<SurfaceId> surfaces_id((*it_cell)->getSurfaceIds());

		/* Now get the surfaces and put the references inside the cell */
	    vector<Cell::SenseSurface> bounding_surfaces;
	    vector<SurfaceId>::const_iterator it = surfaces_id.begin();

	    for (;it != surfaces_id.end(); ++it) {
	    	/* Get user ID */
	    	tokenizer<char_separator<char> > tok((*it),sep);
	    	SurfaceId user_surface_id(*tok.begin());

	    	/* Get internal index */
	    	map<SurfaceId,Surface*>::const_iterator it_sur = user_surfaces.find(user_surface_id);
	    	if(it_sur == user_surfaces.end())
	    		throw Cell::BadCellCreation(user_cell_id,"Surface number " + toString(user_surface_id) + " doesn't exist.");

	    	/* New surface for this cell */
	    	Surface* new_surface = 0;

	    	/* Check for already created surfaces inside this universe */
	    	map<SurfaceId,Surface*>::const_iterator it_temp_sur = temp_sur_map.find(user_surface_id);
	    	if(it_temp_sur != temp_sur_map.end())
	    		/* The surface is created */
	    		new_surface = (*it_temp_sur).second;
	    	else {
	    		new_surface = addSurface((*it_sur).second,parent_cell,user_surface_id);
		    	temp_sur_map[new_surface->getUserId()] = new_surface;
	    	}

	    	/* Get surface with sense */
	    	Cell::SenseSurface newSurface(new_surface,getSign(*it));

	    	/* Push it into the container */
	        bounding_surfaces.push_back(newSurface);
	    }

	    /* Push the surfaces with sense into the cell definition */
	    (*it_cell)->setSenseSurface(bounding_surfaces);

	    /* Now we can construct the cell */
	    Cell* new_cell = cell_factory.createCell((*it_cell));
	    /* Get new cell ID based on the parent cell */
	    CellId cell_id;
	    if(parent_cell.getId().size() == 0) cell_id = (*it_cell)->getUserCellId();
	    else cell_id = (*it_cell)->getUserCellId() + "<" + parent_cell.getId();
		/* Set internal / unique index */
	    new_cell->setInternalId(cells.size());

	    /* Update cell map */
	    cell_path_map[new_cell->getInternalId()] = cell_id;
	    /* Update internal map */
	    cell_internal_map[(*it_cell)->getUserCellId()].push_back(new_cell->getInternalId());
	    /* Update reverse map */
	    cell_reverse_map[(*it_cell)->getUserCellId()] = new_cell->getInternalId();

	    /* Update material map */
	    material_map[new_cell->getInternalId()] = (*it_cell)->getMatId();
	    /* Push the cell into the container */
	    cells.push_back(new_cell);
	    /* Link this cell with the new universe */
	    new_universe->addCell(new_cell);

	    /* Check if this cell is filled by another universe */
	    UniverseId fill_universe_id = (*it_cell)->getFill();
	    if(fill_universe_id != Universe::BASE) {

	    	/* Get parent surfaces */
	    	std::vector<Cell::SenseSurface> parent_surfaces = parent_cell.getSurfaces();

	    	/* Push parent surfaces into bounding surfaces */
	    	vector<Cell::SenseSurface>::const_iterator it_psur = parent_surfaces.begin();
	    	for(; it_psur != parent_surfaces.end() ; ++it_psur)
	    		bounding_surfaces.push_back((*it_psur));

	    	/* Propagate the transformation... */
	    	Transformation new_transformation = parent_cell.getTransformation() + (*it_cell)->getTransformation();
	    	/* ... and create a new Parent Cell */
	    	ParentCell new_parent(new_transformation,bounding_surfaces,cell_id);

	    	/* Create recursively the other universes */
	    	Universe* fill_universe = addUniverse(fill_universe_id,u_cells,user_surfaces,new_parent);

	    	if(fill_universe)
	    		new_cell->setFill(fill_universe);
	    	else
	    		throw Cell::BadCellCreation((*it_cell)->getUserCellId(),
	    				"Attempting to fill with an empty universe (fill = " + toString(fill_universe_id) + ") " );
	    }
	}

	/* Return the universe */
	return new_universe;
}

void Geometry::setupMaterials(const Materials& materials) {
	/* Iterate over each material on the map */
	map<InternalCellId, MaterialId>::const_iterator it_mat = material_map.begin();
	for(; it_mat != material_map.end() ; ++it_mat) {
		/* Get cell */
		Cell* cell = cells[(*it_mat).first];
		/* Get material ID */
		MaterialId matId = (*it_mat).second;
		if(matId != Material::NONE && matId != Material::VOID) {
			try {
				cell->setMaterial(materials.getMaterial(matId));
			} catch (std::exception& error) {
				throw Cell::BadCellCreation(getUserId(cell),error.what());
			}

		} else if (matId == Material::NONE) {
			/* No material in this cell, we should check if the cell is filled with something */
			if(!cell->getFill())
				throw Cell::BadCellCreation(getUserId(cell),
						"The cell is not filled with a material or a universe");
		}
	}
}

void Geometry::printGeo(std::ostream& out) const {
	vector<Universe*>::const_iterator it_uni = universes.begin();
	for(; it_uni != universes.end() ; it_uni++) {
		out << "---- universe = " << (*it_uni)->getUserId() << endl;
		(*it_uni)->print(out,this);
	}
}

Geometry::~Geometry() {
	purgePointers(surfaces);
	purgePointers(cells);
	purgePointers(universes);
}

} /* namespace Helios */
