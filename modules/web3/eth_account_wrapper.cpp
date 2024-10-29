#include "eth_account_wrapper.h"
#include "core/variant/variant.h"
#include "eth_abi/abi_util.h"
#include "account.h"
#include "eth_ecdsa.h"

PackedByteArray EthAccount::get_private_key() const {
	return uint8PtrToPackedByteArray(&account.privkey[0], sizeof(account.privkey));
}

PackedByteArray EthAccount::get_public_key() const {
	return uint8PtrToPackedByteArray(pubKey33, sizeof(pubKey33));
}

PackedByteArray EthAccount::get_address() const {
	return uint8PtrToPackedByteArray(account.address, sizeof(account.address));
}

PackedByteArray EthAccount::sign_data(const PackedByteArray &data) const {
	struct eth_ecdsa_signature signature{};
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

static String convert_to_hex(const PackedByteArray &byte_array) {
	return packedByteArrayToHexString(byte_array);
}

String EthAccount::get_hex_address() const {
	return "0x" + convert_to_hex(get_address());
}

// Initialize the account
bool EthAccount::init(const struct eth_account *m_account) {
	if (!m_account) {
		return false;
	}

	if (eth_ecdsa_pubkey_get_with_compressed(pubKey33, m_account->privkey) != 1) {
		// Handle error: eth_ecdsa_pubkey_get_with_compressed failed
		return false;
	}

	// Copy account data
	memcpy(&account, m_account, sizeof(eth_account));

	return true;
}

void EthAccount::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_private_key"), &EthAccount::get_private_key);
	ClassDB::bind_method(D_METHOD("get_public_key"), &EthAccount::get_public_key);
	ClassDB::bind_method(D_METHOD("get_address"), &EthAccount::get_address);
	ClassDB::bind_method(D_METHOD("get_hex_address"), &EthAccount::get_hex_address);
	ClassDB::bind_method(D_METHOD("sign_data", "data"), &EthAccount::sign_data);
	ClassDB::bind_method(D_METHOD("sign_data_with_prefix", "data"), &EthAccount::sign_data_with_prefix);
}

Ref<EthAccount> EthAccountManager::create(const PackedByteArray &entropy) {
	struct eth_account m_account{};
	Ref<EthAccount> account;

	account.instantiate();
	// Call external library function to create account
	eth_account_create(&m_account, entropy.ptr());
	account->init(&m_account);

	return account;
}

Ref<EthAccount> EthAccountManager::privateKeyToAccount(const PackedByteArray &privkey) {
	struct eth_account m_account{};
	Ref<EthAccount> account;

	account.instantiate();
	// Call external library function to create account from private key
	eth_account_from_privkey(&m_account, privkey.ptr());

	account->init(&m_account);

	return account;
}

// Bind methods to the Godot scripting system
void EthAccountManager::_bind_methods() {
	ClassDB::bind_method(D_METHOD("create","entropy"), &EthAccountManager::create,DEFVAL(PackedByteArray()));
	ClassDB::bind_method(D_METHOD("from_private_key", "privkey"), &EthAccountManager::privateKeyToAccount);
}
