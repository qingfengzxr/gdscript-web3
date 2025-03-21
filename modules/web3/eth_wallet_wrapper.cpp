#include "eth_wallet_wrapper.h"

#include "abi_util.h"
#include "account_utils.h"
#include "bip32.h"
#include "bip39.h"
#include "internals.h"

#include "core/variant/variant.h"

const uint32_t ETH_MAIN_PATH[] = { BIP32_INITIAL_HARDENED_CHILD + 44, BIP32_INITIAL_HARDENED_CHILD + 60, BIP32_INITIAL_HARDENED_CHILD, 0 };

PackedStringArray mnemonic_to_packed_string_array(const char *mnemonic_str) {
	PackedStringArray packed_array;
	const char *start = mnemonic_str;
	const char *end;

	while (*start) {
		end = start;
		while (*end && *end != ' ') {
			end++;
		}

		size_t length = end - start;

		if (length > 0) {
			String word(start, length);
			packed_array.push_back(word);
		}

		start = (*end) ? end + 1 : end;
	}

	return packed_array;
}

static String merge_mnemonic_phrases(const PackedStringArray &p_array) {
	String result;

	for (int i = 0; i < p_array.size(); ++i) {
		if (i > 0) {
			result += " ";
		}
		result += p_array[i];
	}

	return result;
}

EthWallet::EthWallet(int account_number, const PackedByteArray &entropy, const String &passphrase, Error *r_error) {
	if (r_error) {
		*r_error = OK;
	}

	char *mnemonic_str = generate_mnemonic(entropy.ptr(), entropy.size());
	if (!validate_mnemonic(mnemonic_str)) {
		if (r_error) {
			*r_error = FAILED;
		}
		return;
	}

	if (!init_keys(mnemonic_str, passphrase.utf8().get_data())) {
		if (r_error) {
			*r_error = FAILED;
		}
	}

	this->mnemonic = mnemonic_to_packed_string_array(mnemonic_str);

	for (size_t i = 0; i < account_number; i++) {
		add();
	}
}

EthWallet::EthWallet(const PackedStringArray &mnemonic, const String &passphrase, Error *r_error) {
	if (r_error) {
		*r_error = OK;
	}
	const char *mnemonic_str = merge_mnemonic_phrases(mnemonic).utf8().ptr();
	if (!validate_mnemonic(mnemonic_str)) {
		if (r_error) {
			*r_error = FAILED;
		}
		return;
	}

	if (!init_keys(mnemonic_str, passphrase.utf8().get_data())) {
		if (r_error) {
			*r_error = FAILED;
		}
		clear();
	}
}

char *EthWallet::generate_mnemonic(const uint8_t *entropy, const size_t lens) {
	char *mnemonic_str;
	int ret;

	if (entropy == nullptr || lens < 16) {
		unsigned char random[16];
		get_random_bytes(8, random, 16);
		ret = bip39_mnemonic_from_bytes(nullptr, random, 16, &mnemonic_str);
	} else {
		ret = bip39_mnemonic_from_bytes(nullptr, entropy, lens, &mnemonic_str);
	}

	if (ret != WEB3_OK) {
		return nullptr;
	}
	return mnemonic_str;
}

bool EthWallet::validate_mnemonic(const char *mnemonic_str) {
	if (bip39_mnemonic_validate(nullptr, mnemonic_str) != WEB3_OK) {
		return false;
	}
	return true;
}

bool EthWallet::init_keys(const char *mnemonic_str, const char *passphrase_str) {
	size_t written;

	if (bip39_mnemonic_to_seed(mnemonic_str, passphrase_str, rootSeed, BIP39_SEED_LEN_512, &written) != WEB3_OK) {
		ERR_PRINT("Failed to derive wallet seed from mnemonic");
		return false;
	}
	if (bip32_key_from_seed(rootSeed, sizeof(rootSeed), BIP32_VER_MAIN_PRIVATE, 0, &rootKey) != WEB3_OK) {
		ERR_PRINT("Failed to derive root key from seed");
		return false;
	}
	if (bip32_key_from_parent_path(&rootKey, ETH_MAIN_PATH, 4, BIP32_FLAG_KEY_PRIVATE, &masterKey) != WEB3_OK) {
		ERR_PRINT("Failed to derive master key from root key");
		return false;
	}
	return true;
}

bool EthWallet::add(const PackedByteArray &privKey) {
	ETH_EXT_KEY childKey{};
	Ref<EthAccount> account;

	if (privKey.size() == 0) {
		size_t index = accounts_.size();
		if (bip32_key_from_parent(&masterKey, index, BIP32_FLAG_KEY_PRIVATE, &childKey) != WEB3_OK) {
			return false;
		}
	} else {
		if (privKey.size() == 33) {
			memcpy(childKey.priv_key, privKey.ptr(), sizeof(childKey.priv_key));
		} else if (privKey.size() == 32) {
			childKey.priv_key[0] = 0;
			memcpy(childKey.priv_key + 1, privKey.ptr(), privKey.size());
		} else {
			ERR_PRINT("Invalid private key size: " + privKey.size());
			return false;
		}
	}

	account.instantiate();
	account = EthAccountManager::privateKeyToAccount(uint8PtrToPackedByteArray(&childKey.priv_key[1], sizeof(childKey.priv_key) - 1));
	accounts_.push_back(account);

	return true;
}

bool EthWallet::remove_address(PackedByteArray address) {
	for (size_t i = 0; i < accounts_.size(); ++i) {
		if (accounts_[i]->get_address() == address) {
			accounts_.remove_at(i);
			return true;
		}
	}
	return false;
}

bool EthWallet::remove(uint64_t index) {
	return index < accounts_.size() ? (accounts_.remove_at(index), true) : false;
}

bool EthWallet::clear() {
	accounts_.clear();
	mnemonic.clear();
	secure_clean(rootSeed, sizeof(rootSeed));
	secure_clean(&rootKey, sizeof(rootKey));
	secure_clean(&masterKey, sizeof(masterKey));
	return true;
}

Array EthWallet::get_accounts() const {
	Array result;
	for (int i = 0; i < accounts_.size(); ++i) {
		result.push_back(accounts_[i]);
	}
	return result;
}

PackedStringArray EthWallet::get_mnemonic() {
	return mnemonic;
}

void EthWallet::_bind_methods() {
	ClassDB::bind_method(D_METHOD("add", "privKey"), &EthWallet::add, DEFVAL(PackedByteArray()));
	ClassDB::bind_method(D_METHOD("remove_address", "address"), &EthWallet::remove_address);
	ClassDB::bind_method(D_METHOD("remove", "index"), &EthWallet::remove);
	ClassDB::bind_method(D_METHOD("clear"), &EthWallet::clear);
	ClassDB::bind_method(D_METHOD("get_accounts"), &EthWallet::get_accounts);
	ClassDB::bind_method(D_METHOD("get_mnemonic"), &EthWallet::get_mnemonic);
}

Ref<EthWallet> EthWalletManager::create(int strength,
		const PackedByteArray &entropy,
		const String &passphrase) {
	Ref<EthWallet> hd_wallet;
	Error r_error = OK;
	hd_wallet.instantiate(strength, entropy, passphrase, &r_error);
	return r_error == OK ? hd_wallet : Ref<EthWallet>();
}

Ref<EthWallet> EthWalletManager::from_mnemonic(const PackedStringArray &mnemonic,
		const String &passphrase) {
	Ref<EthWallet> hd_wallet;
	Error r_error = OK;
	hd_wallet.instantiate(mnemonic, passphrase, &r_error);
	return (hd_wallet.is_valid() && r_error == OK) ? hd_wallet : Ref<EthWallet>();
}

Array EthWalletManager::encrypt(Ref<EthWallet> hdWallet, const String &p_password, const Dictionary &options) {
	Array encrypted_accounts;

	if (!hdWallet.is_valid() || p_password.is_empty()) {
		return encrypted_accounts;
	}

	Array accounts = hdWallet->get_accounts();

	for (int i = 0; i < accounts.size(); i++) {
		Ref<EthAccount> account = accounts[i];
		if (!account.is_valid())
			continue;

		PackedByteArray privkey = account->get_private_key();
		String privkey_hex = packedByteArrayToHexString(privkey);
		Dictionary account_result = EthAccountManager::encrypt(privkey_hex, p_password, options);
		if (account_result.has("error_code")) {
			return encrypted_accounts;
		}

		encrypted_accounts.push_back(account_result);
	}

	return encrypted_accounts;
}

Ref<EthWallet> EthWalletManager::decrypt(const Array &keystoreArray, const String &p_password, const Dictionary &options) {
	Ref<EthWallet> wallet;
	wallet.instantiate();

	if (keystoreArray.size() == 0 || p_password.is_empty()) {
		return wallet;
	}

	for (int i = 0; i < keystoreArray.size(); i++) {
		Dictionary keystore = keystoreArray[i];
		Dictionary result = EthAccountManager::decrypt(keystore, p_password);

		if (!result["success"]) {
			return wallet;
		}

		Ref<EthAccount> account = result["account"];
		if (!account.is_valid()) {
			return wallet;
		}

		wallet->add(account->get_private_key());
	}

	Array accounts = wallet->get_accounts();
	Ref<EthAccount> account = accounts[0];

	return wallet;
}

void EthWalletManager::_bind_methods() {
	ClassDB::bind_static_method("EthWalletManager", D_METHOD("create", "strength", "entropy", "passphrase"),
			&EthWalletManager::create, DEFVAL(0), DEFVAL(PackedByteArray()), DEFVAL(""));

	ClassDB::bind_static_method("EthWalletManager", D_METHOD("from_mnemonic", "mnemonic", "passphrase"),
			&EthWalletManager::from_mnemonic, DEFVAL(""));

	ClassDB::bind_static_method("EthWalletManager", D_METHOD("encrypt", "wallet", "password", "options"),
			&EthWalletManager::encrypt, DEFVAL(Dictionary()));

	ClassDB::bind_static_method("EthWalletManager", D_METHOD("decrypt", "keystore", "password", "options"),
			&EthWalletManager::decrypt, DEFVAL(Dictionary()));
}
