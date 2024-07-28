#ifndef ETH_ABI_WRAPPER_H
#define ETH_ABI_WRAPPER_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "eth_abi/abi.h"
#include "eth_abi/abi_util.h"
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

	PackedByteArray encode_call(const String& funcName, const Array& args);
	Variant decode_call(const String& value, const Array& args);

private:

	PackedByteArray eth_abi_encode_int(Variant value);
	Variant eth_abi_decode_int(const String& value);

	PackedByteArray eth_abi_encode_bool(Variant value);
	Variant eth_abi_decode_bool(const String& value);

	PackedByteArray eth_abi_encode_bytes(const PackedByteArray& value);
	PackedByteArray eth_abi_decode_bytes(const String& value);

	PackedByteArray eth_abi_encode_address(Variant value);
	Variant eth_abi_decode_address(const String& value);

	PackedByteArray eth_abi_encode_array(Variant value, const String& type);
	Variant eth_abi_decode_array(const String& value, const String& type);

};

#endif
