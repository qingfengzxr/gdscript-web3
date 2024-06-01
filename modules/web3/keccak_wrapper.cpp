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

int eth_keccak256(uint8_t *dest, const uint8_t *bytes, size_t len) {
  Keccak_HashInstance instance;

  if (dest == NULL || bytes == NULL)
    return -1;

  if (Keccak_HashInitialize(&instance, KECCAK256_RATE, KECCAK256_CAPACITY,
                            KECCAK256_HASHBITLEN,
                            KECCAK256_DELIMITED_SUFFIX) == KECCAK_FAIL)
    return -1;

  if (Keccak_HashUpdate(&instance, bytes, len * 8) == KECCAK_FAIL)
    return -1;

  return Keccak_HashFinal(&instance, dest) == KECCAK_SUCCESS ? 1 : -1;
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

