#include "eth_abi_wrapper.h"
#include "eth_abi/abi.h"


EthABIWrapper::EthABIWrapper() {

}

EthABIWrapper::~EthABIWrapper() {

}


void EthABIWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("encode"), &EthABIWrapper::encode);
	ClassDB::bind_method(D_METHOD("decode"), &EthABIWrapper::decode);
}

PackedByteArray EthABIWrapper::encode(const String& type, Variant value) {
	if (type == "int") {
		return eth_abi_encode_int(value);
	}
	else if (type == "bool") {
		return eth_abi_encode_bool(value);
	}
}

Variant EthABIWrapper::decode(const String& type, const String& value) {
	if (type == "int") {
		return eth_abi_decode_int(value);
	}
	else if (type == "bool") {
		return eth_abi_decode_bool(value);
	}
}

PackedByteArray EthABIWrapper::eth_abi_encode_int(Variant value) {
	struct eth_abi data;
	size_t hexlen;
	char* hex;
	int64_t _value = (int64_t)value;
	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_int64(&data, &_value);
	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);
	std::cout << "INT64 HEX LEN: " << hexlen << ", HEX: " << *hex << ", " << hex << std::endl;
	PackedByteArray result;
	hexStringToPackedByteArray(hex, result);
	return result;
}

PackedByteArray EthABIWrapper::eth_abi_encode_bool(Variant value) {
	struct eth_abi data;
	size_t hexlen;
	char* hex;
	uint8_t _value = (uint8_t)value;
	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_bool(&data, &_value);
	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);
	std::cout << "BOOL HEX LEN: " << hexlen << ", HEX: " << *hex << ", " << hex << std::endl;
	PackedByteArray result;
	hexStringToPackedByteArray(hex, result);
	return result;
}

Variant EthABIWrapper::eth_abi_decode_int(const String& value) {
    struct eth_abi data;
    int64_t res;
    CharString utf8_str = value.utf8();
    size_t length = utf8_str.length();
    char* hex = new char[length + 1];
    std::copy(utf8_str.get_data(), utf8_str.get_data() + length, hex);
    hex[length] = '\0';

    eth_abi_from_hex(&data, hex, -1);
	eth_abi_int64(&data, &res);
    eth_abi_free(&data);
    int result = static_cast<int>(res);
    std::cout << "PACKED: " << hex << ", " << *hex << ", Int64=" << static_cast<int>(res) << ", int=" << result << std::endl;
    delete[] hex;
    return Variant(result);
}

Variant EthABIWrapper::eth_abi_decode_bool(const String& value) {
	struct eth_abi data;
	uint8_t res;
	CharString utf8_str = value.utf8();
	size_t length = utf8_str.length();
	char* hex = new char[length + 1];
	std::copy(utf8_str.get_data(), utf8_str.get_data() + length, hex);
	hex[length] = '\0';

	eth_abi_from_hex(&data, hex, -1);
	eth_abi_bool(&data, &res);
	eth_abi_free(&data);
	bool result = static_cast<int>(res);
	std::cout << "PACKED: " << hex << ", " << *hex << ", Int64=" << static_cast<int>(res) << ", int=" << result << std::endl;
	delete[] hex;
	return Variant(result);
}


uint8_t EthABIWrapper::hexCharToUint8(char c) {
	if (std::isdigit(c)) {
		return c - '0';
	}
	else if (std::isalpha(c)) {
		return std::tolower(c) - 'a' + 10;
	}
	return 0;
}

void EthABIWrapper::hexStringToPackedByteArray(const char* hex, PackedByteArray& result) {
	size_t length = strlen(hex);
	size_t byteArraySize = length / 2;  // 每两个字符转换为一个字节

	uint8_t* byteArray = new uint8_t[byteArraySize];
	for (size_t i = 0; i < byteArraySize; ++i) {
		byteArray[i] = (hexCharToUint8(hex[2 * i]) << 4) | hexCharToUint8(hex[2 * i + 1]);
	}

	result.resize(byteArraySize);
	memcpy(result.ptrw(), byteArray, byteArraySize);

	delete[] byteArray;
}

char* EthABIWrapper::packedByteArrayToHexString(const PackedByteArray& a) {
	static const char hex_digits[] = "0123456789abcdef";
	size_t len = a.size();
	char* hex = new char[len * 2 + 1];

	for (size_t i = 0; i < len; ++i) {
		hex[i * 2] = hex_digits[(a[i] >> 4) & 0x0F];
		hex[i * 2 + 1] = hex_digits[a[i] & 0x0F];
	}
	hex[len * 2] = '\0';

	return hex;
}
