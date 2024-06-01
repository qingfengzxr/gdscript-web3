#include "keccak_wrapper.h"

#define KECCAK256_RATE 1088
#define KECCAK256_CAPACITY 512
#define KECCAK256_HASHBITLEN 256
#define KECCAK256_DELIMITED_SUFFIX 0x1

KeccakWrapper::KeccakWrapper() {
	;
}

KeccakWrapper::~KeccakWrapper() {
	;
}

PackedByteArray KeccakWrapper::keccak256_hash(const PackedByteArray &data) {
	uint8_t hash[32];

	const uint8_t* data_ptr = data.ptr();
	eth_keccak256(hash, data_ptr, data.size());

	PackedByteArray result;
	result.resize(32);
	memcpy(result.ptrw(), hash, 32);
	return result;
}

void KeccakWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("keccak256_hash"), &KeccakWrapper::keccak256_hash);
}

