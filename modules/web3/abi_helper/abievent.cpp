#include "abievent.h"

ABIEvent* NewABIEvent(const String &name, const String &rawName, bool anonymous, ABIArguments inputs) {
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
