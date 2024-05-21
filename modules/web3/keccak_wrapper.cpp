#include "keccak_wrapper.h"
#include "keccak.h"


KeccakWrapper::KeccakWrapper() {
	;
}

KeccakWrapper::~KeccakWrapper() {
	;
}

PackedByteArray KeccakWrapper::keccak256_hash(const PackedByteArray &data) {
	union ethash_hash256 hash;

	const uint8_t* data_ptr = data.ptr();
	hash = ethash_keccak256(data_ptr, data.size());

	PackedByteArray result;
	result.resize(32);
	memcpy(result.ptrw(), hash.bytes, 32);
	return result;
}

void KeccakWrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("keccak256_hash"), &KeccakWrapper::keccak256_hash);
}

