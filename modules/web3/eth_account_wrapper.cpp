#include "eth_account_wrapper.h"
#include "core/variant/variant.h"
#include "eth_abi/abi_util.h"
#include "ethprotocol/account.h"


PackedByteArray EthAccount::get_private_key() const {
	return uint8PtrToPackedByteArray(account.privkey, sizeof(account.privkey));
}

PackedByteArray EthAccount::get_public_key() const {
	return uint8PtrToPackedByteArray(account.pubkey, sizeof(account.pubkey));
}

PackedByteArray EthAccount::get_address() const {
	return uint8PtrToPackedByteArray(account.address, sizeof(account.address));
}

String EthAccount::get_hex_address() const {
	return "0x" + convert_to_hex(get_address());
}

PackedStringArray EthAccount::get_mnemonic() const {
	return this->mnemonic;
}

PackedByteArray EthAccount::sign_data_with_prefix(const PackedByteArray &data) const {
	struct eth_ecdsa_signature signature;
	unsigned char signature_bytes[65] = { 0 };

	// Call external library function to sign data
	eth_account_signp(&signature, &account, data.ptr(), data.size());

	// Copy signature data to result array
	memcpy(signature_bytes, signature.r, sizeof(signature.r));
	memcpy(signature_bytes + 32, signature.s, sizeof(signature.s));
	signature_bytes[64] = signature.recid;

	PackedByteArray result;
	result.resize(65);
	memcpy(result.ptrw(), signature_bytes, 65);

	return result;
}

PackedByteArray EthAccount::sign_data(const PackedByteArray &data) const {
	struct eth_ecdsa_signature signature;
	unsigned char signature_bytes[65] = { 0 };

	// Call external library function to sign data
	eth_account_sign(&signature, &account, data.ptr(), data.size());

	// Copy signature data to result array
	memcpy(signature_bytes, signature.r, sizeof(signature.r));
	memcpy(signature_bytes + 32, signature.s, sizeof(signature.s));
	signature_bytes[64] = signature.recid;

	PackedByteArray result;
	result.resize(65);
	memcpy(result.ptrw(), signature_bytes, 65);

	return result;
}

// Initialize the account
bool EthAccount::init(const struct eth_account *m_account) {
	if (!m_account) {
		return false;
	}
	// Copy account data
	memcpy(&account, m_account, sizeof(eth_account));
	return true;
}

// Convert a byte array to a hexadecimal string
String EthAccount::convert_to_hex(const PackedByteArray &byte_array) const {
	return packedByteArrayToHexString(byte_array);
}

void EthAccount::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_private_key"), &EthAccount::get_private_key);
	ClassDB::bind_method(D_METHOD("get_public_key"), &EthAccount::get_public_key);
	ClassDB::bind_method(D_METHOD("get_address"), &EthAccount::get_address);
	ClassDB::bind_method(D_METHOD("get_hex_address"), &EthAccount::get_hex_address);
	ClassDB::bind_method(D_METHOD("get_mnemonic"), &EthAccount::get_mnemonic);
	ClassDB::bind_method(D_METHOD("sign_data", "data"), &EthAccount::sign_data);
	ClassDB::bind_method(D_METHOD("sign_data_with_prefix", "data"), &EthAccount::sign_data_with_prefix);
}

Ref<EthAccount> EthAccountManager::create_with_entropy(const PackedByteArray &entropy) {
	struct eth_account m_account;
	Ref<EthAccount> wallet;

	wallet.instantiate();
	// Call external library function to create account
	eth_account_create(&m_account, entropy.ptr());
	wallet->init(&m_account);

	return wallet;
}

Ref<EthAccount> EthAccountManager::create() {
	PackedByteArray entropy;
	entropy.clear();
	return create_with_entropy(entropy);
}

Ref<EthAccount> EthAccountManager::from_private_key(const PackedByteArray &privkey) {
	struct eth_account m_account;
	Ref<EthAccount> wallet;

	wallet.instantiate();
	// Call external library function to create account from private key
	eth_account_from_privkey(&m_account, privkey.ptr());
	wallet->init(&m_account);

	return wallet;
}

// Create an account from mnemonic (to be implemented)
Ref<EthAccount> EthAccountManager::from_mnemonic_key(const PackedStringArray &mnemonic) {
	// TODO: Implement logic to generate an account from mnemonic
	// Future versions should implement the logic to generate an account from mnemonic.
	// Refer to Ethereum's mnemonic generation algorithms, such as BIP-39 standard.
	return NULL;
}

// Bind methods to the Godot scripting system
void EthAccountManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create"), &EthAccountManager::create);
	ClassDB::bind_method(D_METHOD("create_with_entropy", "entropy"), &EthAccountManager::create_with_entropy);
	ClassDB::bind_method(D_METHOD("from_private_key", "privkey"), &EthAccountManager::from_private_key);
	//ClassDB::bind_method(D_METHOD("from_mnemonic_key", "mnemonic"), &EthAccountManager::from_mnemonic_key);
}
