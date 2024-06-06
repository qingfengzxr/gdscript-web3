#ifndef LEGACY_TX_H
#define LEGACY_TX_H

#include <iostream>

#include "core/error/error_list.h"
#include "core/error/error_macros.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"

#include "big_int.h"
#include "rlp.h"
#include "keccak256.h"
#include "secp256k1_wrapper.h"


class LegacyTx : public RefCounted {
	GDCLASS(LegacyTx, RefCounted);

	// chain information
	Ref<BigInt> m_chain_id;

	// transaction information
	uint64_t m_nonce; // nonce of sender account
	Ref<BigInt> m_gas_price; // wei per gas
	uint64_t m_gas_limit; // gas limit
	// TODO: @Wyatt, need to support Address object
	String m_to; // null means contract creation
	// To       *common.Address `rlp:"nil"`
	Ref<BigInt> m_value; // wei amount
	PackedByteArray m_data; // contract invocation input data

	// signature values
	Ref<BigInt> m_v;
	Ref<BigInt> m_r;
	Ref<BigInt> m_s;

protected:
	static void _bind_methods();

public:
	LegacyTx();
	~LegacyTx();

	void set_chain_id(Ref<BigInt> chain_id);
	void set_nonce(uint64_t nonce);
	void set_gas_price(Ref<BigInt> gas_price);
	void set_gas_limit(uint64_t gas_limit);
	void set_to_address(String to);
	void set_value(Ref<BigInt> value);
	void set_data(PackedByteArray data);
	void set_sign_v(Ref<BigInt> v);
	void set_sign_r(Ref<BigInt> r);
	void set_sign_s(Ref<BigInt> s);

	Ref<BigInt> get_chain_id() const;
	uint64_t get_nonce() const;
	Ref<BigInt> get_gas_price() const;
	uint64_t get_gas_limit() const;
	String get_to_address() const;
	Ref<BigInt> get_value() const;
	PackedByteArray get_data() const;
	Ref<BigInt> get_sign_v() const;
	Ref<BigInt> get_sign_r() const;
	Ref<BigInt> get_sign_s() const;

	// TODO: @cooper, support transaction functions
	PackedByteArray rlp_hash();
	String get_nonce_hex() const;
};

#endif // LEGACY_TX_H
