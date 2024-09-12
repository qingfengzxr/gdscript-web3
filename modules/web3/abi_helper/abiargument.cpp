#include "abiargument.h"
#include <iostream>

void Argument::printf() const {
    std::cout << "Argument {" << std::endl;
    std::cout << "  name: " << name.utf8().get_data() << "," << std::endl;
    std::cout << "  type: ";
    type.printf();
    std::cout << "  indexed: " << (indexed ? "true" : "false") << std::endl;
    std::cout << "}" << std::endl;
}

Argument convert_to_argument(const ABIArgumentMarshaling &abi_arg) {
    Argument arg;
    arg.name = abi_arg.name;
    arg.type = NewABIType(abi_arg.type, abi_arg.internalType, abi_arg.components);
    arg.indexed = abi_arg.indexed;
    return arg;
}

PackedByteArray abiarguments_pack(const ABIArguments& args) {
    // TODO: implement this function

    return PackedByteArray();
}
