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

#ifndef MCMODULE_HPP_
#define MCMODULE_HPP_

#include <vector>
#include <string>

#include "../Common/Common.hpp"

namespace Helios {

	class McEnvironment;

	/*
	 * Generic McObject definition
	 * All definitions on each module should be derived from this class
	 */
	class McObject {
		/* Pointer to the parent environment */
		McEnvironment* environment;
		/* Module name where this object should be constructed */
		std::string module;
		/* Name of this object */
		std::string name;
		/* Set the environment (each time this object pass trough the environment) */
		void setEnvironment(McEnvironment* env) {environment = env;}
		/* Friendly environment */
		friend class McEnvironment;
	protected:
		/*  Get the environment */
		const McEnvironment* getEnvironment() const;
	public:
		McObject(const std::string& module,const std::string& name) : environment(0), module(module), name(name) {/* */};

		/* Get the name of the module that deals with this object */
		std::string getModuleName() const {return module;}
		/* Get the name of this object */
		std::string getObjectName() const {return name;}

		virtual ~McObject(){/* */};
	};

	/*
	 * This class represents a component on the program that deal with families of related or dependent
	 * objects. For example, the Geometry class, the Materials class, and so on.
	 */
	class McModule {
		/* Module name */
		std::string name;
		/* Pointer to the parent environment */
		const McEnvironment* environment;
	public:
		McModule(const std::string& name, const McEnvironment* environment) : name(name), environment(environment) {/* */};
		/* Get module name */
		std::string getName() const {return name;}
		/*  Get the environment */
		const McEnvironment* getEnvironment() const {return environment;}
		virtual ~McModule() {/* */};
	};

	/* This class construct a specific module */
	class ModuleFactory {
		/* Name of the modules this factory creates */
		std::string name;
		/* Pointer to the parent environment */
		const McEnvironment* environment;
	protected:
		const McEnvironment* getEnvironment() const;
	public:
		ModuleFactory(const std::string& name, const McEnvironment* environment) : name(name), environment(environment) {/* */}
		/* Create an instance of an object from a group of objects definition */
		virtual McModule* create(const std::vector<McObject*>& objects) const = 0;
		/* Get name */
		std::string getName() const {return name;}
		virtual ~ModuleFactory() {/* */}
	};

} /* namespace Helios */
#endif /* MCMODULE_HPP_ */
