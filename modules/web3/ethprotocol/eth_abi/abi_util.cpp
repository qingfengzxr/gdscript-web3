#include "abi_util.h"

uint8_t hexCharToUint8(char c) {
	if (std::isdigit(c)) {
		return c - '0';
	}
	else if (std::isalpha(c)) {
		return std::tolower(c) - 'a' + 10;
	}
	return 0;
}

void hexStringToPackedByteArray(const char* hex, PackedByteArray& result) {
	size_t length = strlen(hex);
	size_t byteArraySize = length / 2;

	uint8_t* byteArray = new uint8_t[byteArraySize];
	for (size_t i = 0; i < byteArraySize; ++i) {
		byteArray[i] = (hexCharToUint8(hex[2 * i]) << 4) | hexCharToUint8(hex[2 * i + 1]);
	}

	result.resize(byteArraySize);
	memcpy(result.ptrw(), byteArray, byteArraySize);

	delete[] byteArray;
}

PackedByteArray uint8PtrToPackedByteArray(const uint8_t* data, size_t size) {
	PackedByteArray array;
	array.resize(size);
	uint8_t* ptr = array.ptrw();
	memcpy(ptr, data, size);
	return array;
}

char* packedByteArrayToHexString(const PackedByteArray& a) {
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

int getArrayArgSize(const char* argName) {
	std::string input(argName);
	std::size_t start_pos = input.find('[');
	std::size_t end_pos = input.find(']');

	if (start_pos != std::string::npos && end_pos != std::string::npos && end_pos > start_pos) {
		return std::stoi(input.substr(start_pos + 1, end_pos - start_pos - 1));
	}
	else {
		return -1;
	}
}


String join(const Vector<String>& elements, const String& delimiter) {
    String result;
    for (size_t i = 0; i < elements.size(); ++i) {
        result += elements[i];
        if (i < elements.size() - 1) {
            result += delimiter;
        }
    }
    return result;
}

String ResolveNameConflict(const String& rawName, const std::function<bool(const String&)>& used) {
    String name = rawName;
    bool ok = used(name);
    for (int idx = 0; ok; idx++) {
        name = rawName + String::num(idx);
        ok = used(name);
    }
    return name;
}