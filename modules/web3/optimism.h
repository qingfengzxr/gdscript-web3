#ifndef OPTIMISM_H
#define OPTIMISM_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <regex>

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/string/print_string.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "core/error/error_macros.h"
#include "core/error/error_list.h"
#include "core/io/json.h"

#include "common.h"
#include "secp256k1_wrapper.h"
#include "keccak_wrapper.h"
#include "jsonrpc_helper.h"
#include "big_int.h"
#include "eth_abi_wrapper.h"
#include "eth_account_wrapper.h"
#include "legacy_tx.h"

class Optimism : public RefCounted {
	GDCLASS(Optimism, RefCounted);

	Ref<Secp256k1Wrapper> m_secp256k1;
	Ref<KeccakWrapper> m_keccak;
	Ref<JsonrpcHelper> m_jsonrpc_helper;
	Ref<EthAccount> m_eth_account;

	String m_rpc_url;
	uint32_t m_req_id;

protected:
	static void _bind_methods();

public:
	Optimism();
	~Optimism();

	bool init_secp256k1_instance();
	Ref<Secp256k1Wrapper> get_secp256k1_wrapper();
	Ref<KeccakWrapper> get_keccak_wrapper();
	Ref<EthAccount> get_eth_account();
	void set_eth_account(const Ref<EthAccount> &account);

	String get_rpc_url() const;
	void set_rpc_url(const String &url);

	/**
	 * @brief Sign a transaction by eth account which is set by set_eth_account method.
	 *
	 * @param transaction Dictionary of transaction data.
	 * @return Hex string of signed transaction.
	 */
	String sign_transaction(const Dictionary &transaction);


	// sync jsonrpc request method, base on JsonrpcHelper class

	Ref<BigInt> chain_id(const Variant &id = "");
	Dictionary network_id(const Variant &id = "");
	Dictionary block_by_hash(const String &hash, const Variant &id = "");
	Dictionary block_by_number(const Ref<BigInt> &number, const Variant &id = "");
	Dictionary block_number(const Variant &id = "");
	Dictionary block_receipts_by_number(const int64_t &number, const Variant &id = "");
	Dictionary block_receipts_by_hash(const String &hash, const Variant &id = "");
	Dictionary transaction_by_hash(const String &hash, const Variant &id = "");
	Dictionary transaction_receipt_by_hash(const String &hash, const Variant &id);
	Dictionary balance_at(const String &account, const Ref<BigInt> &block_number, const Variant &id = "");
	uint64_t nonce_at(const String &account, const Ref<BigInt> &block_number = Ref<BigInt>(), const Variant &id = "");
	// TODO: BalanceAtHash()
	Dictionary send_transaction(const String &signed_tx, const Variant &id = "");

	Dictionary header_by_hash(const String &hash, const Variant &id = "");
	Dictionary header_by_number(const Ref<BigInt> &number, const Variant &id = "");
	Dictionary call_contract(Dictionary call_msg, const String &block_number, const Variant &id = "");
	Ref<BigInt> suggest_gas_price(const Variant &id = "");
	uint64_t estimate_gas(const Dictionary &call_msg, const Variant &id = "");

	// async jsonrpc request method, base on JSONRPC class.
	// Only return request dictionary

	Dictionary async_block_by_hash(const String &hash, const Variant &id = "");
	Dictionary async_block_by_number(const Ref<BigInt> &number, const Variant &id = "");
	Dictionary async_block_number(const Variant &id = "");
	Dictionary async_block_receipts_by_number(const int64_t &number, const Variant &id = "");
	Dictionary async_block_receipts_by_hash(const String &hash, const Variant &id = "");
	Dictionary async_transaction_by_hash(const String &hash, const Variant &id = "");
	Dictionary async_transaction_receipt_by_hash(const String &hash, const Variant &id);
	Dictionary async_balance_at(const String &account, const Ref<BigInt> &block_number, const Variant &id = "");
	Dictionary async_nonce_at(const String &account, const Ref<BigInt> &block_number = Ref<BigInt>(), const Variant &id = "");
	Dictionary async_send_transaction(const String &signed_tx, const Variant &id = "");
	Dictionary async_header_by_hash(const String &hash, const Variant &id = "");
	Dictionary async_header_by_number(const Ref<BigInt> &number, const Variant &id = "");
	Dictionary async_call_contract(Dictionary call_msg, const String &block_number, const Variant &id = "");
	Dictionary async_suggest_gas_price(const Variant &id = "");
	Dictionary async_estimate_gas(const Dictionary &call_msg, const Variant &id = "");
};

#endif // OPTIMISM_H
