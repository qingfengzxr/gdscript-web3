#include "abievent.h"

ABIEvent* NewABIEvent(const String &name, const String &rawName, bool anonymous, ABIArguments inputs) {
    Vector<String> names;
    Vector<String> types;

    for (int64_t i = 0; i < inputs.size(); ++i) {
        if (inputs[i].name == "") {
            String tmp = vformat("arg%d", i);
            inputs.write[i].name = tmp;
		}
		// string representation
		String type_str = inputs[i].type;
        String name_str = type_str + " " + inputs[i].name;
        if (inputs[i].indexed) {
            name_str = type_str + " indexed " + inputs[i].name;
        }
        names.push_back(name_str);
		// sig representation
        types.push_back(type_str);
    }

    String str = "event " + rawName + "(" + join(names, ", ") + ")";
    String sig = rawName + "(" + join(types, ",") + ")";

    // calculate event id
    PackedByteArray id;
    id.resize(32);
    Vector<uint8_t> sig_bytes = sig.to_utf8_buffer();
    int res = eth_keccak256(id.ptrw(), sig_bytes.ptr(), sig_bytes.size());
	if ( res < 0 ) {
		ERR_PRINT("error in NewABIEvent() dealing with keccak256");
		return nullptr;
	}

    return memnew(ABIEvent(name, rawName, anonymous, inputs, str, sig, id));
}
