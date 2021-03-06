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

#ifndef XMLPARSER_HPP_
#define XMLPARSER_HPP_

#include <vector>
#include <set>
#include <string>

#include "ticpp.h"
#include "../Parser.hpp"

namespace Helios {

	class XmlParser : public Parser {

		/* Prevent copy */
		XmlParser(const XmlParser& p);
		XmlParser& operator=(const XmlParser& p);

		/* Process root node */
		void rootNode(TiXmlNode* pParent, const std::string& filename);

		/* ---- Process nodes */
		void geoNode(TiXmlNode* pParent);
		void matNode(TiXmlNode* pParent);
		void srcNode(TiXmlNode* pParent);
		void setNode(TiXmlNode* pParent);

		/* Map of functions for each root node */
		typedef void (XmlParser::*NodeParser)(TiXmlNode* node);
		std::map<std::string,NodeParser> root_map;

		/* Parse a file */
		void parseInputFile(const std::string& file);

	public:

		XmlParser();

		/* Map of attributes of a XML elements */
		typedef std::map<std::string,std::string> AttribMap;

		class XmlAttributes {
			/* Required arguments */
			std::vector<std::string> required;
			/* Optional arguments */
			std::vector<std::string> optional;
		public:
			XmlAttributes(const std::vector<std::string>& required, const std::vector<std::string>& optional) : required(required), optional(optional) {/* */};
			/* Checks attributes from the user specified map, this will throw an exception if something is wrong */
			void checkAttributes(const AttribMap& attrib_map, const string& object);
			~XmlAttributes() {/* */};
		};

		template<class T>
		class AttributeValue {
			/* Attribute value */
			std::string attrib_name;
			/* Default value */
			T default_value;
			/* Required arguments */
			std::map<std::string,T> values;
			/* Throw exception */
			void throwException(const AttribMap& attrib_map, const std::string& msg) const;
		public:
			AttributeValue(const std::string& attrib_name, const T& default_value,
					       const std::map<std::string,T>& values = std::map<std::string,T>()) :
				attrib_name(attrib_name), default_value(default_value), values(values) {/* */};
			/* Checks attributes from the user specified map, this will throw an exception if something is wrong */
			T getValue(const AttribMap& attrib_map) const;
			/* Get value as a string */
			std::string getString(const AttribMap& attrib_map) const;
			~AttributeValue() {/* */};
		};

		~XmlParser() {/* */};
	};

	template<class T>
	T XmlParser::AttributeValue<T>::getValue(const AttribMap& attrib_map) const {
		AttribMap::const_iterator it_att = attrib_map.find(attrib_name);
		if(it_att != attrib_map.end()) {
			/* Found attribute, get value */
			std::string value = (*it_att).second;
			typename std::map<std::string,T>::const_iterator it_val = values.find(value);
			if(it_val != values.end() || values.size() == 0) {
				return (*it_val).second;
			} else {
				/* And also we should print the available options into the screen */
				std::string options = "";
				typename std::map<std::string,T>::const_iterator it_values = values.begin();
				for(; it_values != values.end() ; ++it_values)
					options += (*it_values).first + " ";
				std::string msg = "Bad value <" + value + "> for attribute *" + attrib_name + "* ( options are : " + options + ")";
				throwException(attrib_map,msg);
			}
		}
		/* Not found, return default value */
		return default_value;
	}

	template<class T>
	std::string XmlParser::AttributeValue<T>::getString(const AttribMap& attrib_map) const {
		AttribMap::const_iterator it_att = attrib_map.find(attrib_name);
		if(it_att != attrib_map.end()) {
			return (*it_att).second;
		}
		else
			return default_value;
	}

	template<class T>
	void XmlParser::AttributeValue<T>::throwException(const AttribMap& attrib_map, const std::string& msg) const {
		AttribMap::const_iterator it_att = attrib_map.find(attrib_name);
		std::string value = (*it_att).second;
		/* Bad value for attribute, throw an exception */
		std::vector<std::string> keywords;
		it_att = attrib_map.begin();
		for(; it_att != attrib_map.end() ; ++it_att) {
			keywords.push_back((*it_att).first);
			keywords.push_back((*it_att).second);
		}
		throw KeywordParserError(msg,keywords);
	}

	/* Returns a string map of attributes on a node */
	static inline XmlParser::AttribMap dump_attribs(TiXmlElement* pElement) {
		if ( !pElement ) return XmlParser::AttribMap(); /* No attributes */
		TiXmlAttribute* pAttrib=pElement->FirstAttribute();
		/* Map of attributes */
		XmlParser::AttribMap mapAttrib;
		while (pAttrib) {
			/* Get attribute and push it into the map */
			mapAttrib[pAttrib->Name()] = pAttrib->Value();
			pAttrib=pAttrib->Next();
		}
		/* Return map */
		return mapAttrib;
	}
} /* namespace Helios */

#endif /* XMLPARSER_H_ */
