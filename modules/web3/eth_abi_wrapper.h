#ifndef ETH_ABI_WRAPPER_H
#define ETH_ABI_WRAPPER_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "eth_abi/abi.h"
#include <iostream>


class EthABIWrapper : public RefCounted {
	GDCLASS(EthABIWrapper, RefCounted);

public:
	EthABIWrapper();
	~EthABIWrapper();

protected:
	static void _bind_methods();

	PackedByteArray encode(const String& type, Variant value);

	Variant decode(const String& type, const String& value);


private:

	PackedByteArray eth_abi_encode_int(Variant value);
	Variant eth_abi_decode_int(const String& value);
	PackedByteArray eth_abi_encode_bool(Variant value);
	Variant eth_abi_decode_bool(const String& value);

	uint8_t hexCharToUint8(char c);

	void hexStringToPackedByteArray(const char* hex, PackedByteArray& result);

	char* packedByteArrayToHexString(const PackedByteArray& a);

};

#endif
