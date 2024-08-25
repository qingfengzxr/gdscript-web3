#ifndef ABIARGUMENT_H
#define ABIARGUMENT_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include <vector>
#include "abiargument.h"

// 假设Type是一个枚举或者类，需要根据实际情况定义
enum TypeKind {
    // 示例类型
    IntType,
    StringType,
    BoolType
    // 其他类型...
};

struct Type {
    String string_kind;
    TypeKind kind;
    int size;
    bool isDynamic;
};

// Argument holds the name of the argument and the corresponding type.
// Types are used when packing and testing arguments.
struct Argument {
    String name;
    Type type;
    bool indexed; // indexed is only used by events
};

struct ABIArgumentMarshaling {
	String name;
	String type;
	String internalType;
	Vector<ABIArgumentMarshaling> components;
	bool   indexed;

    // Logic variable use to pack or unpack, it was not from abi.json.
    // But usefull for unpack abi or pack abi.
    // TODO
    String string_kind;
    TypeKind kind;
    int size;
    bool isDynamic;

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

PackedByteArray abiarguments_pack(const ABIArguments& args) {
    // TODO: implement this function

    return PackedByteArray();
}



#endif // ABIARGUMENT_H
