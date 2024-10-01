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
#include "jsonrpc_helper.h"
#include "big_int.h"

class Optimism : public RefCounted {
	GDCLASS(Optimism, RefCounted);

	Ref<Secp256k1Wrapper> m_secp256k1;
	Ref<KeccakWrapper> m_keccak;
	Ref<JsonrpcHelper> m_jsonrpc_helper;
	String m_rpc_url;

protected:
	static void _bind_methods();

public:
	Optimism();
	~Optimism();

	bool init_secp256k1_instance();
	Ref<Secp256k1Wrapper> get_secp256k1_wrapper();
	Ref<KeccakWrapper> get_keccak_wrapper();

	String get_rpc_url() const;
	void set_rpc_url(const String &url);

	// sync jsonrpc request method, base on JsonrpcHelper class

	Dictionary chain_id(const Variant &id);
	Dictionary block_by_hash(const String &hash, const Variant &id);
	Dictionary block_by_number(const Ref<BigInt> &number, const Variant &id);
	Dictionary block_number(const Variant &id);
	Dictionary send_transaction(const String &signed_tx, const Variant &id);
	// TODO: PeerCount()
	// TODO: BlockReceipts()
	Dictionary header_by_hash(const String &hash, const Variant &id);
	Dictionary header_by_number(const Ref<BigInt> &number, const Variant &id);
	Dictionary call_contract(const Dictionary &call_msg, const String &block_number, const Variant &id);



	// async jsonrpc request method, base on JSONRPC class.
	// Only return request dictionary

	Dictionary async_block_number(const Variant &id);
	Dictionary async_send_transaction(const String &signed_tx, const Variant &id);
};

#endif // OPTIMISM_H
