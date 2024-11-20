#include "abitype.h"
#include "abiargument.h"

TypeKind string_to_typekind(const String &type_str) {
    static const Dictionary type_map = []() {
        Dictionary map;
        map["int"] = IntTy;
        map["uint"] = UintTy;
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
	return UnkonwnTy;
}

String typekind_to_string(TypeKind type) {
    static const Dictionary type_map = []() {
        Dictionary map;
        map[IntTy] = "int";
        map[UintTy] = "uint";
        map[BoolTy] = "bool";
        map[AddressTy] = "address";
        map[StringTy] = "string";
        map[BytesTy] = "bytes";
        map[TupleTy] = "tuple";
        map[FunctionTy] = "function";
        return map;
    }();

    if (type_map.has(type)) {
        return type_map[type];
    } else {
        ERR_PRINT("Unknown TypeKind: " + itos(type));
        return "unknown"; // default value
    }
}

ABIType::ABIType() {
    size = 0;
    kind = IntTy;
    tuple_type = nullptr;
}

ABIType::~ABIType() {
    // TODO
}

// printf method implementation
void ABIType::format_output() const {
    std::cout << "ABIType {" << std::endl;
    std::cout << "  elem: " << (elem.is_null() ? "nullptr" : elem->string_kind.utf8().get_data()) << "," << std::endl;
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

Ref<ABIType> NewABIType(const String& t, const String& internalType, const Vector<ABIArgumentMarshaling>& components) {
    Ref<ABIType> typ = Ref<ABIType>(memnew(ABIType));

    // Check that array brackets are equal if they exist
    if (t.count("[") != t.count("]")) {
        ERR_FAIL_V_MSG(Ref<ABIType>(), "invalid arg type in abi");
    }
    typ->string_kind = t;

    // If there are brackets, get ready to go into slice/array mode and recursively create the type
    if (t.count("[") != 0) {
        String subInternal = internalType;
        int i = internalType.rfind("[");
        if (i != -1) {
            subInternal = subInternal.substr(0, i);
        }

        // Recursively embed the type
        i = t.rfind("[");
        Ref<ABIType> embeddedType = NewABIType(t.substr(0, i), subInternal, components);

        // Grab the last cell and create a type from there
        String sliced = t.substr(i);
        Ref<RegExMatch> match = sliceSizeRegex.search(sliced);
        Vector<String> intz;
        if (match.is_valid()) {
            intz.push_back(match->get_string(0));
        }

        if (intz.size() == 0) {
            // Is a slice
            typ->kind = SliceTy;
            typ->elem = embeddedType;
            typ->string_kind = embeddedType->string_kind + sliced;
            if (typ->elem == nullptr) {
                ERR_FAIL_V_MSG(Ref<ABIType>(), "Memory allocation for typ.elem slice failed");
            }
        } else if (intz.size() == 1) {
            // Is an array
            typ->kind = ArrayTy;
            typ->elem = embeddedType;
            if (typ->elem == nullptr) {
                ERR_FAIL_V_MSG(Ref<ABIType>(), "Memory allocation for typ.elem array failed");
            }
            typ->size = intz[0].to_int();
            typ->string_kind = embeddedType->string_kind + sliced;
        } else {
            ERR_FAIL_V_MSG(Ref<ABIType>(), "invalid formatting of array type");
        }
        return typ;
    }

    // Parse the type and size of the abi-type
    TypedArray<RegExMatch> typed_matches = typeRegex.search_all(t);
    if (typed_matches.size() == 0) {
        ERR_FAIL_V_MSG(Ref<ABIType>(), "invalid type '" + t + "'");
    }

    Ref<RegExMatch> match = typed_matches[0];
    if (!match.is_valid()) {
        ERR_FAIL_V_MSG(Ref<ABIType>(), "invalid match for type '" + t + "'");
    }

    String parsedType = match->get_string(1);
    int varSize = 0;

    if (match->get_string(2).length() > 0) {
        varSize = match->get_string(2).to_int();
    } else {
        if (parsedType == "uint" || parsedType == "int") {
            ERR_FAIL_V_MSG(Ref<ABIType>(), "unsupported arg type: " + t);
        }
    }

    // varType is the parsed abi type
    if (parsedType == "int") {
        typ->size = varSize;
        typ->kind = IntTy;
    } else if (parsedType == "uint") {
        typ->size = varSize;
        typ->kind = UintTy;
    } else if (parsedType == "bool") {
        typ->kind = BoolTy;
    } else if (parsedType == "address") {
        typ->size = 20;
        typ->kind = AddressTy;
    } else if (parsedType == "string") {
        typ->kind = StringTy;
    } else if (parsedType == "bytes") {
        if (varSize == 0) {
            typ->kind = BytesTy;
        } else {
            if (varSize > 32) {
                ERR_FAIL_V_MSG(Ref<ABIType>(), "unsupported arg type: " + t);
            }
            typ->kind = FixedBytesTy;
            typ->size = varSize;
        }
    } else if (parsedType == "tuple") {
        Vector<Ref<ABIType>> elems;
        Vector<String> names;
        String expression = "(";
        Dictionary used;

        for (int idx = 0; idx < components.size(); ++idx) {
            const ABIArgumentMarshaling& c = components[idx];
            Ref<ABIType> cType = NewABIType(c.type, c.internalType, c.components);

            String name = c.name;
            if (name == "") {
                ERR_FAIL_V_MSG(Ref<ABIType>(), "abi: purely anonymous or underscored field is not supported");
            }
            String fieldName = ResolveNameConflict(name, [&](const String &s) { return used.has(s); });
            used[fieldName] = true;

            // TODO: Need to pay attention to the implementation here.
            // In Go, reflection is used, so it is necessary. Is it also needed in C++?
            if (!isValidFieldName(fieldName)) {
                ERR_FAIL_V_MSG(Ref<ABIType>(), "field " + itos(idx) + " has invalid name");
            }

            elems.push_back(cType);
            names.push_back(c.name);
            expression += cType->string_kind;
            if (idx != components.size() - 1) {
                expression += ",";
            }
        }
        expression += ")";

        typ->tuple_type = &typeid(ABIType);
        typ->tuple_elems = elems;
        typ->tuple_raw_names = names;
        typ->kind = TupleTy;
        typ->string_kind = expression;

        const String structPrefix = "struct ";
        // After solidity 0.5.10, a new field of abi "internalType"
        // is introduced. From that we can obtain the struct name
        // user defined in the source code.
        if (internalType != "" && internalType.begins_with(structPrefix)) {
            // Foo.Bar type definition is not allowed in golang,
            // convert the format to FooBar
            typ->tuple_raw_name = internalType.substr(structPrefix.length()).replace(".", "");
        }
    } else if (parsedType == "function") {
        typ->kind = FunctionTy;
        typ->size = 24;
    } else {
        ERR_FAIL_V_MSG(Ref<ABIType>(), "unsupported arg type: " + t);
    }

    return typ;
}

// typeErr returns a formatted type casting error.
Error typeErr(Variant::Type expected, Variant::Type actual) {
    ERR_PRINT(vformat("ABI: cannot use " + Variant::get_type_name(actual) + " as type " + Variant::get_type_name(expected) + " as argument"));
    return ERR_INVALID_PARAMETER;
}

// sliceTypeCheck checks that the given slice can by assigned to the reflection
// type in t.
bool sliceTypeCheck(const Ref<ABIType> t, const Variant& v) {
    if (v.get_type() != Variant::ARRAY) {
        typeErr(Variant::ARRAY, v.get_type());
        return false;
    }

    Array array = v;
    for (int i = 0; i < array.size(); ++i) {
        if (!typeCheck(t->elem, array[i])) {
            return false;
        }
    }

    return true;
}

// typeCheck checks that the given reflection value can be assigned to the reflection
// type in t.
bool typeCheck(const Ref<ABIType> t, const Variant& v) {
    if (t->kind == SliceTy || t->kind == ArrayTy) {
        return sliceTypeCheck(t, v);
    }

    // Check the validity of basic types
    Variant::Type variant_type = v.get_type();
    if (t->kind == IntTy && (variant_type != Variant::INT && variant_type != Variant::STRING)) {
        // BigInt we deal with convert from int or string
        typeErr(Variant::INT, variant_type);
        return false;
    } else if (t->kind == StringTy && variant_type != Variant::STRING) {
        typeErr(Variant::STRING, variant_type);
        return false;
    } else if (t->kind == BoolTy && variant_type != Variant::BOOL) {
        typeErr(Variant::BOOL, variant_type);
        return false;
    } else if (t->kind == FixedBytesTy && v.get_type() == Variant::PACKED_BYTE_ARRAY) {
        PackedByteArray byte_array = v;
        if (t->size != byte_array.size()) {
            typeErr(Variant::PACKED_BYTE_ARRAY, variant_type);
            return false;
        }
    } else if (t->kind == ArrayTy && v.get_type() == Variant::ARRAY) {
        Array array = v;
        for (int i = 0; i < array.size(); ++i) {
            if (!typeCheck(t->elem, array[i])) {
                return false;
            }
        }
    } else {
        return true;
    }

    return true;
}

bool isDynamicType(const Ref<ABIType> &t) {
    if (t->kind == TupleTy) {
        for (int i = 0; i < t->tuple_elems.size(); ++i) {
            if (isDynamicType(t->tuple_elems[i])) {
                return true;
            }
        }
        return false;
    }
    return t->kind == StringTy || t->kind == BytesTy || t->kind == SliceTy || (t->kind == ArrayTy && isDynamicType(t->elem));
}

int getTypeSize(const Ref<ABIType> &t) {
    if (t->kind == ArrayTy && !isDynamicType(t)) {
        // If it is a nested array, recursively calculate the type size
        if (t->elem->kind == ArrayTy || t->elem->kind == TupleTy) {
            return t->size * getTypeSize(t->elem);
        }
        return t->size * 32;
    } else if (t->kind == TupleTy && !isDynamicType(t)) {
        int total = 0;
        for (int i = 0; i < t->tuple_elems.size(); ++i) {
            total += getTypeSize(t->tuple_elems[i]);
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

// Right pad byte array to length l
PackedByteArray RightPadBytes(const PackedByteArray &slice, int l) {
    if (l <= slice.size()) {
        return slice;
    }

    PackedByteArray padded;
    padded.resize(l);
    memset(padded.ptrw(), 0, l);
    for (int i = 0; i < slice.size(); ++i) {
        padded.write[i] = slice[i];
    }
    return padded;
}

// Left pad byte array to length l
PackedByteArray LeftPadBytes(const PackedByteArray &slice, int l) {
    if (l <= slice.size()) {
        return slice;
    }

    PackedByteArray padded;
    padded.resize(l);
    memset(padded.ptrw(), 0, l);
    for (int i = 0; i < slice.size(); ++i) {
        padded.write[l - slice.size() + i] = slice[i];
    }

    return padded;
}

// encode_eth_address encodes an Ethereum address as a 32-byte slice.
PackedByteArray encode_eth_address(const String &eth_address) {
    String address = eth_address.strip_edges().replace("0x", "");

    ERR_FAIL_COND_V(address.length() != 40, PackedByteArray());

    PackedByteArray address_bytes;
    address_bytes.resize(20); // ETH address is 20 bytes
    memset(address_bytes.ptrw(), 0, 20);
    for (int i = 0; i < 20; ++i) {
        address_bytes.write[i] = static_cast<uint8_t>(strtol(address.substr(i * 2, 2).utf8().get_data(), nullptr, 16));
    }

    PackedByteArray padded_address = LeftPadBytes(address_bytes, 32);
    return padded_address;
}

// PaddingBigBytes encodes a big integer as a big-endian byte slice.
// The length of the byte slice is at least n bytes.
PackedByteArray PaddedBigBytes(const mpz_t &bigint, int n) {
    size_t byte_len = (mpz_sizeinbase(bigint, 2) + 7) / 8;

	// If the byte length of the big integer is greater than or equal to n,
	// directly return the byte array of the big integer
    if (byte_len >= static_cast<size_t>(n)) {
        size_t count;
        PackedByteArray result;
        result.resize(byte_len);
        memset(result.ptrw(), 0, byte_len);
        mpz_export(result.ptrw(), &count, 1, 1, 1, 0, bigint);
        return result;
    }

	// Otherwise, create a byte array of length n and fill it with the byte array of the big integer
    PackedByteArray result;
    result.resize(n);
    memset(result.ptrw(), 0, n);
    mpz_export(result.ptrw() + (n - byte_len), nullptr, 1, 1, 1, 0, bigint);
    return result;
}

String bytes_to_hex(const PackedByteArray &bytes) {
    String hex_str;
    for (int i = 0; i < bytes.size(); ++i) {
        hex_str += String::num_int64(bytes[i], 16).pad_zeros(2);
    }
    return hex_str;
}

// packNum packs the given number (using the reflect value) and will cast it to the
// appropriate number representation.
PackedByteArray packNum(const Variant& v) {
    PackedByteArray result;
    mpz_t value;
    mpz_init(value);

    if (v.get_type() == Variant::INT) {
        mpz_set_si(value, int64_t(v));
    } else if (v.get_type() == Variant::FLOAT) {
        mpz_set_d(value, double(v));
    } else if (v.get_type() == Variant::STRING) {
        // WARNING: Currently only supports decimal strings, hexadecimal is not supported.
        mpz_set_str(value, String(v).utf8().get_data(), 10);
    } else {
        ERR_PRINT("Unknown type for packNum");
        mpz_clear(value);
        return result;
    }

    // Resize result to 32 bytes and initialize with zeros
    result.resize(32);
    memset(result.ptrw(), 0, 32);

    // Export the value to the result array in big-endian order
    size_t count;
    mpz_export(result.ptrw() + (32 - (mpz_sizeinbase(value, 2) + 7) / 8), &count, 1, 1, 1, 0, value);

    mpz_clear(value);
    return result;
}

// packBytesSlice packs the given bytes as [L, V] as the canonical representation bytes slice.
PackedByteArray packBytesSlice(const PackedByteArray &bytes, int l) {
    PackedByteArray len = packNum(l);
    PackedByteArray paddedBytes = RightPadBytes(bytes, (l + 31) / 32 * 32);
    len.append_array(paddedBytes);
    return len;
}

// packElement packs the given value according to the abi specification in t.
PackedByteArray packElement(const ABIType& t, const Variant& v) {
    switch (t.kind) {
        case IntTy:
        case UintTy:
            return packNum(v);
        case StringTy: {
            PackedByteArray bytes;
            String str = v;
            PackedByteArray temp;
            temp.resize(str.utf8().length());
            memcpy(temp.ptrw(), str.utf8().get_data(), str.utf8().length());
            bytes.append_array(temp);
            return packBytesSlice(bytes, str.length());
        }
        case AddressTy: {
            String address = v;
            return encode_eth_address(address);
        }
        case BoolTy:
            return packNum(v ? 1 : 0);
        case BytesTy: {
            PackedByteArray bytes;
            if (v.get_type() == Variant::ARRAY) {
                Array arr = v;
                bytes.resize(arr.size());
                for (int i = 0; i < arr.size(); ++i) {
                    bytes.write[i] = arr[i];
                }
            } else if (v.get_type() != Variant::PACKED_BYTE_ARRAY) {
                ERR_PRINT("Bytes type is neither PackedByteArray nor Array");
                return PackedByteArray();
            }
            return packBytesSlice(bytes, bytes.size());
        }
        case FixedBytesTy:
        case FunctionTy: {
            PackedByteArray bytes;
            if (v.get_type() == Variant::ARRAY) {
                Array arr = v;
                bytes.resize(arr.size());
                for (int i = 0; i < arr.size(); ++i) {
                    bytes.write[i] = arr[i];
                }
            }

            return RightPadBytes(bytes, 32);
        }
        default:
            ERR_PRINT("Unknown type for packElement");
            return PackedByteArray();
    }
}

bool ABIType::requires_length_prefix() const {
    return kind == StringTy || kind == BytesTy || kind == SliceTy;
}

// FIXME: Complete the mapping of input parameters to Solidity struct fields.
// This function is **not complete** and may not be used in the current implementation.
Dictionary mapArgNamesToStructFields(const Vector<String>& argNames, const Variant& value) {
    Dictionary abi2struct;
    Dictionary struct2abi;

    for (const String& argName : argNames) {
        String structFieldName = argName.to_camel_case();

        if (structFieldName.is_empty()) {
            ERR_FAIL_V_MSG(Dictionary(), "abi: purely underscored output cannot unpack to struct");
        }

        if (abi2struct.has(argName)) {
            if (abi2struct[argName] != structFieldName &&
                !struct2abi.has(structFieldName) &&
                value.operator Dictionary().has(structFieldName)) {
                ERR_FAIL_V_MSG(Dictionary(), "abi: multiple variables maps to the same abi field '" + argName + "'");
            }
            continue;
        }

        if (struct2abi.has(structFieldName)) {
            ERR_FAIL_V_MSG(Dictionary(), "abi: multiple outputs mapping to the same struct field '" + structFieldName + "'");
        }

        if (value.get_type() == Variant::DICTIONARY && value.operator Dictionary().has(structFieldName)) {
            abi2struct[argName] = structFieldName;
            struct2abi[structFieldName] = argName;
        } else {
            struct2abi[structFieldName] = argName;
        }
    }

    return abi2struct;
}

Vector<uint8_t> ABIType::pack(const Variant& v) const {
    // Ref<ABIType> p = Ref<ABIType>(this);
    if (!typeCheck(Ref<ABIType>(this), v)) {
        ERR_FAIL_V_MSG(Vector<uint8_t>(), "Type check failed");
    }

    Vector<uint8_t> ret;

    switch (kind) {
        case SliceTy:
        case ArrayTy: {
            if (v.get_type() != Variant::ARRAY) {
                ERR_FAIL_V_MSG(Vector<uint8_t>(), "SliceTy, ArrayTy: Expected value to be an array");
            }
            Array value = v;

            if (requires_length_prefix()) {
                ret = packNum(value.size());
            }

            int offset = 0;
            bool offsetReq = isDynamicType(*elem);
            if (offsetReq) {
                offset = getTypeSize(elem) * int(value.size());
            }

            Vector<uint8_t> tail;
            for (int i = 0; i < int(value.size()); ++i) {
				Vector<uint8_t> val = elem->pack(value[i]);
                if (!offsetReq) {
                    ret.append_array(val);
                    continue;
                }
                ret.append_array(packNum(offset));
                offset += val.size();
                tail.append_array(val);
            }
            ret.append_array(tail);
            break;
        }
        case TupleTy: {
            if (v.get_type() != Variant::ARRAY) {
                ERR_FAIL_V_MSG(Vector<uint8_t>(), "TupleTy: Expected value to be an array");
            }
            Array value = v;

            // Map values to each specific ABIType
            // FIXME: If we don't use fieldmap for specific mapping, will it cause errors due to inconsistent struct field order?
            // Dictionary fieldmap = mapArgNamesToStructFields(tuple_raw_names, value);
            int offset = 0;
            for (const auto& te : tuple_elems) {
                offset += getTypeSize(te);
            }

            Vector<uint8_t> tail;
            for (int i = 0; i < tuple_elems.size(); ++i) {
                Variant field = value[i];

                Vector<uint8_t> val = tuple_elems[i]->pack(field);
                if (isDynamicType(*tuple_elems[i])) {
                    ret.append_array(packNum(offset));
                    tail.append_array(val);
                    offset += val.size();
                } else {
                    ret.append_array(val);
                }
            }
            ret.append_array(tail);
            break;
        }
        default:
            return packElement(*this, v);
    }

    return ret;
}
