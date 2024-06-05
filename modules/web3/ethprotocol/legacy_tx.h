#ifndef LEGACY_TX_H
#define LEGACY_TX_H

#include <gmp.h>
#include <iostream>

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "core/error/error_macros.h"
#include "core/error/error_list.h"

class LegacyTx : public RefCounted {
	GDCLASS(LegacyTx, RefCounted);

	uint64_t nonce;          // nonce of sender account

	mpz_t gas_price;        // wei per gas
	uint64_t gas_limit;          // gas limit
	// TODO: @Wyatt, need to support Address object
	// To       *common.Address `rlp:"nil"` // nil means contract creation
	mpz_t value;        // wei amount
	// TODO: @cooper, find a great way to support this
	// Data     []byte          // contract invocation input data
	// signature values
	mpz_t sign_v;
	mpz_t sign_r;
	mpz_t sign_s;

protected:
	static void _bind_methods();

public:
	LegacyTx();
	~LegacyTx();

	// TODO: @cooper, support transaction functions
};

#endif // LEGACY_TX_H
