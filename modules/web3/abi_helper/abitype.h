#ifndef ABITYPE_H
#define ABITYPE_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "core/string/char_utils.h"
#include "modules/regex/regex.h"
#include <vector>
#include "abi_util.h"

// 前向声明 ABIArgumentMarshaling 类型
struct ABIArgumentMarshaling;

// Used to convert and map with abi
enum TypeKind {
	IntTy,
	UintTy,
	BoolTy,
	StringTy,
	SliceTy,
	ArrayTy,
	TupleTy,
	AddressTy,
	FixedBytesTy,
	BytesTy,
	HashTy,
	FixedPointTy,
	FunctionTy
    // other type ...
};

TypeKind string_to_typekind(const String &type_str);

struct ABIType {
    ABIType* elem;
    int size;
    String string_kind;
    TypeKind kind; // TODO: 暂时没有用到这个字段

    // Tuple relative fields
    String tuple_raw_name; // Raw struct name defined in source code, may be empty
    Vector<ABIType*> tuple_elems; // Type information of all tuple fields
    Vector<String> tuple_raw_names; // Raw field name of all tuple fields
    const std::type_info* tuple_type; // Underlying struct of the tuple (using typeid for type information); TODO: 需要再明确一下这个字段的作用

    // Constructor with initialization list
    ABIType() : elem(nullptr), size(0), kind(IntTy), tuple_type(nullptr) {}

    // printf method to output the values of the structure
    void printf() const;
};

ABIType NewABIType(const String& t, const String& internalType, const Vector<ABIArgumentMarshaling>& components);

// Helper function to check if a character is a letter or underscore
bool is_letter(char32_t c);

// isValidFieldName checks if a string is a valid (struct) field name or not.
bool isValidFieldName(const String &fieldName);

#endif // ABITYPE_H
