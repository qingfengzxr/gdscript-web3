#ifndef ABITYPE_H
#define ABITYPE_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/typed_array.h"
#include "core/variant/variant.h"
#include "core/string/char_utils.h"
#include "core/os/memory.h"
#include "modules/regex/regex.h"

#include <vector>
#include <iostream>
#include <cstdio>

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
	FunctionTy,
    // other type ...

	UnkonwnTy = -1
};

TypeKind string_to_typekind(const String &type_str);
String typekind_to_string(TypeKind type);

class ABIType : public RefCounted {
    GDCLASS(ABIType, RefCounted);

// protected:
//     static void _bind_methods();

public:
    ABIType();
    ~ABIType();

    Ref<ABIType> elem; // Use Ref<> to manage the lifecycle of objects
    int size;
    String string_kind;
    TypeKind kind; // TODO:  This field is not used at the moment
	String name;

    // Tuple relative fields
    String tuple_raw_name; // Raw struct name defined in source code, may be empty
    Vector<Ref<ABIType>> tuple_elems; // Type information of all tuple fields
    Vector<String> tuple_raw_names; // Raw field name of all tuple fields
    const std::type_info* tuple_type; // Underlying struct of the tuple (using typeid for type information); TODO: need to clear this field's usage

    // printf method to output the values of the structure
    void format_output() const;
    Vector<uint8_t> pack(const Variant& v) const;
    bool requires_length_prefix() const;
};

Ref<ABIType> NewABIType(const String& t, const String& internalType, const Vector<ABIArgumentMarshaling>& components);

// Helper function to check if a character is a letter or underscore
bool is_letter(char32_t c);

// isValidFieldName checks if a string is a valid (struct) field name or not.
bool isValidFieldName(const String &fieldName);

bool typeCheck(const Ref<ABIType> t, const Variant& v);
bool isDynamicType(const Ref<ABIType> &t);
int getTypeSize(const Ref<ABIType> &t);
Vector<uint8_t> packNum(const Variant& v);
Vector<uint8_t> packElement(const ABIType& t, const Variant& v);
Dictionary mapArgNamesToStructFields(const Vector<String>& tupleRawNames, const Variant& v);

#endif // ABITYPE_H
