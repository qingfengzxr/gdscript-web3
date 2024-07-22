#include "eth_abi_wrapper.h"
#include "eth_abi/abi.h"


EthABIWrapper::EthABIWrapper() {

}

EthABIWrapper::~EthABIWrapper() {

}


void EthABIWrapper::_bind_methods() {
	//ClassDB::bind_method(D_METHOD("keccak256_hash"), &KeccakWrapper::keccak256_hash);
	ClassDB::bind_method(D_METHOD("float_plus"), &EthABIWrapper::float_plus);
	ClassDB::bind_method(D_METHOD("encode"), &EthABIWrapper::encode);
	ClassDB::bind_method(D_METHOD("decode"), &EthABIWrapper::decode);
}

float EthABIWrapper::float_plus(float a, float b)
{
	return a + b;
}

PackedByteArray EthABIWrapper::encode(int a) {
	struct eth_abi data;
	uint8_t aa = a;
	size_t hexlen;
	char* hex;
	eth_abi_init(&data, ETH_ABI_ENCODE);
	eth_abi_uint8(&data, &aa);
	eth_abi_to_hex(&data, &hex, &hexlen);
	eth_abi_free(&data);

	PackedByteArray result;
	result.resize(hexlen);
	memcpy(result.ptrw(), hex, hexlen);
	return result;
}

int EthABIWrapper::decode(PackedByteArray& a) {
	struct eth_abi data;
	uint8_t aa;
	char* tt;
	a.resize(128);
	memcpy(tt, *a.ptrw(), 128);
	eth_abi_from_hex(&data, tt, -1);
	eth_abi_uint8(&data, &aa);
	eth_abi_free(&data);
	return (int)aa;
}


