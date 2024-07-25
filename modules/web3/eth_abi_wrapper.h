#ifndef ETH_ABI_WRAPPER_H
#define ETH_ABI_WRAPPER_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "eth_abi/abi.h"
#include <iostream>


class EthABIWrapper : public RefCounted {
	GDCLASS(EthABIWrapper, RefCounted);

protected:
	static void _bind_methods();

	float float_plus(float a, float b);
	PackedByteArray encode(int a);
	int decode(const String& a);

	uint8_t hexCharToUint8(char c);
	void hexStringToPackedByteArray(const char* hex, PackedByteArray& result);
	char* packedByteArrayToHexString(const PackedByteArray& a);

public:
	EthABIWrapper();
	~EthABIWrapper();
};

#endif
