#ifndef ABIARGUMENT_H
#define ABIARGUMENT_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include <vector>
#include "abitype.h"


// Argument holds the name of the argument and the corresponding type.
// Types are used when packing and testing arguments.
struct Argument {
    String name;
    ABIType type;
    bool indexed; // indexed is only used by events

    void printf() const;
};

struct ABIArgumentMarshaling {
	String name;
	String type;
	String internalType;
	Vector<ABIArgumentMarshaling> components;
	bool   indexed;

    // Logic variable use to pack or unpack, it was not from abi.json.
    // But usefull for unpack abi or pack abi.
    // TODO: 做个转换，每个ABIArgumentMarshaling都有一个Argument, Argument才是实际的存储用于abi编解码参数的数据结构
    Argument argument;

	void unmarshal(const Dictionary &dict) {
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
    }
};

using ABIArguments = Vector<ABIArgumentMarshaling>;


Argument convert_to_argument(const ABIArgumentMarshaling &abi_arg);

PackedByteArray abiarguments_pack(const ABIArguments& args);

#endif // ABIARGUMENT_H
