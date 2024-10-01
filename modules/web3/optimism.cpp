#include "optimism.h"

Optimism::Optimism() {
	m_secp256k1 = Ref<Secp256k1Wrapper>(memnew(Secp256k1Wrapper));
	m_keccak = Ref<KeccakWrapper>(memnew(KeccakWrapper));
	m_jsonrpc_helper = Ref<JsonrpcHelper>(memnew(JsonrpcHelper));
}

Optimism::~Optimism() {
	;
}

bool Optimism::init_secp256k1_instance() {
	return m_secp256k1->initialize();
}

Ref<Secp256k1Wrapper> Optimism::get_secp256k1_wrapper() {
	return m_secp256k1;
}

Ref<KeccakWrapper> Optimism::get_keccak_wrapper() {
	return m_keccak;
}

String Optimism::get_rpc_url() const {
    return m_rpc_url;
}

void Optimism::set_rpc_url(const String &url) {
    m_rpc_url = url;
	if ( m_rpc_url != "" && m_jsonrpc_helper != NULL ) {
		m_jsonrpc_helper->set_hostname(m_rpc_url);
	}
}

// chain_id() retrieves the current chain ID for transaction replay protection.
Dictionary Optimism::chain_id(const Variant &id) {
	Vector<Variant> p_params	= Vector<Variant>();
	return m_jsonrpc_helper->call_method("eth_chainId", p_params, id);
}

// block_by_hash() returns the given full block.
//
// Note that loading full blocks requires two requests. Use header_by_hash()
// if you don't need all transactions or uncle headers.
Dictionary Optimism::block_by_hash(const String &hash, const Variant &id) {
	Vector<Variant> p_params	= Vector<Variant>();
	p_params.push_back(hash);
	p_params.push_back(true);
	return m_jsonrpc_helper->call_method("eth_getBlockByHash", p_params, id);
}

// header_by_hash() returns the block header with the given hash.
Dictionary Optimism::header_by_hash(const String &hash, const Variant &id) {
	Vector<Variant> p_params	= Vector<Variant>();
	p_params.push_back(hash);
	p_params.push_back(false);
	return m_jsonrpc_helper->call_method("eth_getBlockByHash", p_params, id);
}

// block_by_number() returns a block from the current canonical chain. If number is null, the
// latest known block is returned.
//
// Note that loading full blocks requires two requests. Use header_by_number()
// if you don't need all transactions or uncle headers.
Dictionary Optimism::block_by_number(const Ref<BigInt> &number, const Variant &id) {
	String number_str = "";
	if ( number == NULL ) {
		number_str = "latest";
	} else {
		if ( number->sgn() < 0 ) {
			Dictionary call_result;
			call_result["success"] = false;
			call_result["errmsg"] = "block number must be positive.";
			return call_result;
		} else {
			number_str = number->to_hex();
		}
	}

	Vector<Variant> p_params = Vector<Variant>();
	p_params.push_back(number_str);
	p_params.push_back(true);
	return m_jsonrpc_helper->call_method("eth_getBlockByNumber", p_params, id);
}

// HeaderByNumber returns a block header from the current canonical chain. If number is
// null, the latest known header is returned.
Dictionary Optimism::header_by_number(const Ref<BigInt> &number, const Variant &id) {
	String number_str = "";
	if ( number == NULL ) {
		number_str = "latest";
	} else {
		if ( number->sgn() < 0 ) {
			Dictionary call_result;
			call_result["success"] = false;
			call_result["errmsg"] = "block number must be positive.";
			return call_result;
		} else {
			number_str = number->to_hex();
		}
	}

	Vector<Variant> p_params = Vector<Variant>();
	p_params.push_back(number_str);
	p_params.push_back(false);
	return m_jsonrpc_helper->call_method("eth_getBlockByNumber", p_params, id);
}

// block_number() returns the most recent block number
Dictionary Optimism::block_number(const Variant &id) {
	Vector<Variant> p_params	= Vector<Variant>();
	return m_jsonrpc_helper->call_method("eth_blockNumber", p_params, id);
}

Dictionary Optimism::async_block_number(const Variant &id) {
	JSONRPC* jsonrpc = new JSONRPC();

	Vector<String> p_params	= Vector<String>();
	Dictionary request = jsonrpc->make_request("eth_blockNumber", p_params, id);
	return request;
}

// send_transaction injects a signed transaction into the pending pool for execution.
//
// If the transaction was a contract creation use the TransactionReceipt method to get the
// contract address after the transaction has been mined.
//
// signed_tx: The signed transaction data encode as a hex string with 0x prefix.
Dictionary Optimism::send_transaction(const String &signed_tx, const Variant &id) {
	Vector<Variant> p_params	= Vector<Variant>();
	p_params.push_back(signed_tx);
	return m_jsonrpc_helper->call_method("eth_sendRawTransaction", p_params, id);
}

Dictionary Optimism::async_send_transaction(const String &signed_tx, const Variant &id) {
	JSONRPC* jsonrpc = new JSONRPC();

	Vector<String> p_params	= Vector<String>();
	p_params.push_back(signed_tx);
	Dictionary request = jsonrpc->make_request("eth_blockNumber", p_params, id);
	return request;
}

// call_contract executes a message call transaction, which is directly executed in the VM
// of the node, but never mined into the blockchain.
//
// blockNumber selects the block height at which the call runs. It can be nil, in which
// case the code is taken from the latest known block. Note that state from very old
// blocks might not be available.
Dictionary Optimism::call_contract(const Dictionary &call_msg, const String &block_number, const Variant &id) {
	Vector<Variant> p_params	= Vector<Variant>();

	// first param: call msg
	p_params.push_back(call_msg);
	// second param: block number
	if ( block_number == "" ) {
		p_params.push_back("latest");
	} else {
		p_params.push_back(block_number);
	}
	return m_jsonrpc_helper->call_method("eth_call", p_params, id);
}

// suggest_gas_price retrieves the currently suggested gas price to allow a timely
// execution of a transaction.
Ref<BigInt> Optimism::suggest_gas_price(const Variant &id) {
	Ref<BigInt> gas_price = Ref<BigInt>(memnew(BigInt));

	Vector<Variant> p_params	= Vector<Variant>();
	Dictionary result =  m_jsonrpc_helper->call_method("eth_gasPrice", p_params, id);
	if (bool(result["success"]) == false) {
		ERR_PRINT(
			vformat("Failed with calling eth_gasPrice. errmsg: %s", result["errmsg"])
		);
		return gas_price;
	}

	Ref<JSON> json = Ref<JSON>(memnew(JSON));
	Dictionary res = json->parse_string(result["response_body"]);
	gas_price->from_hex(res["result"]);
	return gas_price;
}

// EstimateGas tries to estimate the gas needed to execute a specific transaction based on
// the current pending state of the backend blockchain. There is no guarantee that this is
// the true gas limit requirement as other transactions may be added or removed by miners,
// but it should provide a basis for setting a reasonable default.
Dictionary Optimism::estimate_gas(const Dictionary &call_msg, const Variant &id) {
	Vector<Variant> p_params	= Vector<Variant>();
	p_params.push_back(call_msg);
	return m_jsonrpc_helper->call_method("eth_estimateGas", p_params, id);
}


void Optimism::_bind_methods() {
	ClassDB::bind_method(D_METHOD("init_secp256k1_instance"), &Optimism::init_secp256k1_instance);
	ClassDB::bind_method(D_METHOD("get_secp256k1_wrapper"), &Optimism::get_secp256k1_wrapper);
	ClassDB::bind_method(D_METHOD("get_keccak_wrapper"), &Optimism::get_keccak_wrapper);
    ClassDB::bind_method(D_METHOD("get_rpc_url"), &Optimism::get_rpc_url);
    ClassDB::bind_method(D_METHOD("set_rpc_url", "url"), &Optimism::set_rpc_url);

	// sync jsonrpc method
	ClassDB::bind_method(D_METHOD("chain_id", "id"), &Optimism::chain_id);
	ClassDB::bind_method(D_METHOD("block_by_hash", "hash", "id"), &Optimism::block_by_hash);
	ClassDB::bind_method(D_METHOD("header_by_hash", "hash", "id"), &Optimism::header_by_hash);
	ClassDB::bind_method(D_METHOD("block_by_number", "number", "id"), &Optimism::block_by_number);
	ClassDB::bind_method(D_METHOD("header_by_number", "number", "id"), &Optimism::header_by_number);
	ClassDB::bind_method(D_METHOD("block_number", "id"), &Optimism::block_number);
	ClassDB::bind_method(D_METHOD("send_transaction", "signed_tx", "id"), &Optimism::send_transaction);
	ClassDB::bind_method(D_METHOD("call_contract", "call_msg", "block_number", "id"), &Optimism::call_contract);
	ClassDB::bind_method(D_METHOD("suggest_gas_price", "id"), &Optimism::suggest_gas_price);
	ClassDB::bind_method(D_METHOD("estimate_gas", "call_msg", "id"), &Optimism::estimate_gas);

	// async jsonrpc method
	ClassDB::bind_method(D_METHOD("async_block_number", "id"), &Optimism::async_block_number);
	ClassDB::bind_method(D_METHOD("async_send_transaction", "signed_tx", "id"), &Optimism::async_send_transaction);
}

