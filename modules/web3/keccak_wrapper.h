#ifndef KECCAK_H
#define KECCAK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "keccak256.h"

#ifdef __cplusplus
}
#endif

#include <iostream>
#include <vector>
#include <iomanip>
#include <stdlib.h>
// #include <gmp.h>

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "core/error/error_macros.h"
#include "core/error/error_list.h"


class KeccakWrapper : public RefCounted {
	GDCLASS(KeccakWrapper, RefCounted);

protected:
	static void _bind_methods();

public:
	KeccakWrapper();
	~KeccakWrapper();

	PackedByteArray keccak256_hash(const PackedByteArray &data);
};

#endif // KECCAK_H
