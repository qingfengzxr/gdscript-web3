#include "abimethod.h"


ABIMethod* NewABIMethod(const String& name, const String& rawName, FunctionType funType, const String& mutability, bool isConst, bool isPayable, const ABIArguments& inputs, const ABIArguments& outputs) {
    Vector<String> types;
    Vector<String> inputNames;
    Vector<String> outputNames;

    for (const auto& input : inputs) {
        inputNames.push_back(input.argument.type->string_kind + " " + input.name);
        types.push_back(input.argument.type->string_kind);
    }

    for (const auto& output : outputs) {
        String outputName = output.argument.type->string_kind;
        if (output.name != "") {
            outputName += " " + output.name;
        }
        outputNames.push_back(outputName);
    }

    String sig;
    Vector<uint8_t> id;
    if (funType == Function) {
        sig = rawName + "(" + join(types, ",") + ")";
		uint8_t hash[32];

		PackedByteArray data = sig.to_utf8_buffer();
		const uint8_t* data_ptr = data.ptr();

        int res = eth_keccak256(hash, data_ptr, data.size());
        if ( res < 0 ) {
            ERR_PRINT("error in NewMethod() dealing with keccak256");
            return nullptr;
        }

        // use the first 4 bytes as the id
        for (int i = 0; i < 4; ++i) {
            id.push_back(hash[i]);
        }
    }

    String identity = "function " + rawName;
    switch (funType) {
        case Fallback: identity = "fallback"; break;
        case Receive: identity = "receive"; break;
        case Constructor: identity = "constructor"; break;
        default: break;
    }

    String str;
    if (mutability == "" || mutability == "nonpayable") {
        str = identity + "(" + join(inputNames, ", ") + ") returns(" + join(outputNames, ", ") + ")";
    } else {
        str = identity + "(" + join(inputNames, ", ") + ") " + mutability + " returns(" + join(outputNames, ", ") + ")";
    }

    return memnew(ABIMethod(name, rawName, funType, mutability, isConst, isPayable, inputs, outputs, str, sig, id));
}

void ABIMethod::format_output() const {
    print_line("ABIMethod {");
    print_line("  name: " + name + ",");
    print_line("  raw_name: " + raw_name + ",");
    print_line("  type: " + String::num(type) + ",");
    print_line("  state_mutability: " + state_mutability + ",");
    print_line("  constant: " + String(constant ? "true" : "false") + ",");
    print_line("  payable: " + String(payable ? "true" : "false") + ",");
    print_line("  inputs: [");
    for (int i = 0; i < inputs.size(); ++i) {
        inputs[i].format_output();
    }
    print_line("  ],");
    print_line("  outputs: [");
    for (int i = 0; i < outputs.size(); ++i) {
        outputs[i].format_output();
    }
    print_line("  ],");
    print_line("  str: " + str + ",");
    print_line("  sig: " + sig + ",");
    print_line("  id: " + String(packedByteArrayToHexString(id)) + ",");
    print_line("}");
}
