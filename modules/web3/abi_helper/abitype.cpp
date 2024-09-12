#include "abitype.h"
#include <iostream>
#include "abiargument.h"  // 确保包含了 abiargument.h 文件

TypeKind string_to_typekind(const String &type_str) {
    static const Dictionary type_map = []() {
        Dictionary map;
        map["int"] = IntTy;
        map["uint"] = UIntTy;
        map["bool"] = BoolTy;
        map["address"] = AddressTy;
        map["string"] = StringTy;
        map["bytes"] = BytesTy;
        map["tuple"] = TupleTy;
        map["function"] = FunctionTy;
        return map;
    }();

    if (type_map.has(type_str)) {
        return static_cast<TypeKind>(int(type_map[type_str]));
    } else {
        // Handle unknown type string, you can throw an exception or return a default value
        ERR_PRINT_ED("Unknown type string: " + String(type_str.utf8().get_data()));
    }
}

// printf method implementation
void ABIType::printf() const {
    std::cout << "ABIType {" << std::endl;
    std::cout << "  elem: " << (elem ? elem->string_kind.utf8().get_data() : "nullptr") << "," << std::endl;
    std::cout << "  size: " << size << "," << std::endl;
    std::cout << "  kind: " << kind << "," << std::endl;
    std::cout << "  string_kind: " << string_kind.utf8().get_data() << "," << std::endl;
    std::cout << "  tuple_raw_name: " << tuple_raw_name.utf8().get_data() << "," << std::endl;
    std::cout << "  tuple_elems: [";
    for (int i = 0; i < tuple_elems.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << tuple_elems[i]->string_kind.utf8().get_data();
    }
    std::cout << "]," << std::endl;
    std::cout << "  tuple_raw_names: [";
    for (int i = 0; i < tuple_raw_names.size(); ++i) {
        if (i > 0) std::cout << ", ";
        std::cout << tuple_raw_names[i].utf8().get_data();
    }
    std::cout << "]," << std::endl;
    std::cout << "  tuple_type: " << (tuple_type ? tuple_type->name() : "nullptr") << std::endl;
    std::cout << "}" << std::endl;
}

RegEx typeRegex("([a-zA-Z]+)(([0-9]+)(x([0-9]+))?)?");
RegEx sliceSizeRegex("[0-9]+");

ABIType NewABIType(const String& t, const String& internalType, const Vector<ABIArgumentMarshaling>& components) {
    ABIType typ;

    // Check that array brackets are equal if they exist
    if (t.count("[") != t.count("]")) {
        ERR_FAIL_V_MSG(ABIType(), "invalid arg type in abi");
    }
    typ.string_kind = t;

    // If there are brackets, get ready to go into slice/array mode and recursively create the type
    if (t.count("[") != 0) {
        String subInternal = internalType;
        int i = internalType.rfind("[");
        if (i != -1) {
            subInternal = subInternal.substr(0, i);
        }

        // Recursively embed the type
        i = t.rfind("[");
        ABIType embeddedType = NewABIType(t.substr(0, i), subInternal, components);

        // Grab the last cell and create a type from there
        String sliced = t.substr(i);
        Ref<RegExMatch> match = sliceSizeRegex.search(sliced);
        Vector<String> intz;
        if (match.is_valid()) {
            intz.push_back(match->get_string(0));
        }

        if (intz.size() == 0) {
            // Is a slice
            typ.kind = SliceTy;
            typ.elem = memnew(ABIType(embeddedType));
            typ.string_kind = embeddedType.string_kind + sliced;
        } else if (intz.size() == 1) {
            // Is an array
            typ.kind = ArrayTy;
            typ.elem = memnew(ABIType(embeddedType));
            typ.size = intz[0].to_int();
            typ.string_kind = embeddedType.string_kind + sliced;
        } else {
            ERR_FAIL_V_MSG(ABIType(), "invalid formatting of array type");
        }
        return typ;
    }

    // Parse the type and size of the abi-type
    TypedArray<RegExMatch> typed_matches = typeRegex.search_all(t);
    if (typed_matches.size() == 0) {
        ERR_FAIL_V_MSG(ABIType(), "invalid type '" + t + "'");
    }

    Ref<RegExMatch> match = typed_matches[0];
    if (!match.is_valid()) {
        ERR_FAIL_V_MSG(ABIType(), "invalid match for type '" + t + "'");
    }

    String parsedType = match->get_string(1);
    print_line("parsedType: " + parsedType);
    int varSize = 0;

    print_line("Match length: " + itos(match->get_string(2).length()));
    if (match->get_string(2).length() > 0) {
        varSize = match->get_string(2).to_int();
    } else {
        if (parsedType == "uint" || parsedType == "int") {
            ERR_FAIL_V_MSG(ABIType(), "unsupported arg type: " + t);
        }
    }

    // varType is the parsed abi type
    if (parsedType == "int") {
        typ.size = varSize;
        typ.kind = IntTy;
    } else if (parsedType == "uint") {
        typ.size = varSize;
        typ.kind = UIntTy;
    } else if (parsedType == "bool") {
        typ.kind = BoolTy;
    } else if (parsedType == "address") {
        typ.size = 20;
        typ.kind = AddressTy;
    } else if (parsedType == "string") {
        typ.kind = StringTy;
    } else if (parsedType == "bytes") {
        if (varSize == 0) {
            typ.kind = BytesTy;
        } else {
            if (varSize > 32) {
                ERR_FAIL_V_MSG(ABIType(), "unsupported arg type: " + t);
            }
            typ.kind = FixedBytesTy;
            typ.size = varSize;
        }
    } else if (parsedType == "tuple") {
        Vector<ABIType*> elems;
        Vector<String> names;
        String expression = "(";
        Dictionary used;

        for (int idx = 0; idx < components.size(); ++idx) {
            const ABIArgumentMarshaling& c = components[idx];
            ABIType cType = NewABIType(c.type, c.internalType, c.components);

            String name = c.name;
            if (name == "") {
                ERR_FAIL_V_MSG(ABIType(), "abi: purely anonymous or underscored field is not supported");
            }
            String fieldName = ResolveNameConflict(name, [&](const String &s) { return used.has(s); });
            used[fieldName] = true;

            // TODO: 这里需要再关注一下实现，go那边是反射，因此需要。C++是否也需要呢？
            if (!isValidFieldName(fieldName)) {
                ERR_FAIL_V_MSG(ABIType(), "field " + itos(idx) + " has invalid name");
            }

            elems.push_back(memnew(ABIType(cType)));
            names.push_back(c.name);
            expression += cType.string_kind;
            if (idx != components.size() - 1) {
                expression += ",";
            }
        }
        expression += ")";

        typ.tuple_type = &typeid(ABIType);
        typ.tuple_elems = elems;
        typ.tuple_raw_names = names;
        typ.kind = TupleTy;
        typ.string_kind = expression;

        const String structPrefix = "struct ";
        // After solidity 0.5.10, a new field of abi "internalType"
        // is introduced. From that we can obtain the struct name
        // user defined in the source code.
        if (internalType != "" && internalType.begins_with(structPrefix)) {
            // Foo.Bar type definition is not allowed in golang,
            // convert the format to FooBar
            typ.tuple_raw_name = internalType.substr(structPrefix.length()).replace(".", "");
        }
    } else if (parsedType == "function") {
        typ.kind = FunctionTy;
        typ.size = 24;
    } else {
        ERR_FAIL_V_MSG(ABIType(), "unsupported arg type: " + t);
    }

    return typ;
}

bool isDynamicType(const ABIType &t) {
    if (t.kind == TupleTy) {
        for (int i = 0; i < t.tuple_elems.size(); ++i) {
            if (isDynamicType(*t.tuple_elems[i])) {
                return true;
            }
        }
        return false;
    }
    return t.kind == StringTy || t.kind == BytesTy || t.kind == SliceTy || (t.kind == ArrayTy && isDynamicType(*t.elem));
}

int getTypeSize(const ABIType &t) {
    if (t.kind == ArrayTy && !isDynamicType(t)) {
        // 如果它是一个嵌套数组，则递归计算类型大小
        if (t.elem->kind == ArrayTy || t.elem->kind == TupleTy) {
            return t.size * getTypeSize(*t.elem);
        }
        return t.size * 32;
    } else if (t.kind == TupleTy && !isDynamicType(t)) {
        int total = 0;
        for (int i = 0; i < t.tuple_elems.size(); ++i) {
            total += getTypeSize(*t.tuple_elems[i]);
        }
        return total;
    }
    return 32;
}

bool is_letter(char32_t c) {
    return (c == '_') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c >= 0x80 && c <= 0x10FFFF);
}

bool isValidFieldName(const String &fieldName) {
    if (fieldName.length() == 0) {
        return false;
    }

    for (int i = 0; i < fieldName.length(); ++i) {
        char32_t c = fieldName[i];
        if (i == 0 && !is_letter(c)) {
            return false;
        }

        if (!(is_letter(c) || is_digit(c))) {
            return false;
        }
    }

    return true;
}


