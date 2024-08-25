#ifndef ABIEVENT_H
#define ABIEVENT_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"

#include "abiargument.h"

struct ABIEvent {
	// Name is the event name used for internal representation. It's derived from
	// the raw name and a suffix will be added in the case of event overloading.
	//
	// e.g.
	// These are two events that have the same name:
	// * foo(int,int)
	// * foo(uint,uint)
	// The event name of the first one will be resolved as foo while the second one
	// will be resolved as foo0.
    String name;

    // RawName is the raw event name parsed from ABI.
    String raw_name;
    bool anonymous;
    ABIArguments inputs;
    String str;

	// Sig contains the string signature according to the ABI spec.
	// e.g.	 event foo(uint32 a, int b) = "foo(uint32,int256)"
	// Please note that "int" is substitute for its canonical representation "int256"
    String sig;

	// ID returns the canonical representation of the event's signature used by the
	// abi definition to identify event names and types.
    PackedByteArray id;

    ABIEvent(const String& name_, const String& raw_name_, bool anonymous_, const ABIArguments& inputs_, const String& str_, const String& sig_, const PackedByteArray& id_)
        : name(name_), raw_name(raw_name_), anonymous(anonymous_), inputs(inputs_), str(str_), sig(sig_), id(id_) {}
};

// NewEvent creates a new Event.
// It sanitizes the input arguments to remove unnamed arguments.
// It also precomputes the id, signature and string representation
// of the event.
inline ABIEvent* NewABIEvent(const String &name, const String &rawName, bool anonymous, ABIArguments inputs) {
    Vector<String> names;
    Vector<String> types;

	// TODO: Event will be implemented in the future.
	// just some dummy code to make it compile
    String str = "event " + name + "(";
    String sig = name + "(";
    for (int64_t i = 0; i < types.size(); ++i) {
        if (i > 0) {
            str += ",";
            sig += ",";
        }
        str += types[i] + " " + names[i];
        sig += types[i];
    }
    str += ")";
    sig += ")";
	PackedByteArray id;

	return memnew(ABIEvent(name, rawName, anonymous, inputs, str, sig, id));
}

#endif // ABIEVENT_H
