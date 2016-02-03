#ifndef HSMTYPES_H_INCLUDED
#define HSMTYPES_H_INCLUDED

// HSM -- a library to create and execute Hierarchial State Machines
//
//    Copyright (C) 2014  Edwin R. Fair Jr.
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
//          email: ed_fair@yahoo.com
//          post:  5252 Kurt Ln, Conyers, GA US Postal Code: 30094
//

#include <list>
#include <string>
#include <tuple>
#include <vector>

// types for hsm storage types -+
//                              |
//                              V
typedef bool                hsmBool;
typedef int64_t             hsmInt;
typedef uint64_t            hsmUInt;
typedef std::string         hsmString;
typedef std::list<int64_t>  hsmListInt;

// an enum for the above
typedef enum {hsmBoolType, hsmIntType, hsmUIntType, hsmStringType,hsmListIntType} hsmVarType;

typedef enum {vertical, horizontal} forwardLinkType;

typedef hsmUInt soType;                                        // sibling order type
typedef std::vector<soType> absolutePathType;

typedef std::tuple<std::string, hsmVarType, size_t> pListEntryType;    // parameter list for edge declarations: variable name, type, and height above
typedef std::vector<pListEntryType> pListType;

typedef enum {hsmTxEdge, hsmRxEdge, hsmInternalEdge} hsmEdgeSignType;


class msg;
class state;
class forwardLink;
class predicate;
class codeFragment;
class edge;
class subMachine;
class hsm;
class variable;
class adverb;

class rtTree;
class rtZone;
class rtFrame;

typedef bool (*predType)(rtFrame *);
typedef void (*codeType)(rtFrame *);

#endif // HSMTYPES_H_INCLUDED
