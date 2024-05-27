#ifndef OPTIMISM_H
#define OPTIMISM_H

#include <iostream>
#include <vector>
#include <iomanip>

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "core/error/error_macros.h"
#include "core/error/error_list.h"

#include "secp256k1_wrapper.h"
#include "keccak_wrapper.h"

class Optimism : public RefCounted {
	GDCLASS(Optimism, RefCounted);

	Ref<Secp256k1Wrapper> m_secp256k1;
	Ref<KeccakWrapper> m_keccak;

protected:
	static void _bind_methods();

public:
	Optimism();
	~Optimism();

	bool init_secp256k1_instance();

	Ref<Secp256k1Wrapper> get_secp256k1_wrapper();
	Ref<KeccakWrapper> get_keccak_wrapper();
};

#endif // OPTIMISM_H
