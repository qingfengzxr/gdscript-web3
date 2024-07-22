#ifndef ETH_ABI_WRAPPER_H
#define ETH_ABI_WRAPPER_H

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "eth_abi/abi.h"


class EthABIWrapper : public RefCounted {
	GDCLASS(EthABIWrapper, RefCounted);

protected:
	static void _bind_methods();

	float float_plus(float a, float b);
	PackedByteArray encode(int a);
	int decode(PackedByteArray& a);

public:
	EthABIWrapper();
	~EthABIWrapper();
};

#endif
