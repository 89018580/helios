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
#include <boost/mpi/environment.hpp>
#include <boost/mpi/communicator.hpp>

#include <iostream>
#include <string>
#include <vector>
#include <list>

#include "Parser/ParserTypes.hpp"
#include "Environment/McEnvironment.hpp"

using namespace std;
namespace mpi = boost::mpi;
using namespace Helios;

int main(int argc, char **argv) {

	/* Print header */
	Log::header();

	/* Check number of arguments */
	if(argc < 2) {
	  Helios::Log::error() << "Usage : " << argv[0] << " <filename>" << Helios::Log::endl;
	  return 1;
	}

	/* Initialize MPI environment */
	mpi::environment env(argc, argv);
	mpi::communicator world;

	/* Set rank on the logger */
	Log::setRank(world.rank());

	/* Parser (XML for now) */
	Parser* parser = new XmlParser;

	/* Container of filenames */
	vector<string> input_files;

	while(*(++argv)) {
		string filename = string(*(argv));
		input_files.push_back(filename);
	}

	/* Environment */
	McEnvironment environment(parser);
	try {
		/* Parse files, to get the information to create the environment */
		environment.parseFiles(input_files);
		/* Setup the problem */
		environment.setup();
		/* Launch simulation */
		environment.simulate(world);
	} catch(exception& error) {
		Log::error() << error.what() << Log::endl;
		return 1;
	}

	delete parser;
	return 0;
}
