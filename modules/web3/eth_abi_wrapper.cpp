#include "eth_abi_wrapper.h"


EthABIWrapper::EthABIWrapper() {

}

EthABIWrapper::~EthABIWrapper() {

}


void EthABIWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("encode"), &EthABIWrapper::encode);
	ClassDB::bind_method(D_METHOD("decode"), &EthABIWrapper::decode);
	ClassDB::bind_method(D_METHOD("encode_call", "func_name", "args"), &EthABIWrapper::encode_call);
	ClassDB::bind_method(D_METHOD("decode_call", "byte_value", "args"), &EthABIWrapper::decode_call);
}

PackedByteArray EthABIWrapper::encode(const String& type, Variant value) {
	if (type == "int") {
		return eth_abi_encode_int(value);
	}
	else if (type == "bool") {
		return eth_abi_encode_bool(value);
	}
	else if (type == "bytes") {
		return eth_abi_encode_bytes(value);
	}
	else if (type == "address") {
		return eth_abi_encode_address(value);
	}
	else if (type.ends_with("[]")) {
		return eth_abi_encode_array(value, type);
	}
}

Variant EthABIWrapper::decode(const String& type, const String& value) {

	if (type == "int") {
		return eth_abi_decode_int(value);
	}
	else if (type == "bool") {
		return eth_abi_decode_bool(value);
	}
	else if (type == "bytes") {
		return eth_abi_decode_bytes(value);
	}
	else if (type == "address") {
		return eth_abi_decode_address(value);
	}
	else if (type.ends_with("[]")) {
		return eth_abi_decode_array(value, type);
	}
}

// =========================== INT ===========================


PackedByteArray EthABIWrapper::eth_abi_encode_int(Variant value) {
	struct eth_abi data;
	size_t hexlen;
	char* hex;
	int64_t _value = (int64_t)value;
	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_int64(&data, &_value);
	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);
	PackedByteArray result;
	hexStringToPackedByteArray(hex, result);
	return result;
}


Variant EthABIWrapper::eth_abi_decode_int(const String& value) {
	struct eth_abi data;
	int64_t res;
	CharString utf8Str = value.utf8();
	size_t length = utf8Str.length();
	char* hex = new char[length + 1];
	std::copy(utf8Str.get_data(), utf8Str.get_data() + length, hex);
	hex[length] = '\0';

	eth_abi_from_hex(&data, hex, -1);
	eth_abi_int64(&data, &res);
	eth_abi_free(&data);
	int result = static_cast<int>(res);
	delete[] hex;
	return Variant(result);
}

// =========================== BOOL ===========================

PackedByteArray EthABIWrapper::eth_abi_encode_bool(Variant value) {
	struct eth_abi data;
	size_t hexlen;
	char* hex;
	uint8_t _value = (uint8_t)value;
	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_bool(&data, &_value);
	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);
	PackedByteArray result;
	hexStringToPackedByteArray(hex, result);
	return result;
}

Variant EthABIWrapper::eth_abi_decode_bool(const String& value) {
	struct eth_abi data;
	uint8_t res;
	CharString utf8Str = value.utf8();
	size_t length = utf8Str.length();
	char* hex = new char[length + 1];
	std::copy(utf8Str.get_data(), utf8Str.get_data() + length, hex);
	hex[length] = '\0';

	eth_abi_from_hex(&data, hex, -1);
	eth_abi_bool(&data, &res);
	eth_abi_free(&data);
	bool result = static_cast<int>(res);
	delete[] hex;
	return Variant(result);
}

// =========================== ADDRESS ===========================

PackedByteArray EthABIWrapper::eth_abi_encode_address(Variant value) {
	struct eth_abi data;
	size_t hexlen;
	char* hex;
	String valueStr = String(value);
	if (!valueStr.begins_with("0x")) {
		valueStr = "0x" + valueStr;
	}
	const char* valueConst = valueStr.utf8().get_data();
	char* _value = new char[strlen(valueConst) + 1];
	strcpy(_value, valueConst);

	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_address(&data, &_value);
	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);
	PackedByteArray result;
	hexStringToPackedByteArray(hex, result);
	return result;
}

Variant EthABIWrapper::eth_abi_decode_address(const String& value) {
	struct eth_abi data;
	//size_t hexlen;
	char* res;
	const char* valueConst = value.utf8().get_data();
	char* hex = new char[strlen(valueConst) + 1];
	strcpy(hex, valueConst);

	eth_abi_from_hex(&data, hex, -1);
	eth_abi_address(&data, &res);
	eth_abi_free(&data);
	delete[] hex;
	Variant result = Variant(String(res));
	return result;
}

// =========================== BYTES ===========================

PackedByteArray EthABIWrapper::eth_abi_encode_bytes(const PackedByteArray& value) {
	/*
	PACKED_BYTE_ARRAY
	*/
	struct eth_abi data;
	char* hex;
	size_t hexlen;
	const uint8_t* valueConst = value.ptr();
	size_t size = value.size();
	uint8_t* _value = new uint8_t[size];
	memcpy(_value, valueConst, size);
	uint8_t** __value = &_value;

	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_bytes(&data, __value, &size);
	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);
	PackedByteArray result;
	hexStringToPackedByteArray(hex, result);
	delete[] _value;
	delete[] hex;
	return result;
}

PackedByteArray EthABIWrapper::eth_abi_decode_bytes(const String& value) {
	struct eth_abi data;
	uint8_t *res;
	const char* valueConst = value.utf8().get_data();
	size_t size = value.utf8().size();
	char* hex = new char[strlen(valueConst) + 1];
	strcpy(hex, valueConst);

	eth_abi_from_hex(&data, hex, -1);
	eth_abi_bytes(&data, &res, &size);
	eth_abi_free(&data);

	PackedByteArray result = uint8PtrToPackedByteArray(res, size);

	//std::cout << "BYTES DECODE BYTE: " << result << std::endl;

	delete[] hex;
	return result;
}

// =========================== ARRAY ===========================

PackedByteArray EthABIWrapper::eth_abi_encode_array(Variant value, const String& type) {
	struct eth_abi data;
	size_t hexlen;
	char* hex;

	uint8_t typeCode = 0;
	if (type.begins_with("int")) {
		typeCode = 1;
	}
	else if (type.begins_with("bool")) {
		typeCode = 2;
	}

	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_array(&data, NULL);
	if (value.get_type() == Variant::ARRAY) {
		Array arr = value;
		for (int i = 0; i < arr.size(); i++) {
			if (typeCode == 1) {
				int64_t val = (int64_t)arr[i];
				eth_abi_int64(&data, &val);
			}
			else if (typeCode == 2) {
				uint8_t val = (uint8_t)arr[i];
				eth_abi_uint8(&data, &val);
			}
		}
	}
	eth_abi_array_end(&data);

	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);

	PackedByteArray result;
	hexStringToPackedByteArray(hex, result);
	return result;
}


Variant EthABIWrapper::eth_abi_decode_array(const String& value, const String& type) {

	struct eth_abi data;
	CharString utf8Str = value.utf8();
	size_t length = utf8Str.length();
	char* hex = new char[length + 1];
	std::copy(utf8Str.get_data(), utf8Str.get_data() + length, hex);
	hex[length] = '\0';
	uint64_t len;
	int arraySize = (int)(length / 64) - 2;

	int64_t convertArray[arraySize];
	eth_abi_from_hex(&data, hex, -1);
	eth_abi_array(&data, &len);
	for (int i = 0; i < arraySize; i++) {
		eth_abi_int64(&data, &convertArray[i]);
		std::cout << "DEC ARRAY " << i << "th ITME: " << convertArray[i] << std::endl;
	}
	eth_abi_array_end(&data);
	eth_abi_free(&data);

	Array resultArray;
	for (size_t i = 0; i < arraySize; ++i) {
		resultArray.append(convertArray[i]);
	}

	delete[] hex;
	return Variant(resultArray);
}

// =========================== CALL ===========================

PackedByteArray EthABIWrapper::encode_call(const String& funcName, const Array& args)
{
	struct eth_abi data;
	size_t hexlen;
	char* hex;

	const char* funcNameConst = funcName.utf8().get_data();
	char* funcNameStr = new char[strlen(funcNameConst) + 1];
	strcpy(funcNameStr, funcNameConst);

	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_call(&data, &funcNameStr, NULL);

	for (int i = 0; i < args.size(); i++) {
		Variant item = args[i];
		String str = String(item);
		//std::cout << "CALL ENC " << i << "th item: " << str.utf8().get_data() << std::endl;
		if (item.get_type() == Variant::INT) {
			int64_t value = (int64_t)item;
			eth_abi_int64(&data, &value);
		}
		else if (item.get_type() == Variant::BOOL) {
			uint8_t value = (uint8_t)item;
			eth_abi_bool(&data, &value);
		}
		else if (item.get_type() == Variant::STRING) {
			String itemStr = String(item);
			const char* itemStrConst = itemStr.utf8().get_data();
			char* itemVal = new char[strlen(itemStrConst) + 1];
			strcpy(itemVal, itemStrConst);
			eth_abi_address(&data, &itemVal);
		}
		else if (item.get_type() == Variant::ARRAY) {
			eth_abi_array(&data, NULL);
			Array arr = item;
			for (int ii = 0; ii < arr.size(); ii++) {
				if (arr[ii].get_type() == Variant::INT) {
					int64_t val = (int64_t)arr[ii];
					eth_abi_int64(&data, &val);
				}
				else if (arr[ii].get_type() == Variant::BOOL) {
					uint8_t val = (uint8_t)arr[ii];
					eth_abi_uint8(&data, &val);
				}
			}
			eth_abi_array_end(&data);
		}
	}

	eth_abi_call_end(&data);

	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);

	PackedByteArray result;
	hexStringToPackedByteArray(hex, result);
	return result;
}

Variant EthABIWrapper::decode_call(const String& value, const Array& args)
{
	/*
	The first 8 characters are the function selector, followed by the parameters in order.
	*/
	struct eth_abi data;
	CharString utf8Str = value.utf8();
	size_t length = utf8Str.length();

	char* funcSelector = new char[9];
	char* hex = new char[length - 8 + 1];

	Array resultArray;

	std::copy(utf8Str.get_data() + 8, utf8Str.get_data() + length, hex);
	hex[length - 8] = '\0';
	std::copy(utf8Str.get_data(), utf8Str.get_data() + 8, funcSelector);
	funcSelector[8] = '\0';
	//std::cout << "SELECTOR: " << funcSelector << std::endl;
	//std::cout << "CALL DEC HEX: " << hex << std::endl;

	resultArray.append(Variant(String(funcSelector)));

	eth_abi_from_hex(&data, hex, -1);
	std::string hexStr(hex);
	int accumCount = 0;
	for (int i = 0; i < args.size(); i++) {
		String argName = String(args[i]);
		if (argName.ends_with("]")) {
			int arraySize = getArrayArgSize(argName.utf8().get_data());
			char* argType;
			if (argName.contains("int")) {
				argType = "int";
			}
			else if (argName.contains("bool")) {
				argType = "bool";
			}
			if (arraySize > 0 && argType != NULL) {
				std::string subHex = hexStr.substr(accumCount, accumCount + 64 * (arraySize + 2));
				subHex[62] = '2';
				//std::cout << "DEC CALL ARRAY SIZE: " << arraySize << ", HEX: " << subHex << std::endl;
				String subHexStr = String(subHex.c_str());
				Variant res = eth_abi_decode_array(subHexStr, argType);
				resultArray.append(res);
				accumCount += 64 * (arraySize + 2);
			}
		}
		else if (argName == "int") {
			std::string subHex = hexStr.substr(accumCount, accumCount + 64);
			String subHexStr = String(subHex.c_str());
			Variant res = eth_abi_decode_int(subHexStr);
			resultArray.append(res);
			//std::cout << "CALL DEC " << i << "th INT value: " << String(res).utf8().get_data() << std::endl;
			accumCount += 64;
		}
		else if (argName == "bool") {
			std::string subHex = hexStr.substr(accumCount, accumCount + 64);
			String subHexStr = String(subHex.c_str());
			Variant res = eth_abi_decode_bool(subHexStr);
			resultArray.append(res);
			//std::cout << "CALL DEC " << i << "th BOOL value: " << String(res).utf8().get_data() << std::endl;
			accumCount += 64;
		}
		else if (argName == "address") {
			std::string subHex = hexStr.substr(accumCount, accumCount + 64);
			String subHexStr = String(subHex.c_str());
			Variant res = eth_abi_decode_address(subHexStr);
			resultArray.append(res);
			//std::cout << "CALL DEC " << i << "th STR value: " << String(res).utf8().get_data() << std::endl;
			accumCount += 64;
		}
	}
	eth_abi_free(&data);
	delete[] hex;
	Variant result = Variant(resultArray);
	return result;
}







