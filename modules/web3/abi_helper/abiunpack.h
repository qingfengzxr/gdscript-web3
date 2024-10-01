#include "abitype.h"

// length_prefix_points_to interprets a 32 byte slice as an offset and then determines which indices to look to decode the type.
Error length_prefix_points_to(int index, const PackedByteArray& output, int& begin, int& length);

// tuple_points_to resolves the location reference for dynamic tuple.
Error tuple_points_to(int index, const PackedByteArray& output, int& begin);

Error for_tuple_unpack_to_dictionary(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result);

Error for_tuple_unpack_to_array(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result);

// for_each_unpack iteratively unpack elements.
Error for_each_unpack(const Ref<ABIType>& type, const PackedByteArray& output, int offset, int length, Variant& result, int unpack_type);

// read_integer reads the integer based on its kind and returns the appropriate value.
Error read_integer(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result);

// read_bool reads a bool.
Error read_bool(const PackedByteArray& output, Variant& result);

// // read_fixed_bytes uses reflection to create a fixed array to be read from.
Error read_fixed_bytes(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result);

// A function type is simply the address with the function selection signature at the end.
//
// read_function_type enforces that standard by always presenting it as a 24-array (address + sig = 24 bytes)
Error read_function_type(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result);

// int unpack_type: 0 - dictionary, 1 - array
Dictionary to_cpp_type(int index, const Ref<ABIType>& t, const PackedByteArray& output, int unpack_type);

