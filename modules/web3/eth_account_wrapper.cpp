#include "eth_account_wrapper.h"
#include "account.h"
#include "core/crypto/crypto_core.h"
#include "core/error/error_list.h"
#include "core/variant/variant.h"
#include "eth_abi/abi_util.h"
#include "eth_ecdsa.h"
#include "keccak256.h"

#include "thirdparty/mbedtls/include/mbedtls/pkcs5.h"

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
	struct eth_ecdsa_signature signature {};
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
	struct eth_account m_account {};
	Ref<EthAccount> account;

	account.instantiate();
	// Call external library function to create account
	eth_account_create(&m_account, entropy.ptr());
	account->init(&m_account);

	return account;
}

Ref<EthAccount> EthAccountManager::privateKeyToAccount(const PackedByteArray &privkey) {
	struct eth_account m_account {};
	Ref<EthAccount> account;

	account.instantiate();
	// Call external library function to create account from private key
	eth_account_from_privkey(&m_account, privkey.ptr());

	account->init(&m_account);

	return account;
}

String generate_uuid_v4() {
	CryptoCore::RandomGenerator rng;

	ERR_FAIL_COND_V_MSG(rng.init(), String(), "Failed to initialize random number generator");

	PackedByteArray uuid;
	uuid.resize(16);
	ERR_FAIL_COND_V_MSG(rng.get_random_bytes(uuid.ptrw(), 16), String(), "Failed to generate UUID");

	uuid.ptrw()[6] = (uuid[6] & 0x0F) | 0x40; // version 4
	uuid.ptrw()[8] = (uuid[8] & 0x3F) | 0x80;

	return String::hex_encode_buffer(uuid.ptr(), 16)
			.insert(8, "-")
			.insert(13, "-")
			.insert(18, "-")
			.insert(23, "-")
			.to_lower();
}

Dictionary EthAccountManager::encrypt(const String &p_privkey, const String &p_password, const Dictionary &options) {
	Dictionary result;

	// Validate input parameters
	String privkey = p_privkey.trim_prefix("0x").strip_edges();
	if (privkey.length() != 64 || !privkey.is_valid_hex_number(false)) {
		result["error_code"] = INVALID_PRIVATE_KEY;
		return result;
	}

	if (p_password.is_empty()) {
		result["error_code"] = EMPTY_PASSWORD;
		return result;
	}

	CryptoCore::RandomGenerator rng;
	if (rng.init() != OK) {
		result["error_code"] = RNG_INIT_FAILED;
		return result;
	}

	CharString password = p_password.utf8();
	const uint8_t *password_ptr = (const uint8_t *)password.get_data();
	size_t password_len = password.length();

	PackedByteArray salt = options.get("salt", PackedByteArray());
	if (salt.size() != 32) {
		salt.resize(32);
		if (rng.get_random_bytes(salt.ptrw(), 32) != OK) {
			result["error_code"] = SALT_GENERATION_FAILED;
			return result;
		}
	}

	PackedByteArray iv = options.get("iv", PackedByteArray());
	if (iv.size() != 16) {
		iv.resize(16);
		if (rng.get_random_bytes(iv.ptrw(), 16) != OK) {
			result["error_code"] = IV_GENERATION_FAILED;
			return result;
		}
	}

	const String kdf = options.get("kdf", "pbkdf2");
	const int dklen = options.get("dklen", 32);
	PackedByteArray derived_key;
	Dictionary kdfparams;

	if (kdf == "pbkdf2") {
		const int c = options.get("c", 262144);
		if (c < 1000) {
			result["error_code"] = PBKDF2_ITERATIONS_TOO_LOW;
			return result;
		}

		derived_key.resize(dklen);
		int ret = mbedtls_pkcs5_pbkdf2_hmac_ext(
				MBEDTLS_MD_SHA256,
				password_ptr,
				password_len,
				salt.ptr(),
				salt.size(),
				static_cast<unsigned int>(c),
				static_cast<size_t>(dklen),
				derived_key.ptrw());

		if (ret != 0) {
			result["error_code"] = PBKDF2_DERIVATION_FAILED;
			return result;
		}

		kdfparams["c"] = c;
		kdfparams["prf"] = "hmac-sha256";

	} else if (kdf == "scrypt") {
		/** Not Ready ! **/
		result["error_code"] = UNSUPPORTED_KDF;
		return result;

	} else {
		result["error_code"] = UNSUPPORTED_KDF;
		return result;
	}

	kdfparams["dklen"] = dklen;
	kdfparams["salt"] = packedByteArrayToHexString(salt);

	PackedByteArray privkey_bytes;
	hexStringToPackedByteArray(privkey.utf8().get_data(), privkey_bytes);

	mbedtls_cipher_context_t ctx;
	mbedtls_cipher_init(&ctx);

	const mbedtls_cipher_info_t *cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CTR);
	if (mbedtls_cipher_setup(&ctx, cipher_info) != 0) {
		result["error_code"] = CIPHER_SETUP_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}

	PackedByteArray cipher;
	cipher.append_array(derived_key.slice(0, 16));

	if (mbedtls_cipher_setkey(&ctx, cipher.ptr(), 128, MBEDTLS_ENCRYPT) != 0) {
		result["error_code"] = CIPHER_KEY_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}

	if (mbedtls_cipher_set_iv(&ctx, iv.ptr(), iv.size()) != 0) {
		result["error_code"] = CIPHER_IV_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}

	PackedByteArray ciphertext;
	ciphertext.resize(privkey_bytes.size());
	size_t olen = 0;

	if (mbedtls_cipher_update(&ctx, privkey_bytes.ptr(), privkey_bytes.size(),
				ciphertext.ptrw(), &olen) != 0) {
		result["error_code"] = ENCRYPTION_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}

	if (mbedtls_cipher_finish(&ctx, ciphertext.ptrw() + olen, &olen) != 0) {
		result["error_code"] = FINAL_ENCRYPTION_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}

	mbedtls_cipher_free(&ctx);

	PackedByteArray mac_data;
	mac_data.append_array(derived_key.slice(16, 32));
	mac_data.append_array(ciphertext);

	PackedByteArray mac;
	mac.resize(32);
	uint8_t hash[32];
	eth_keccak256(hash, mac_data.ptr(), mac_data.size());
	memcpy(mac.ptrw(), hash, 32);

	Dictionary crypto;
	crypto["ciphertext"] = packedByteArrayToHexString(ciphertext);
	crypto["cipher"] = "aes-128-ctr";

	Dictionary cipherparams;
	cipherparams["iv"] = packedByteArrayToHexString(iv);
	crypto["cipherparams"] = cipherparams;

	crypto["kdf"] = kdf;
	crypto["kdfparams"] = kdfparams;
	crypto["mac"] = packedByteArrayToHexString(mac);

	String address = packedByteArrayToHexString(privateKeyToAccount(privkey_bytes)->get_address());
	if (address.is_empty()) {
		result["error_code"] = ADDRESS_GENERATION_FAILED;
		return result;
	}

	result["version"] = 3;
	result["id"] = generate_uuid_v4();
	result["address"] = address;
	result["crypto"] = crypto;

	return result;
}

EthAccountManager::DecryptError validate_keystore(const Dictionary &keystore, Dictionary &crypto) {
	if (!keystore.has("crypto")) {
		return EthAccountManager::INVALID_KEYSTORE_FORMAT;
	}

	crypto = keystore["crypto"];
	if (!crypto.has("cipher") || !crypto.has("ciphertext") || !crypto.has("cipherparams") ||
			!crypto.has("kdf") || !crypto.has("kdfparams") || !crypto.has("mac")) {
		return EthAccountManager::INVALID_CRYPTO_SECTION;
	}

	if (String(crypto["cipher"]) != "aes-128-ctr") {
		return EthAccountManager::UNSUPPORTED_CIPHER;
	}

	return EthAccountManager::SUCCESS;
}

EthAccountManager::DecryptError validate_kdf_params(const Dictionary &crypto, Dictionary &kdfparams) {
	String kdf = crypto["kdf"];
	if (kdf != "pbkdf2") {
		return EthAccountManager::UNSUPPORTED_KDF;
	}

	kdfparams = crypto["kdfparams"];
	if (!kdfparams.has("c") || !kdfparams.has("prf") || !kdfparams.has("salt") || !kdfparams.has("dklen")) {
		return EthAccountManager::INVALID_PBKDF2_PARAMS;
	}

	if (String(kdfparams["prf"]) != "hmac-sha256") {
		return EthAccountManager::UNSUPPORTED_PRF;
	}

	return EthAccountManager::SUCCESS;
}

Dictionary EthAccountManager::decrypt(const Dictionary &keystore, const String &password) {
	Dictionary result;
	result["success"] = false;

	Dictionary crypto;
	DecryptError error = validate_keystore(keystore, crypto);
	if (error != SUCCESS) {
		result["error_code"] = error;
		return result;
	}

	Dictionary kdfparams;
	error = validate_kdf_params(crypto, kdfparams);
	if (error != SUCCESS) {
		result["error_code"] = error;
		return result;
	}

	String salt_hex = String(kdfparams["salt"]).trim_prefix("0x");
	PackedByteArray salt;
	hexStringToPackedByteArray(salt_hex.utf8().get_data(), salt);

	int c = kdfparams["c"];
	int dklen = kdfparams["dklen"];

	CharString password_utf8 = password.utf8();
	const uint8_t *password_ptr = (const uint8_t *)password_utf8.get_data();
	size_t password_len = password_utf8.length();

	PackedByteArray derived_key;
	derived_key.resize(dklen);

	int ret = mbedtls_pkcs5_pbkdf2_hmac_ext(
			MBEDTLS_MD_SHA256,
			password_ptr,
			password_len,
			salt.ptr(),
			salt.size(),
			static_cast<unsigned int>(c),
			static_cast<size_t>(dklen),
			derived_key.ptrw());

	if (ret != 0) {
		result["error_code"] = PBKDF2_DERIVATION_FAILED;
		return result;
	}

	PackedByteArray mac_data;
	mac_data.append_array(derived_key.slice(16, 32));

	String ciphertext_hex = String(crypto["ciphertext"]).trim_prefix("0x");
	PackedByteArray ciphertext;
	hexStringToPackedByteArray(ciphertext_hex.utf8().get_data(), ciphertext);

	mac_data.append_array(ciphertext);

	PackedByteArray calculated_mac;
	calculated_mac.resize(32);

	uint8_t hash[32];
	eth_keccak256(hash, mac_data.ptr(), mac_data.size());
	memcpy(calculated_mac.ptrw(), hash, 32);

	String stored_mac = String(crypto["mac"]).trim_prefix("0x");
	PackedByteArray stored_mac_bytes;
	hexStringToPackedByteArray(stored_mac.utf8().get_data(), stored_mac_bytes);

	if (calculated_mac != stored_mac_bytes) {
		result["error_code"] = INVALID_PASSWORD;
		return result;
	}

	Dictionary cipherparams = crypto["cipherparams"];
	String iv_hex = String(cipherparams["iv"]).trim_prefix("0x");
	PackedByteArray iv;
	hexStringToPackedByteArray(iv_hex.utf8().get_data(), iv);

	mbedtls_cipher_context_t ctx;
	mbedtls_cipher_init(&ctx);

	const mbedtls_cipher_info_t *cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CTR);
	if (mbedtls_cipher_setup(&ctx, cipher_info) != 0) {
		result["error_code"] = CIPHER_SETUP_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}

	if (mbedtls_cipher_setkey(&ctx, derived_key.ptr(), 128, MBEDTLS_DECRYPT) != 0) {
		result["error_code"] = CIPHER_KEY_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}

	if (mbedtls_cipher_set_iv(&ctx, iv.ptr(), iv.size()) != 0) {
		result["error_code"] = CIPHER_IV_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}

	PackedByteArray plaintext;
	plaintext.resize(ciphertext.size());
	size_t olen = 0;

	if (mbedtls_cipher_update(&ctx, ciphertext.ptr(), ciphertext.size(), plaintext.ptrw(), &olen) != 0) {
		result["error_code"] = DECRYPTION_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}
	if (mbedtls_cipher_finish(&ctx, plaintext.ptrw() + olen, &olen) != 0) {
		result["error_code"] = FINAL_DECRYPTION_FAILED;
		mbedtls_cipher_free(&ctx);
		return result;
	}
	mbedtls_cipher_free(&ctx);

	Ref<EthAccount> account = privateKeyToAccount(plaintext);
	if (account.is_valid()) {
		result["success"] = true;
		result["account"] = account;
	} else {
		result["error_code"] = DECRYPTION_FAILED;
	}
	return result;
}

// Bind methods to the Godot scripting system
void EthAccountManager::_bind_methods() {
	ClassDB::bind_static_method("EthAccountManager",
			D_METHOD("create", "entropy"),
			&EthAccountManager::create,
			DEFVAL(PackedByteArray()));

	ClassDB::bind_static_method("EthAccountManager",
			D_METHOD("privateKeyToAccount", "privkey"),
			&EthAccountManager::privateKeyToAccount);

	ClassDB::bind_static_method("EthAccountManager",
			D_METHOD("encrypt", "private_key", "password", "options"),
			&EthAccountManager::encrypt,
			DEFVAL(Dictionary()));

	ClassDB::bind_static_method("EthAccountManager",
			D_METHOD("decrypt", "keystore", "password"),
			&EthAccountManager::decrypt);
}