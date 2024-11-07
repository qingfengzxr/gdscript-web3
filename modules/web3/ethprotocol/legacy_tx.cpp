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

	// ERR_FAIL_COND_V_MSG(eth_rlp_array(&rlp0) < 0, PackedByteArray(), "eth_rlp_array failed");
	if (eth_rlp_array(&rlp0) < 0) {
		ERR_PRINT("eth_rlp_init failed");
		eth_rlp_free(&rlp0);
		return PackedByteArray();
	}
		// rlp format for nonce
		// char* nonce = const_cast<char*>(this->get_nonce_hex().utf8().get_data());
		CharString nonce_cstr = this->get_nonce_hex().utf8();
		char *nonce = nonce_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &nonce, NULL) < 0, PackedByteArray(), "rlp format nonce failed");

		// rlp format for gas price
		CharString gas_price_cstr = m_gas_price->to_hex().utf8();
		char *gas_price = gas_price_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_price, NULL) < 0, PackedByteArray(), "rlp format gas price failed");

		// rlp format for gas limit
		CharString gas_limit_cstr = uint64_to_hex_string(this->m_gas_limit).utf8();
		char *gas_limit = gas_limit_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_limit, NULL) < 0, PackedByteArray(), "rlp format gas limit failed");

		// rlp format for to address
		print_line("rlp_hash format to address start.");
		CharString to_cstr = m_to.utf8();
		char *to = to_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_address(&rlp0, &to) < 0, PackedByteArray(), "rlp format to address failed");
		print_line("rlp_hash format to address done.");

		// rlp format for value
		CharString value_cstr = m_value->to_hex().utf8();
		char *value = value_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &value, NULL) < 0, PackedByteArray(), "rlp format value failed");

		// rlp format for data
		uint8_t *data_bytes = new uint8_t[m_data.size()];
		memcpy(data_bytes, m_data.ptr(), m_data.size());
		size_t data_len = m_data.size();
		ERR_FAIL_COND_V_MSG(eth_rlp_bytes(&rlp0, &data_bytes, &data_len) < 0, PackedByteArray(), "rlp foramt data bytes failed");

		// rlp format for chain id(v)
		ERR_FAIL_COND_V_MSG(m_chain_id->is_zero(), PackedByteArray(), "rlp format failed: chain id should not be zero");
		CharString chain_id_cstr = m_chain_id->to_hex().utf8();
		char *chain_id = chain_id_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &chain_id, NULL) < 0, PackedByteArray(), "rlp format chain id failed");

		// rlp format for r,s
		uint8_t zero=0;
		if (m_r.is_null() || m_r->is_zero()) {
			ERR_FAIL_COND_V_MSG(eth_rlp_uint8(&rlp0, &zero) < 0, PackedByteArray(), "rlp format r(zero) failed");
		} else {
			CharString r_cstr = m_r->to_hex().utf8();
			char *r = r_cstr.ptrw();
			ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &r, NULL) < 0, PackedByteArray(), "rlp format r failed");
		}

		if (m_r.is_null() || m_s->is_zero()) {
			ERR_FAIL_COND_V_MSG(eth_rlp_uint8(&rlp0, &zero) < 0, PackedByteArray(), "rlp format s(zero) failed");
		} else {
			CharString s_cstr = m_s->to_hex().utf8();
			char *s = s_cstr.ptrw();
			ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &s, NULL) < 0, PackedByteArray(), "rlp format s failed");
		}

	// ERR_FAIL_COND_V_MSG(eth_rlp_array_end(&rlp0) < 0, PackedByteArray(), "eth_rlp_array end failed");
	if (eth_rlp_array_end(&rlp0) < 0) {
		ERR_PRINT("eth_rlp_array end failed");
		eth_rlp_free(&rlp0);
		return PackedByteArray();
	}

	ERR_FAIL_COND_V_MSG(eth_rlp_to_bytes(&rlp0_bytes, &rlp0_len, &rlp0) < 0, PackedByteArray(), "eth_rlp_to_bytes failed");
	ERR_FAIL_COND_V_MSG(eth_rlp_free(&rlp0) < 0, PackedByteArray(), "eth_rlp_free failed");

	if (data_bytes != NULL) {
		free(data_bytes);
		data_bytes = NULL;
	}

	// compute the keccak hash of the rlp encoded transaction
	uint8_t hash[32];
	eth_keccak256(hash, rlp0_bytes, rlp0_len);
	if (rlp0_bytes != NULL) {
	    free(rlp0_bytes);
		rlp0_bytes = NULL;
	}

	PackedByteArray result;
	result.resize(32);
	memcpy(result.ptrw(), hash, 32);

	return result;
}

PackedByteArray LegacyTx::rlp_encode() {
	struct eth_rlp rlp0;
	uint8_t *rlp0_bytes;
	size_t rlp0_len;

	// Get the rlp format of the transaction
	ERR_FAIL_COND_V_MSG(eth_rlp_init(&rlp0, ETH_RLP_ENCODE) < 0, PackedByteArray(), "eth_rlp_init failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_array(&rlp0) < 0, PackedByteArray(), "eth_rlp_array failed");
		// rlp format for nonce
		// char* nonce = const_cast<char*>(this->get_nonce_hex().utf8().get_data());
		CharString nonce_cstr = this->get_nonce_hex().utf8();
		char* nonce = nonce_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &nonce, NULL) < 0, PackedByteArray(), "rlp format nonce failed");

		// rlp format for gas price
		CharString gas_price_cstr = m_gas_price->to_hex().utf8();
		char *gas_price = gas_price_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_price, NULL) < 0, PackedByteArray(), "rlp format gas price failed");

		// rlp format for gas limit
		CharString gas_limit_cstr = uint64_to_hex_string(this->m_gas_limit).utf8();
		char *gas_limit = gas_limit_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_limit, NULL) < 0, PackedByteArray(), "rlp format gas limit failed");

		// rlp format for to address
		print_line("rlp_hash format to address start.");
		CharString to_cstr = m_to.utf8();
		char *to = to_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_address(&rlp0, &to) < 0, PackedByteArray(), "rlp format to address failed");
		print_line("rlp_hash format to address done.");

		// rlp format for value
		CharString value_cstr = m_value->to_hex().utf8();
		char *value = value_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &value, NULL) < 0, PackedByteArray(), "rlp format value failed");

		// rlp format for data
		uint8_t *data_bytes = new uint8_t[m_data.size()];
		memcpy(data_bytes, m_data.ptr(), m_data.size());
		size_t data_len = m_data.size();
		ERR_FAIL_COND_V_MSG(eth_rlp_bytes(&rlp0, &data_bytes, &data_len) < 0, PackedByteArray(), "rlp foramt data bytes failed");

		// rlp format for chain id(v)
		ERR_FAIL_COND_V_MSG(m_chain_id->is_zero(), PackedByteArray(), "rlp format failed: chain id should not be zero");
		CharString chain_id_cstr = m_chain_id->to_hex().utf8();
		char *chain_id = chain_id_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &chain_id, NULL) < 0, PackedByteArray(), "rlp format chain id failed");

		// rlp format for r,s
		uint8_t zero=0;
		if (m_r.is_null() || m_r->is_zero()) {
			ERR_FAIL_COND_V_MSG(eth_rlp_uint8(&rlp0, &zero) < 0, PackedByteArray(), "rlp format r(zero) failed");
		} else {
			CharString r_cstr = m_r->to_hex().utf8();
			char *r = r_cstr.ptrw();
			ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &r, NULL) < 0, PackedByteArray(), "rlp format r failed");
		}

		if (m_r.is_null() || m_s->is_zero()) {
			ERR_FAIL_COND_V_MSG(eth_rlp_uint8(&rlp0, &zero) < 0, PackedByteArray(), "rlp format s(zero) failed");
		} else {
			CharString s_cstr = m_s->to_hex().utf8();
			char *s = s_cstr.ptrw();
			ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &s, NULL) < 0, PackedByteArray(), "rlp format s failed");
		}

	ERR_FAIL_COND_V_MSG(eth_rlp_array_end(&rlp0) < 0, PackedByteArray(), "eth_rlp_array failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_to_bytes(&rlp0_bytes, &rlp0_len, &rlp0) < 0, PackedByteArray(), "eth_rlp_to_bytes failed");
	ERR_FAIL_COND_V_MSG(eth_rlp_free(&rlp0) < 0, PackedByteArray(), "eth_rlp_free failed");

	PackedByteArray rlp_encode_bytes;
	rlp_encode_bytes.resize(rlp0_len);
	memcpy(rlp_encode_bytes.ptrw(), rlp0_bytes, rlp0_len);

	free(rlp0_bytes);
	return rlp_encode_bytes;
}

String LegacyTx::get_nonce_hex() const {
    return uint64_to_hex_string(this->m_nonce);
}

int LegacyTx::sign_tx(Ref<Secp256k1Wrapper> signer) {
	// FIXME: does it need to check the signer is valid?
	// FIXME: does it need to check the LegacyTx data is valid?
	PackedByteArray rlp_hash = this->rlp_hash();
	PackedByteArray signature = signer->sign(rlp_hash);

	if (signature.size() != 65) {
		return -1;
	}

	uint8_t _signature[65];
	memcpy(_signature, signature.ptr(), 65);

	// Get the r, s, and v values from the signature and caculate v
	uint8_t r[32];
	uint8_t s[32];
	memcpy(r, _signature, 32);
	memcpy(s, _signature + 32, 32);

	int v_value = signature[64] + 35 + 2 * this->get_chain_id()->to_int();
	String v_value_str = String::num(v_value);

	// Set the r, s, and v values
	Ref<BigInt> r_bigint = Ref<BigInt>(memnew(BigInt));
	Ref<BigInt> s_bigint = Ref<BigInt>(memnew(BigInt));
	Ref<BigInt> v_bigint = Ref<BigInt>(memnew(BigInt));
	r_bigint->set_bytes(r, 32);
	s_bigint->set_bytes(s, 32);
	v_bigint->from_string(v_value_str);

	this->set_sign_r(r_bigint);
	this->set_sign_s(s_bigint);
	this->set_sign_v(v_bigint);
	return 0;
}

int LegacyTx::sign_tx_by_account(Ref<EthAccount> signer) {
	// FIXME: does it need to check the signer is valid?
	// FIXME: does it need to check the LegacyTx data is valid?
	PackedByteArray rlp_encode_data = this->rlp_encode();
	PackedByteArray signature = signer->sign_data(rlp_encode_data);
	if (signature.size() != 65) {
		return -1;
	}

	uint8_t _signature[65];
	memcpy(_signature, signature.ptr(), 65);

	// Get the r, s, and v values from the signature and caculate v
	uint8_t r[32];
	uint8_t s[32];
	memcpy(r, _signature, 32);
	memcpy(s, _signature + 32, 32);

	int v_value = signature[64] + 35 + 2 * this->get_chain_id()->to_int();
	String v_value_str = String::num(v_value);

	// Set the r, s, and v values
	Ref<BigInt> r_bigint = Ref<BigInt>(memnew(BigInt));
	Ref<BigInt> s_bigint = Ref<BigInt>(memnew(BigInt));
	Ref<BigInt> v_bigint = Ref<BigInt>(memnew(BigInt));
	r_bigint->set_bytes(r, 32);
	s_bigint->set_bytes(s, 32);
	v_bigint->from_string(v_value_str);

	this->set_sign_r(r_bigint);
	this->set_sign_s(s_bigint);
	this->set_sign_v(v_bigint);
	return 0;
}


String LegacyTx::signedtx_marshal_binary() {
	struct eth_rlp rlp0;

	// Get the rlp format of the transaction
	ERR_FAIL_COND_V_MSG(eth_rlp_init(&rlp0, ETH_RLP_ENCODE) < 0, String(), "eth_rlp_init failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_array(&rlp0) < 0, String(), "eth_rlp_array failed");
		// rlp format for nonce
		CharString nonce_cstr = this->get_nonce_hex().utf8();
		char* nonce = nonce_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &nonce, NULL) < 0, String(), "rlp format nonce failed");

		// rlp format for gas price
		CharString gas_price_cstr = m_gas_price->to_hex().utf8();
		char *gas_price = gas_price_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_price, NULL) < 0, String(), "rlp format gas price failed");

		// rlp format for gas limit
		CharString gas_limit_cstr = uint64_to_hex_string(this->m_gas_limit).utf8();
		char *gas_limit = gas_limit_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_limit, NULL) < 0, String(), "rlp format gas limit failed");

		// rlp format for to address
		print_line("rlp_hash format to address start.");
		CharString to_cstr = m_to.utf8();
		char *to = to_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_address(&rlp0, &to) < 0, String(), "rlp format to address failed");
		print_line("rlp_hash format to address done.");

		// rlp format for value
		CharString value_cstr = m_value->to_hex().utf8();
		char *value = value_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &value, NULL) < 0, String(), "rlp format value failed");

		// rlp format for data
		uint8_t *data_bytes = new uint8_t[m_data.size()];
		memcpy(data_bytes, m_data.ptr(), m_data.size());
		size_t data_len = m_data.size();
		ERR_FAIL_COND_V_MSG(eth_rlp_bytes(&rlp0, &data_bytes, &data_len) < 0, String(), "rlp foramt data bytes failed");

		// rlp format for v
		ERR_FAIL_COND_V_MSG(m_v->is_zero(), String(), "rlp format failed: chain id should not be zero");
		CharString v_cstr = m_v->to_hex().utf8();
		char *v = v_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &v, NULL) < 0, String(), "rlp format chain id failed");

		// rlp format for r
		ERR_FAIL_COND_V_MSG(m_r.is_null(), String(), "rlp format failed: r is zero");
		CharString r_cstr = m_r->to_hex().utf8();
		char *r = r_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &r, NULL) < 0, String(), "rlp format r failed");

		// rlp format for s
		ERR_FAIL_COND_V_MSG(m_s.is_null(), String(), "rlp format failed: r is zero");
		CharString s_cstr = m_s->to_hex().utf8();
		char *s = s_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &s, NULL) < 0, String(), "rlp format s failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_array_end(&rlp0) < 0, String(), "eth_rlp_array failed");

	char *txn;
	ERR_FAIL_COND_V_MSG(eth_rlp_to_hex(&txn, &rlp0) < 0, String(), "eth_rlp_to_hex failed");
	ERR_FAIL_COND_V_MSG(eth_rlp_free(&rlp0) < 0, String(), "eth_rlp_free failed");

	String res = String(txn);
	free(txn);
	return "0x" + res;
}

PackedByteArray LegacyTx::hash() {
	struct eth_rlp rlp0;
	uint8_t *rlp0_bytes;
	size_t rlp0_len;

	// Get the rlp format of the transaction
	ERR_FAIL_COND_V_MSG(eth_rlp_init(&rlp0, ETH_RLP_ENCODE) < 0, PackedByteArray(), "eth_rlp_init failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_array(&rlp0) < 0, PackedByteArray(), "eth_rlp_array failed");
		// rlp format for nonce
		CharString nonce_cstr = this->get_nonce_hex().utf8();
		char* nonce = nonce_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &nonce, NULL) < 0, PackedByteArray(), "rlp format nonce failed");

		// rlp format for gas price
		CharString gas_price_cstr = m_gas_price->to_hex().utf8();
		char *gas_price = gas_price_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_price, NULL) < 0, PackedByteArray(), "rlp format gas price failed");

		// rlp format for gas limit
		CharString gas_limit_cstr = uint64_to_hex_string(this->m_gas_limit).utf8();
		char *gas_limit = gas_limit_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &gas_limit, NULL) < 0, PackedByteArray(), "rlp format gas limit failed");

		// rlp format for to address
		print_line("rlp_hash format to address start.");
		CharString to_cstr = m_to.utf8();
		char *to = to_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_address(&rlp0, &to) < 0, PackedByteArray(), "rlp format to address failed");
		print_line("rlp_hash format to address done.");

		// rlp format for value
		CharString value_cstr = m_value->to_hex().utf8();
		char *value = value_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &value, NULL) < 0, PackedByteArray(), "rlp format value failed");

		// rlp format for data
		uint8_t *data_bytes = new uint8_t[m_data.size()];
		memcpy(data_bytes, m_data.ptr(), m_data.size());
		size_t data_len = m_data.size();
		ERR_FAIL_COND_V_MSG(eth_rlp_bytes(&rlp0, &data_bytes, &data_len) < 0, PackedByteArray(), "rlp foramt data bytes failed");

		// rlp format for v
		ERR_FAIL_COND_V_MSG(m_v->is_zero(), PackedByteArray(), "rlp format failed: chain id should not be zero");
		CharString v_cstr = m_v->to_hex().utf8();
		char *v = v_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &v, NULL) < 0, PackedByteArray(), "rlp format chain id failed");

		// rlp format for r
		ERR_FAIL_COND_V_MSG(m_r.is_null(), PackedByteArray(), "rlp format failed: r is zero");
		CharString r_cstr = m_r->to_hex().utf8();
		char *r = r_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &r, NULL) < 0, PackedByteArray(), "rlp format r failed");

		// rlp format for s
		ERR_FAIL_COND_V_MSG(m_s.is_null(), PackedByteArray(), "rlp format failed: r is zero");
		CharString s_cstr = m_s->to_hex().utf8();
		char *s = s_cstr.ptrw();
		ERR_FAIL_COND_V_MSG(eth_rlp_hex(&rlp0, &s, NULL) < 0, PackedByteArray(), "rlp format s failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_array_end(&rlp0) < 0, PackedByteArray(), "eth_rlp_array failed");

	ERR_FAIL_COND_V_MSG(eth_rlp_to_bytes(&rlp0_bytes, &rlp0_len, &rlp0) < 0, PackedByteArray(), "eth_rlp_to_bytes failed");
	ERR_FAIL_COND_V_MSG(eth_rlp_free(&rlp0) < 0, PackedByteArray(), "eth_rlp_free failed");

	// compute the keccak hash of the rlp encoded transaction
	uint8_t hash[32];
	eth_keccak256(hash, rlp0_bytes, rlp0_len);
    free(rlp0_bytes);

	PackedByteArray result;
	result.resize(32);
	memcpy(result.ptrw(), hash, 32);
	return result;
}

void LegacyTx::_bind_methods() {
	ClassDB::bind_method(D_METHOD("set_chain_id", "chain_id"), &LegacyTx::set_chain_id);
	ClassDB::bind_method(D_METHOD("get_chain_id"), &LegacyTx::get_chain_id);

	ClassDB::bind_method(D_METHOD("set_nonce", "nonce"), &LegacyTx::set_nonce);
	ClassDB::bind_method(D_METHOD("get_nonce"), &LegacyTx::get_nonce);

	ClassDB::bind_method(D_METHOD("set_gas_price", "gas_price"), &LegacyTx::set_gas_price);
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
	ClassDB::bind_method(D_METHOD("hash"), &LegacyTx::hash);
	ClassDB::bind_method(D_METHOD("sign_tx", "signer"), &LegacyTx::sign_tx);
	ClassDB::bind_method(D_METHOD("sign_tx_by_account", "signer"), &LegacyTx::sign_tx_by_account);
	ClassDB::bind_method(D_METHOD("signedtx_marshal_binary"), &LegacyTx::signedtx_marshal_binary);
}

