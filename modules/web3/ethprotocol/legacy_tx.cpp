#include "legacy_tx.h"


String uint64_to_hex_string(uint64_t value) {
    if (value == 0) {
        return "0";
    }

    String result;
    char hex_chars[] = "0123456789abcdef";
    for (int i = 0; i < 16; ++i) {
        result = String() + hex_chars[value & 0xF] + result;
        value >>= 4;
    }

    // Remove leading zeros
    while (result.length() > 1 && result[0] == '0') {
        result = result.substr(1);
    }

    return result;
}


LegacyTx::LegacyTx() {
}

LegacyTx::~LegacyTx() {
}

void LegacyTx::set_chain_id(Ref<BigInt> chain_id) {
    this->m_chain_id = chain_id;
}

void LegacyTx::set_nonce(uint64_t nonce) {
    this->m_nonce = nonce;
}

void LegacyTx::set_gas_price(Ref<BigInt> gas_price) {
    this->m_gas_price = gas_price;
}

void LegacyTx::set_gas_limit(uint64_t gas_limit) {
    this->m_gas_limit = gas_limit;
}

void LegacyTx::set_to_address(String to) {
    this->m_to = to;
}

void LegacyTx::set_value(Ref<BigInt> value) {
	ERR_FAIL_COND_MSG(value->sgn() < 0, "value cannot be less than 0");
    this->m_value = value;
}

void LegacyTx::set_data(PackedByteArray data) {
    this->m_data = data;
}

void LegacyTx::set_sign_v(Ref<BigInt> sign_v) {
    this->m_v = sign_v;
}

void LegacyTx::set_sign_r(Ref<BigInt> sign_r) {
    this->m_r = sign_r;
}

void LegacyTx::set_sign_s(Ref<BigInt> sign_s) {
    this->m_s = sign_s;
}

Ref<BigInt> LegacyTx::get_chain_id() const {
    return this->m_chain_id;
}

uint64_t LegacyTx::get_nonce() const {
    return this->m_nonce;
}

Ref<BigInt> LegacyTx::get_gas_price() const {
    return this->m_gas_price;
}

uint64_t LegacyTx::get_gas_limit() const {
    return this->m_gas_limit;
}

String LegacyTx::get_to_address() const {
    return this->m_to;
}

Ref<BigInt> LegacyTx::get_value() const {
    return this->m_value;
}

PackedByteArray LegacyTx::get_data() const {
    return this->m_data;
}

Ref<BigInt> LegacyTx::get_sign_v() const {
    return this->m_v;
}

Ref<BigInt> LegacyTx::get_sign_r() const {
    return this->m_r;
}

Ref<BigInt> LegacyTx::get_sign_s() const {
    return this->m_s;
}

PackedByteArray LegacyTx::rlp_hash() {
	struct eth_rlp rlp0;
	uint8_t *rlp0_bytes;
	size_t rlp0_len;

	// Get the rlp format of the transaction
	ERR_FAIL_COND_V_MSG(eth_rlp_init(&rlp0, ETH_RLP_ENCODE) < 0, PackedByteArray(), "eth_rlp_init failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_array(&rlp0) < 0, PackedByteArray(), "eth_rlp_array failed");
		// rlp format for nonce
		char* nonce = const_cast<char*>(this->get_nonce_hex().utf8().get_data());
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &nonce, NULL) < 0, PackedByteArray(), "rlp format nonce failed");

		// rlp format for gas price
		char *gas_price_cstr = const_cast<char*>(m_gas_price->to_hex().utf8().get_data());
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_price_cstr, NULL) < 0, PackedByteArray(), "rlp format gas price failed");

		// rlp format for gas limit
		char *gas_limit = const_cast<char*>(uint64_to_hex_string(this->m_gas_limit).utf8().get_data());
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_limit, NULL) < 0, PackedByteArray(), "rlp format gas limit failed");

		// rlp format for to address
		char *to_cstr = const_cast<char*>(m_to.utf8().get_data());
		ERR_FAIL_COND_V_MSG(eth_rlp_address(&rlp0, &to_cstr) < 0, PackedByteArray(), "rlp format to address failed");

		// rlp format for value
		char *value_cstr = const_cast<char*>(m_value->to_hex().utf8().get_data());
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &value_cstr, NULL) < 0, PackedByteArray(), "rlp format value failed");

		// rlp format for data
		uint8_t *data_bytes = new uint8_t[m_data.size()];
		memcpy(data_bytes, m_data.ptr(), m_data.size());
		size_t data_len = m_data.size();
		ERR_FAIL_COND_V_MSG(eth_rlp_bytes(&rlp0, &data_bytes, &data_len) < 0, PackedByteArray(), "rlp foramt data bytes failed");

		// rlp format for chain id
		char *m_chain_id_cstr = const_cast<char*>(m_chain_id->to_hex().utf8().get_data());
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &m_chain_id_cstr, NULL) < 0, PackedByteArray(), "rlp format chain id failed");

		// rlp format for last double zero
		uint8_t zero=0;
		ERR_FAIL_COND_V_MSG(eth_rlp_uint8(&rlp0, &zero) < 0, PackedByteArray(), "rlp format zero_1 failed");
		ERR_FAIL_COND_V_MSG(eth_rlp_uint8(&rlp0, &zero) < 0, PackedByteArray(), "rlp format zero_2 failed");
	ERR_FAIL_COND_V_MSG(eth_rlp_array_end(&rlp0) < 0, PackedByteArray(), "eth_rlp_array failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_to_bytes(&rlp0_bytes, &rlp0_len, &rlp0) < 0, PackedByteArray(), "eth_rlp_to_bytes failed");
	ERR_FAIL_COND_V_MSG(eth_rlp_free(&rlp0) < 0, PackedByteArray(), "eth_rlp_free failed");

	// print rlp bytes in hexadecimal
	printf("debug print rlp bytes:"); // todo: delete debug
    for(size_t i = 0; i < rlp0_len; i++) {
        printf("%02x", rlp0_bytes[i]);
    }
    printf("\n");

	// compute the keccak hash of the rlp encoded transaction
	uint8_t hash[32];
	eth_keccak256(hash, rlp0_bytes, rlp0_len);
    free(rlp0_bytes);

	PackedByteArray result;
	result.resize(32);
	memcpy(result.ptrw(), hash, 32);

	return result;
}

String LegacyTx::get_nonce_hex() const {
    return uint64_to_hex_string(this->m_nonce);
}


void LegacyTx::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_chain_id"), &LegacyTx::set_chain_id);
	ClassDB::bind_method(D_METHOD("get_chain_id"), &LegacyTx::get_chain_id);

	ClassDB::bind_method(D_METHOD("set_nonce"), &LegacyTx::set_nonce);
	ClassDB::bind_method(D_METHOD("get_nonce"), &LegacyTx::get_nonce);

	ClassDB::bind_method(D_METHOD("set_gas_price"), &LegacyTx::set_gas_price);
	ClassDB::bind_method(D_METHOD("get_gas_price"), &LegacyTx::get_gas_price);

    ClassDB::bind_method(D_METHOD("get_gas_limit"), &LegacyTx::get_gas_limit);
    ClassDB::bind_method(D_METHOD("set_gas_limit", "gas_limit"), &LegacyTx::set_gas_limit);

    ClassDB::bind_method(D_METHOD("get_to_address"), &LegacyTx::get_to_address);
    ClassDB::bind_method(D_METHOD("set_to_address", "to"), &LegacyTx::set_to_address);

    ClassDB::bind_method(D_METHOD("get_value"), &LegacyTx::get_value);
    ClassDB::bind_method(D_METHOD("set_value", "value"), &LegacyTx::set_value);

    ClassDB::bind_method(D_METHOD("get_data"), &LegacyTx::get_data);
    ClassDB::bind_method(D_METHOD("set_data", "data"), &LegacyTx::set_data);

    ClassDB::bind_method(D_METHOD("get_sign_v"), &LegacyTx::get_sign_v);
    ClassDB::bind_method(D_METHOD("set_sign_v", "sign_v"), &LegacyTx::set_sign_v);

    ClassDB::bind_method(D_METHOD("get_sign_r"), &LegacyTx::get_sign_r);
    ClassDB::bind_method(D_METHOD("set_sign_r", "sign_r"), &LegacyTx::set_sign_r);

    ClassDB::bind_method(D_METHOD("get_sign_s"), &LegacyTx::get_sign_s);
    ClassDB::bind_method(D_METHOD("set_sign_s", "sign_s"), &LegacyTx::set_sign_s);


	ClassDB::bind_method(D_METHOD("rlp_hash"), &LegacyTx::rlp_hash);
}

