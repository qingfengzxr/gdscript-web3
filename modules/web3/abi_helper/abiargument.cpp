#include "abiargument.h"
#include "abiunpack.h"

void Argument::format_output() const {
    print_line("Argument {");
    print_line("  name: " + String(name.utf8().get_data()) + ",");
    print_line("  type: ");
    type->format_output();
    print_line("  indexed: " + String(indexed ? "true" : "false"));
    print_line("}");
}

Argument fill_argument(const ABIArgumentMarshaling &abi_arg) {
    Argument arg;
    arg.name = abi_arg.name;
    arg.type = NewABIType(abi_arg.type, abi_arg.internalType, abi_arg.components);
    arg.indexed = abi_arg.indexed;
    return arg;
}

void ABIArgumentMarshaling::unmarshal(const Dictionary &dict) {
    name = dict.get("name", "");
    type = dict.get("type", "");
    internalType = dict.get("internalType", "");
    indexed = dict.get("indexed", false);

    if (dict.has("components")) {
        Array comps = dict["components"];
        for (int i = 0; i < comps.size(); ++i) {
            Dictionary compDict = comps[i];
            ABIArgumentMarshaling component;
            component.unmarshal(compDict);
            components.push_back(component);
        }
    }

    argument = fill_argument(*this);
}


ABIArguments non_indexed(const ABIArguments& args) {
    Vector<ABIArgumentMarshaling> ret;
    for (int i = 0; i < args.size(); ++i) {
        if (!args[i].indexed) {
            ret.push_back(args[i]);
        }
    }
    return ret;
}

PackedByteArray abiarguments_pack(const ABIArguments& abiargs, const Array& args) {
	// Ensure the number of passed arguments matches the number of arguments defined in the function
    if (args.size() != abiargs.size()) {
        ERR_FAIL_V_MSG(PackedByteArray(), "argument count mismatch: got " + itos(args.size()) + " for " + itos(abiargs.size()));
    }

	// variableInput is the output appended at the end of the packed output.
	// This is used for strings and bytes types input.
    PackedByteArray variableInput;

    // input offset is the bytes offset for packed output
    int inputOffset = 0;
    for (int i = 0; i < abiargs.size(); ++i) {
        inputOffset += getTypeSize(abiargs[i].argument.type);
    }

    PackedByteArray ret;
    for (int i = 0; i < args.size(); ++i) {
        const ABIArgumentMarshaling& input = abiargs[i];
        const Variant& a = args[i];

        // pack the input
        PackedByteArray packed = input.argument.type->pack(a);
        if (packed.size() == 0) {
            ERR_FAIL_V_MSG(PackedByteArray(), "packing error for argument " + itos(i));
        }

        #ifdef DEBUG_ENABLED_WED3_MODULE
            print_line(vformat("===> abiargument type: %s\n", input.name.utf8().get_data()));
            print_line(vformat("===> packed size: %d", packed.size()));
            print_line(vformat("===> packed: %s",  String::hex_encode_buffer(packed.ptr(), packed.size()).utf8().get_data()));
        #endif

        // check for dynamic types
        if (isDynamicType(input.argument.type)) {
            // set the offset
            ret.append_array(packNum(inputOffset));
            // calculate next offset
            inputOffset += packed.size();
            // append the packed value to the variable input
            variableInput.append_array(packed);
        } else {
            ret.append_array(packed);
        }
    }

    // append the variable input to the end of the packed output
    ret.append_array(variableInput);

    return ret;
}

Error unpack_abiarguments(const ABIArguments& abiargs, const PackedByteArray& data, Variant& result) {
    if (data.size() == 0) {
        if (abiargs.size() != 0) {
            ERR_PRINT("abi: attempting to unmarshal an empty string while arguments are expected");
            return ERR_INVALID_DATA;
        }
        return OK;
    }

    if (result.get_type() == Variant::DICTIONARY) {
        Dictionary marshalled_values;
        Variant marshalled_values_variant = marshalled_values;
        Error err = unpack_abiarguments_values(abiargs, data, marshalled_values_variant);
        if (err != OK) {
            return err;
        }
        marshalled_values = marshalled_values_variant;

        for (int i = 0; i < abiargs.size(); ++i) {
            ABIArgumentMarshaling arg = abiargs[i];
            String name = arg.name;
            if (name.is_empty()) {
                name = itos(i);
            }

            Dictionary result_dict = result;
            result_dict[name] = marshalled_values[name];
            result = result_dict; // update result
        }
    } else if (result.get_type() == Variant::ARRAY) {
        Array marshalled_values;
        Variant marshalled_values_variant = marshalled_values;
        Error err = unpack_abiarguments_values(abiargs, data, marshalled_values_variant);
        if (err != OK) {
            return err;
        }
        marshalled_values = marshalled_values_variant;

        for (int i = 0; i < abiargs.size(); ++i) {
            ABIArgumentMarshaling arg = abiargs[i];
            String name = arg.name;
            if (name.is_empty()) {
                name = itos(i);
            }

            Array result_array = result;
            result_array.append(marshalled_values[i]);
            result = result_array; // update result
        }
    } else {
        ERR_PRINT("param result is not a dictionary or array");
        return ERR_INVALID_PARAMETER;
    }

    return OK;
}

Error unpack_abiarguments_values(const ABIArguments& abiargs, const PackedByteArray& data, Variant& result) {
    ABIArguments non_indexed_args = non_indexed(abiargs);

    // Check if the data length is as expected
    ERR_FAIL_COND_V_MSG(data.size() % 32 != 0, ERR_INVALID_DATA, "Data length is not a multiple of 32");

    int unpack_type = 0;
    if (result.get_type() == Variant::ARRAY) {
        unpack_type = 1;
    }

    int virtual_args = 0;
    for (int index = 0; index < non_indexed_args.size(); ++index) {
        const ABIArgumentMarshaling &arg = non_indexed_args[index];
        Ref<ABIType> type = arg.argument.type;
        Variant marshalled_value;

        #ifdef DEBUG_ENABLED_WED3_MODULE
        	print_line(vformat("===> arg type: %s\n", arg.name.utf8().get_data()));
        #endif
        Dictionary ret = to_cpp_type((index + virtual_args) * 32, type, data, unpack_type);
		if (static_cast<Error>(int(ret["error"])) != OK) {
			Error err = static_cast<Error>(int(ret["error"]));
            return err;
        }
		marshalled_value = ret["result"];

        // Handle static arrays
        if (type->kind == ArrayTy && !isDynamicType(type)) {
            virtual_args += getTypeSize(type) / 32 - 1;
        } else if (type->kind == TupleTy && !isDynamicType(type)) {
            // Handle static tuples
            virtual_args += getTypeSize(type) / 32 - 1;
        }

        if (result.get_type() == Variant::DICTIONARY) {
            String name = arg.name;
            if (name.is_empty()) {
                name = itos(index);
            }

            Dictionary result_dict = result;
            result_dict[name] = marshalled_value;
            result = result_dict; // update result
        } else if (result.get_type() == Variant::ARRAY) {
            Array result_array = result;
            result_array.append(marshalled_value);
            result = result_array; // update result
        } else {
            ERR_PRINT("param result is not a dictionary or array");
            return ERR_INVALID_PARAMETER;
        }
    }

    return OK;
}
