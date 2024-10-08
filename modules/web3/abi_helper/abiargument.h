#ifndef ABIARGUMENT_H
#define ABIARGUMENT_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include <vector>
#include <iostream>
#include "abitype.h"


// Argument holds the name of the argument and the corresponding type.
// Types are used when packing and testing arguments.
struct Argument {
    String name;
    Ref<ABIType> type;
    bool indexed; // indexed is only used by events

    void format_output() const;
};


struct ABIArgumentMarshaling {
	String name;
	String type;
	String internalType;
	Vector<ABIArgumentMarshaling> components;
	bool   indexed;

    // Logic variable use to pack or unpack, it was not from abi.json.
    // But usefull for unpack abi or pack abi.
	// Each ABIArgumentMarshaling has an Argument.
	// Argument is the actual data structure used for ABI encoding and decoding parameters.
    Argument argument;

	void unmarshal(const Dictionary &dict);
};

using ABIArguments = Vector<ABIArgumentMarshaling>;


Argument fill_argument(const ABIArgumentMarshaling &abi_arg);

ABIArguments non_indexed(const ABIArguments& args);
Dictionary abiarguments_pack(const ABIArguments& abiargs, const Array& args);
// Error unpack_abiarguments_into_dictionary(const ABIArguments& abiargs, const PackedByteArray& data, Dictionary& result);
// Error unpack_abiarguments_into_array(const ABIArguments& abiargs, const PackedByteArray& data, Array& result);
Error unpack_abiarguments(const ABIArguments& abiargs, const PackedByteArray& data, Variant& result);
Error unpack_abiarguments_values(const ABIArguments& abiargs, const PackedByteArray& data, Variant& result);

#endif // ABIARGUMENT_H
