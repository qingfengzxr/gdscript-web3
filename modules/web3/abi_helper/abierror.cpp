#include "abierror.h"

ABIError* NewABIError(const String &name, ABIArguments inputs) {
    Vector<String> names;
    Vector<String> types;

    for (int64_t i = 0; i < inputs.size(); ++i) {
        if (inputs[i].name == "") {
            String temp_name = vformat("arg%d", i);
            inputs.write[i].name = temp_name;
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

    String str = "error " + name + "(" + join(names, ", ") + ")";
    String sig = name + "(" + join(types, ",") + ")";

    // calculate event id
    PackedByteArray id;
    id.resize(32);
    Vector<uint8_t> sig_bytes = sig.to_utf8_buffer();
    int res = eth_keccak256(id.ptrw(), sig_bytes.ptr(), sig_bytes.size());
	if ( res < 0 ) {
		ERR_PRINT("error in NewABIError() dealing with keccak256");
		return nullptr;
	}

    return memnew(ABIError(name, inputs, str, sig, id));
}

Error ABIError::unpack(const PackedByteArray &data, Variant& result) const {
    if (data.size() < 4) {
        ERR_PRINT(vformat("insufficient data for unpacking: have %d, want at least 4", data.size()));
        return ERR_INVALID_PARAMETER;
    }

    PackedByteArray id_prefix = data.slice(0, 4);
    PackedByteArray expected_id_prefix = id.slice(0, 4);

    if (id_prefix != expected_id_prefix) {
        ERR_PRINT(vformat("invalid identifier, have %#x want %#x", id_prefix, expected_id_prefix));
        return ERR_INVALID_DATA;
    }

	return unpack_abiarguments(inputs, data.slice(4, data.size()), result);
}
