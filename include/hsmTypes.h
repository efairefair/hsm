#ifndef HSMTYPES_H_INCLUDED
#define HSMTYPES_H_INCLUDED

#include <list>
#include <string>
#include <tuple>
#include <vector>
#include <atomic>

// types for hsm storage
// for now just a few types
//
typedef bool                              hsmBool;
typedef int64_t                           hsmInt;
typedef uint64_t                          hsmUint;
typedef std::string                       hsmString;
typedef std::atomic<hsmUint>              hsmAtomicUint;
typedef std::vector<hsmUint>              hsmPath;

// an enum for the above - ideally we'll get rid of this and just use templates
typedef enum {hsmBoolType, hsmIntType, hsmUintType, hsmStringType, hsmAtomicUintType, hsmPathType} hsmType;

typedef enum {vertical, horizontal} forwardLinkType;

typedef enum {hsmTxEdge, hsmRxEdge, hsmInternalEdge} hsmEdgeType;

class pdu;
class msg;
class state;
class forwardLink;
class predicate;
class codeFragment;
class edge;
class subMachine;
class hsm;

class adverb;

class rtTree;
class rtZone;
class rtFrame;

typedef bool (*predType)(rtFrame *);
typedef void (*codeType)(rtFrame *);

//class hsmBoolVariable;
//class hsmIntVariable;
//class hsmUIntVariable;
//class hsmStringVariable;
//class hsmListUIntVariable;

#endif // HSMTYPES_H_INCLUDED
