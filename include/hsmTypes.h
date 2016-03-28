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
typedef uint64_t                          hsmUint;
typedef std::string                       hsmString;
typedef std::atomic<hsmUint>              hsmAtomicUint;
typedef std::vector<hsmUint>              hsmPath;

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
class rtTree;
class rtZone;
class rtFrame;

typedef bool (*predType)(rtFrame *);    // predicates - boolean functions
typedef void (*codeType)(rtFrame *);    // code fragments - void functions

//class hsmBoolVariable;
//class hsmIntVariable;
//class hsmUIntVariable;
//class hsmStringVariable;
//class hsmListUIntVariable;

#endif // HSMTYPES_H_INCLUDED
