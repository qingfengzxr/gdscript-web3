#include "abiunpack.h"

// Error length_prefix_points_to(int index, const PackedByteArray& output, int& begin, int& length) {
//     ERR_FAIL_COND_V_MSG(index + 32 > output.size(), ERR_INVALID_DATA,
//         vformat("abi: length_prefix_points_to: offset would go over slice boundary. index: %d, output size: %d",
// 			index, output.size()));

//     // Extract 32 bytes of offset
//     mpz_t offset;
//     mpz_init(offset);
//     mpz_import(offset, 32, 1, 1, 0, 0, output.ptr() + index);
//     mpz_add_ui(offset, offset, 32);

//     // Check if the offset exceeds the boundary of the output
//     mpz_t output_size;
//     mpz_init_set_ui(output_size, output.size());
//     if (mpz_cmp(offset, output_size) > 0) {
//         ERR_PRINT(vformat("abi: length_prefix_points_to: offset [%d] would go over slice boundary. outputsize: [%d]",
// 								offset., output_size));
//         mpz_clear(offset);
//         mpz_clear(output_size);
//         return ERR_INVALID_DATA;
//     }

//     // Check if the offset exceeds the maximum value of int64
//     if (mpz_cmp_ui(offset, INT64_MAX) > 0) {
//         ERR_PRINT("abi: offset larger than int64");
//         mpz_clear(offset);
//         mpz_clear(output_size);
//         return ERR_INVALID_DATA;
//     }

//     int offset_end = mpz_get_ui(offset);

//     // Extract length information
//     mpz_t length_big;
//     mpz_init(length_big);
//     mpz_import(length_big, 32, 1, 1, 0, 0, output.ptr() + offset_end - 32);

//     mpz_t total_size;
//     mpz_init(total_size);
//     mpz_add(total_size, offset, length_big);

//     // Check if the total size exceeds the boundary of the output
//     if (mpz_cmp(total_size, output_size) > 0) {
//         ERR_PRINT("abi: cannot marshal into cpp type: length insufficient");
//         mpz_clear(offset);
//         mpz_clear(output_size);
//         mpz_clear(length_big);
//         mpz_clear(total_size);
//         return ERR_INVALID_DATA;
//     }

//     // Check if the total size exceeds the maximum value of int64
//     if (mpz_cmp_ui(total_size, INT64_MAX) > 0) {
//         ERR_PRINT("abi: length larger than int64");
//         mpz_clear(offset);
//         mpz_clear(output_size);
//         mpz_clear(length_big);
//         mpz_clear(total_size);
//         return ERR_INVALID_DATA;
//     }

//     begin = offset_end;
//     length = mpz_get_ui(length_big);

//     mpz_clear(offset);
//     mpz_clear(output_size);
//     mpz_clear(length_big);
//     mpz_clear(total_size);

//     return OK;
// }

Error length_prefix_points_to(int index, const PackedByteArray& output, int& begin, int& length) {
    ERR_FAIL_COND_V_MSG(index + 32 > output.size(), ERR_INVALID_DATA,
        vformat("abi: length_prefix_points_to: offset would go over slice boundary. index: %d, output size: %d",
                index, output.size()));

    // Extract 32 bytes of offset
    int64_t offset = 0;
    std::memcpy(&offset, output.ptr() + index, 32);
    offset += 32;

    if (offset > output.size()) {
        ERR_PRINT(vformat("abi: length_prefix_points_to: offset [%d] would go over slice boundary. output size: [%d]",
                          offset, output.size()));
        return ERR_INVALID_DATA;
    }

    if (offset > INT64_MAX) {
        ERR_PRINT("abi: offset larger than int64");
        return ERR_INVALID_DATA;
    }

    int64_t offset_end = offset;

    // Extract length from the output
    int64_t length_big = 0;
    std::memcpy(&length_big, output.ptr() + offset_end - 32, 32);

    int64_t total_size = offset + length_big;

    if (total_size > INT64_MAX) {
        ERR_PRINT("abi: length larger than int64");
        return ERR_INVALID_DATA;
    }

    if (total_size > output.size()) {
        ERR_PRINT(vformat("abi: cannot marshal into go type: length insufficient %d require %d", output.size(), total_size));
        return ERR_INVALID_DATA;
    }

    begin = static_cast<int>(offset);
    length = static_cast<int>(length_big);

    return OK;
}

Error tuple_points_to(int index, const PackedByteArray& output, int& begin) {
    ERR_FAIL_COND_V_MSG(index + 32 > output.size(), ERR_INVALID_DATA,
        "abi: cannot marshal into cpp slice: offset would go over slice boundary");

    // Extract 32 bytes of offset
    PackedByteArray offset_bytes;
    offset_bytes.resize(32);
    memset(offset_bytes.ptrw(), 0, 32);
    for (int i = 0; i < 32; ++i) {
        offset_bytes.write[i] = output[index + i];
    }

    // Convert the offset to a 64 bits integer
    int64_t offset = 0;
    for (int i = 0; i < 32; ++i) {
        offset = (offset << 8) | offset_bytes[i];
    }

    // Check if the offset exceeds the boundary of the output
    ERR_FAIL_COND_V_MSG(offset > output.size(), ERR_INVALID_DATA,
		vformat("abi: cannot marshal into cpp slice: offset %d would go over slice boundary (len=%d)",
			offset, output.size())
	);

    // Check if the offset exceeds the maximum value of int64
    ERR_FAIL_COND_V_MSG(offset > INT64_MAX, ERR_INVALID_DATA, "abi offset larger than int64");

    begin = static_cast<int>(offset);
    return OK;
}

Error for_tuple_unpack_to_dictionary(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result) {
    ERR_FAIL_COND_V_MSG(type->kind != TupleTy, ERR_INVALID_PARAMETER, "abi: invalid type in call to unpack tuple");

    Dictionary retval;
    int virtual_args = 0;

    for (int index = 0; index < type->tuple_elems.size(); ++index) {
        Ref<ABIType> elem = type->tuple_elems[index];
		String raw_name = type->tuple_raw_names[index];
        Variant marshalled_value;
        Dictionary ret = to_cpp_type((index + virtual_args) * 32, elem, output, 0);
        if (static_cast<Error>(int(ret["error"])) != OK) {
			Error err = static_cast<Error>(int(ret["error"]));
            return err;
        }
		marshalled_value = ret["result"];

        #ifdef DEBUG_ENABLED_WED3_MODULE
		if (marshalled_value.get_type() == Variant::ARRAY) {
			Array arr = marshalled_value;
			for (int i = 0; i < arr.size(); i++) {
				print_line(vformat("[debug] for_tuple_unpack_to_dictionary, marshalled_value[%d]: %s\n", i, static_cast<String>(arr[i]).utf8().get_data()));
			}
		} else {
			String str = marshalled_value;
			print_line(vformat("[debug] for_tuple_unpack_to_dictionary, str marshalled_value: %s\n", str.utf8().get_data()));
		}
        #endif

        if (elem->kind == ArrayTy && !isDynamicType(*elem)) {
            virtual_args += getTypeSize(elem) / 32 - 1;
        } else if (elem->kind == TupleTy && !isDynamicType(*elem)) {
            virtual_args += getTypeSize(elem) / 32 - 1;
        }

        retval[raw_name] = marshalled_value;
    }

    result = retval;
    return OK;
}

Error for_tuple_unpack_to_array(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result) {
    ERR_FAIL_COND_V_MSG(type->kind != TupleTy, ERR_INVALID_PARAMETER, "abi: invalid type in call to unpack tuple");

    Array retval;
    int virtual_args = 0;

    for (int index = 0; index < type->tuple_elems.size(); ++index) {
        Ref<ABIType> elem = type->tuple_elems[index];
        Variant marshalled_value;
        Dictionary ret = to_cpp_type((index + virtual_args) * 32, elem, output, 1);
        if (static_cast<Error>(int(ret["error"])) != OK) {
			Error err = static_cast<Error>(int(ret["error"]));
            return err;
        }
		marshalled_value = ret["result"];

        #ifdef DEBUG_ENABLED_WED3_MODULE
            if (marshalled_value.get_type() == Variant::ARRAY) {
                Array arr = marshalled_value;
                for (int i = 0; i < arr.size(); i++) {
                    print_line(vformat("[debug] for_tuple_unpack_to_array, marshalled_value[%d]: %s\n", i, static_cast<String>(arr[i]).utf8().get_data()));
                }
            } else {
                String str = marshalled_value;
                print_line(vformat("[debug] for_tuple_unpack_to_array, str marshalled_value: %s\n", str.utf8().get_data()));
            }
        #endif

        if (elem->kind == ArrayTy && !isDynamicType(*elem)) {
            virtual_args += getTypeSize(elem) / 32 - 1;
        } else if (elem->kind == TupleTy && !isDynamicType(*elem)) {
            virtual_args += getTypeSize(elem) / 32 - 1;
        }

        retval.append(marshalled_value);
    }

    result = retval;
    return OK;
}

Error for_each_unpack(const Ref<ABIType>& type, const PackedByteArray& output, int offset, int length, Variant& result, int unpack_type) {
    ERR_FAIL_COND_V_MSG(length < 0, ERR_INVALID_PARAMETER,
        "cannot marshal input to array, size is negative");
    ERR_FAIL_COND_V_MSG(offset + 32 * length > output.size(), ERR_INVALID_DATA,
        "abi: cannot marshal into cpp array: offset would go over slice boundary");

    Array ref_array;

    switch (type->kind) {
        case SliceTy:
            ref_array.resize(length);
            break;
        case ArrayTy:
            ref_array.resize(length);
            break;
        default:
            ERR_PRINT("abi: invalid type in array/slice unpacking stage");
            return ERR_INVALID_DATA;
    }

    int elem_size = getTypeSize(type->elem);

    for (int i = offset, j = 0; j < length; i += elem_size, ++j) {
        Variant inter;
        Dictionary ret = to_cpp_type(i, type->elem, output, unpack_type);
		if (static_cast<Error>(int(ret["error"])) != OK) {
			Error err = static_cast<Error>(int(ret["error"]));
            ERR_PRINT(vformat("abi: error in unpacking array/slice. err: %d", err));
            return err;
        }
		inter = ret["result"];

        #ifdef DEBUG_ENABLED_WED3_MODULE
		if (inter.get_type() == Variant::INT) {
			int tmp = inter;
            print_line(vformat("[debug] for_each_unpack, inter: %d\n", tmp));
        } else {
			String str = inter;
            print_line(vformat("[debug] for_each_unpack, str inter: %s\n", str.utf8().get_data()));
		}
        #endif

        ref_array[j] = inter;
    }

    result = ref_array;
    return OK;
}

Error read_integer(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result) {
    ERR_FAIL_COND_V_MSG(type->kind != UintTy && type->kind != IntTy, ERR_INVALID_PARAMETER,
        "abi: invalid type in call to read integer");

    ERR_FAIL_COND_V_MSG(output.size() < 32, ERR_INVALID_DATA,
        "abi: output size is less than 32 bytes");

    uint64_t u64 = 0;
    int64_t i64 = 0;
    for (int i = 0; i < 32; ++i) {
        u64 = (u64 << 8) | output[i];
        i64 = (i64 << 8) | output[i];
    }

    if (type->kind == UintTy) {
        switch (type->size) {
            case 8:
                if (u64 > UINT8_MAX) {
                    ERR_PRINT("abi: value exceeds uint8 max");
                    return ERR_INVALID_DATA;
                }
                result = static_cast<uint8_t>(u64);
                return OK;
            case 16:
                if (u64 > UINT16_MAX) {
                    ERR_PRINT("abi: value exceeds uint16 max");
                    return ERR_INVALID_DATA;
                }
                result = static_cast<uint16_t>(u64);
                return OK;
            case 32:
                if (u64 > UINT32_MAX) {
                    ERR_PRINT("abi: value exceeds uint32 max");
                    return ERR_INVALID_DATA;
                }
                result = static_cast<uint32_t>(u64);
                return OK;
            case 64:
                result = u64;
                return OK;
            default: {
                // the only case left for unsigned integer is uint256.
                mpz_t big_int;
                mpz_init(big_int);
                mpz_import(big_int, 32, 1, 1, 0, 0, output.ptr());
                result = String(mpz_get_str(nullptr, 10, big_int));
                mpz_clear(big_int);
                return OK;
            }
        }
    } else if (type->kind == IntTy) {
        if (output[0] & 0x80) { // Check if the number is negative
            for (int i = 0; i < 32; ++i) {
                i64 = (i64 << 8) | (~output[i] & 0xFF);
            }
            i64 = -(i64 + 1);
        }

        switch (type->size) {
            case 8:
                if (i64 < INT8_MIN || i64 > INT8_MAX) {
                    ERR_PRINT("abi: value exceeds int8 range");
                    return ERR_INVALID_DATA;
                }
                result = static_cast<int8_t>(i64);
                return OK;
            case 16:
                if (i64 < INT16_MIN || i64 > INT16_MAX) {
                    ERR_PRINT("abi: value exceeds int16 range");
                    return ERR_INVALID_DATA;
                }
                result = static_cast<int16_t>(i64);
                return OK;
            case 32:
                if (i64 < INT32_MIN || i64 > INT32_MAX) {
                    ERR_PRINT("abi: value exceeds int32 range");
                    return ERR_INVALID_DATA;
                }
                result = static_cast<int32_t>(i64);
                return OK;
            case 64:
                result = i64;
                return OK;
            default: {
                // the only case left for integer is int256
                // 处理 int256
                mpz_t big_int, max_uint256, one;
                mpz_init(big_int);
                mpz_import(big_int, 32, 1, 1, 0, 0, output.ptr());

                // 初始化 max_uint256 和 one
                mpz_init_set_str(max_uint256, "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF", 16);
                mpz_init_set_ui(one, 1);

                if (mpz_tstbit(big_int, 255)) { // 检查第 255 位是否为 1
                    mpz_neg(big_int, big_int);
                    mpz_add(big_int, big_int, max_uint256);
                    mpz_add(big_int, big_int, one);
                    mpz_neg(big_int, big_int);
                }

                result = String(mpz_get_str(nullptr, 10, big_int));

                // 清理
                mpz_clear(big_int);
                mpz_clear(max_uint256);
                mpz_clear(one);
                return OK;
            }
        }
    }

    return ERR_INVALID_PARAMETER;
}

Error read_bool(const PackedByteArray& output, Variant& result) {
    ERR_FAIL_COND_V_MSG(output.size() < 32, ERR_INVALID_DATA,
        "abi: output size is less than 32 bytes");

    for (int i = 0; i < 31; ++i) {
        if (output[i] != 0) {
            ERR_PRINT("abi: invalid bool value, non-zero byte found in first 31 bytes");
            return ERR_INVALID_DATA;
        }
    }

    switch (output[31]) {
        case 0:
            result = false;
            return OK;
        case 1:
            result = true;
            return OK;
        default:
            ERR_PRINT("abi: invalid bool value, last byte is not 0 or 1");
            return ERR_INVALID_DATA;
    }
}

Error read_fixed_bytes(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result) {
    ERR_FAIL_COND_V_MSG(type->kind != FixedBytesTy, ERR_INVALID_PARAMETER,
        "abi: invalid type in call to make fixed byte array");

    ERR_FAIL_COND_V_MSG(output.size() < type->size, ERR_INVALID_DATA,
        "abi: output size is less than required fixed byte size");

    PackedByteArray fixed_bytes;
    fixed_bytes.resize(type->size);
    memset(fixed_bytes.ptrw(), 0, type->size);
    for (int i = 0; i < type->size; ++i) {
        fixed_bytes.write[i] = output[i];
    }

    result = fixed_bytes;
    return OK;
}

Error read_function_type(const Ref<ABIType>& type, const PackedByteArray& output, Variant& result) {
    ERR_FAIL_COND_V_MSG(type->kind != FunctionTy, ERR_INVALID_PARAMETER,
        "abi: invalid type in call to make function type byte array");

    ERR_FAIL_COND_V_MSG(output.size() < 32, ERR_INVALID_DATA,
        "abi: output size is less than 32 bytes");

    uint64_t garbage = 0;
    for (int i = 24; i < 32; ++i) {
        garbage = (garbage << 8) | output[i];
    }

    ERR_FAIL_COND_V_MSG(garbage != 0, ERR_INVALID_DATA,
        vformat("abi: got improperly encoded function type, garbage %d", garbage));

    PackedByteArray func_ty;
    func_ty.resize(24);
    memset(func_ty.ptrw(), 0, 24);
    for (int i = 0; i < 24; ++i) {
        func_ty.set(i, output[i]);
    }

    result = func_ty;
    return OK;
}

Dictionary to_cpp_type(int index, const Ref<ABIType>& t, const PackedByteArray& output, int unpack_type) {
    Dictionary ret;
    ret["error"] = OK;
    ret["result"] = Variant();

    if (index + 32 > output.size()) {
        ERR_PRINT(vformat("abi: cannot marshal into cpp type: length insufficient ", output.size(), " require ", index + 32));
        ret["error"] = ERR_INVALID_DATA;
        return ret;
    }

    PackedByteArray return_output;
    int begin = 0, length = 0;
    Error err = OK;

    // If a length prefix is required, find the starting position and the returned size
    if (t->requires_length_prefix()) {
        err = length_prefix_points_to(index, output, begin, length);
        if (err != OK) {
            ERR_PRINT(vformat("abi: to_cpp_type length_prefix_points_to failed. err: %d", err));
            ret["error"] = err;
            return ret;
        }
    } else {
        return_output = output.slice(index, index + 32);
    }

    Variant result;

    switch (t->kind) {
        case TupleTy:
            #ifdef DEBUG_ENABLED_WED3_MODULE
                print_line(vformat("[debug] type.name %s, TupleTy\n", t->string_kind.utf8().get_data()));
            #endif

            if (isDynamicType(t)) {
                err = tuple_points_to(index, output, begin);
                if (err != OK) {
                    ERR_PRINT(vformat("abi: to_cpp_type tuple_points_to failed. err: %d", err));
                    ret["error"] = err;
                    return ret;
                }
                if (unpack_type == 0) {
                    err = for_tuple_unpack_to_dictionary(t, output.slice(begin, output.size()), result);
                } else {
                    err = for_tuple_unpack_to_array(t, output.slice(begin, output.size()), result);
                }
            } else {
                if (unpack_type == 0) {
                    err = for_tuple_unpack_to_dictionary(t, output.slice(index, output.size()), result);
                } else {
                    err = for_tuple_unpack_to_array(t, output.slice(index, output.size()), result);
                }
            }
            break;
        case SliceTy:
            #ifdef DEBUG_ENABLED_WED3_MODULE
                print_line(vformat("[debug] type.name %s, SliceTy\n", t->string_kind.utf8().get_data()));
            #endif

            err = for_each_unpack(t, output.slice(begin, output.size()), 0, length, result, unpack_type);
            break;
        case ArrayTy:
            #ifdef DEBUG_ENABLED_WED3_MODULE
                print_line(vformat("[debug] type.name %s, ArrayTy\n", t->string_kind.utf8().get_data()));
            #endif

            if (isDynamicType(t->elem)) {
                uint64_t offset = 0;
                for (int i = 0; i < 8; ++i) {
                    offset = (offset << 8) | return_output[24 + i];
                }
                if (offset > static_cast<uint64_t>(output.size())) {
                    ERR_PRINT(vformat("abi: to_cpp_type offset greater than output length: offset: ", offset, ", len(output): ", output.size()));
                    ret["error"] = ERR_INVALID_DATA;
                    return ret;
                }
                err = for_each_unpack(t, output.slice(offset, output.size()), 0, t->size, result, unpack_type);
            } else {
                err = for_each_unpack(t, output.slice(index, output.size()), 0, t->size, result, unpack_type);
            }
            break;
        case StringTy:
            result = String::utf8((const char*)output.slice(begin, begin + length).ptr(), length);

            #ifdef DEBUG_ENABLED_WED3_MODULE
                print_line(vformat("[debug] to_cpp_type, StringTy result: %s\n", static_cast<String>(result).utf8().get_data()));
            #endif

            err = OK;
            break;
        case IntTy:
        case UintTy:
            #ifdef DEBUG_ENABLED_WED3_MODULE
                print_line(vformat("[debug] type.name %s, IntTy, UintTy\n", t->string_kind.utf8().get_data()));
            #endif

            err = read_integer(t, return_output, result);
            break;
        case BoolTy:
            err = read_bool(return_output, result);
            break;
        case AddressTy:
            {
                // Address represents the 20 byte address of an Ethereum account.
                PackedByteArray address_bytes = return_output.slice(12, return_output.size());
                result = String::hex_encode_buffer(address_bytes.ptr(), address_bytes.size());
                err = OK;
            }
            break;
        case HashTy:
            result = String::hex_encode_buffer(return_output.ptr(), return_output.size());
            err = OK;
            break;
        case BytesTy:
            result = output.slice(begin, begin + length);
            err = OK;
            break;
        case FixedBytesTy:
            err = read_fixed_bytes(t, return_output, result);
            break;
        case FunctionTy:
            err = read_function_type(t, return_output, result);
            break;
        default:
            ERR_PRINT(vformat("abi: unknown type ", t->kind));
            ret["error"] = ERR_INVALID_DATA;
            return ret;
    }

    if (err != OK) {
        ret["error"] = err;
    } else {
        ret["result"] = result;
    }

    return ret;
}
