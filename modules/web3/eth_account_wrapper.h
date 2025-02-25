#ifndef ETH_ACCOUNT_WRAPPER_H
#define ETH_ACCOUNT_WRAPPER_H

#include "core/error/error_macros.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"

#include "account.h"


/**
 * @brief Ethereum account class for handling basic operations of Ethereum accounts.
 */
class EthAccount : public RefCounted {
	GDCLASS(EthAccount, RefCounted);

protected:
	/**
	 * @brief Bind methods to the Godot scripting system.
	 */
	static void _bind_methods();

public:
	EthAccount() = default;
	~EthAccount() = default;

	/**
	 * @brief Get the private key of the account.
	 * @return Byte array of the account's private key.
	 */
	PackedByteArray get_private_key() const;

	/**
	 * @brief Get the public key of the account.
	 * @return Byte array of the account's public key.
	 */
	PackedByteArray get_public_key() const;

	/**
	 * @brief Get the address of the account.
	 * @return Byte array of the account's address.
	 */
	PackedByteArray get_address() const;

	/**
	 * @brief Get the hex format address of the account.
	 * @return account's hex string address with '0x' prefix.
	 */
	String get_hex_address() const;

	/**
	 * @brief Sign data.
	 * @param data Byte array of the data to be signed.
	 * @return Byte array of the signed data.
	 */
	PackedByteArray sign_data(const PackedByteArray &data) const;

	/**
	 * @brief Sign data after caculate keccak256 hash with '\x19Ethereum Signed Message' prefix.
	 *
	 *        Rule: keccak256("\x19Ethereum Signed Message:\n" + len(message) + message).
	 * @param data Byte array of the data to be signed.
	 * @return Byte array of the signed data.
	 */
	PackedByteArray sign_data_with_prefix(const PackedByteArray &data) const;

	/**
	 * @brief Initialize the account.
	 * @param m_account Account data structure.
	 * @return True if initialization is successful, otherwise false.
	 */

	bool init(const struct eth_account *m_account);

private:
	struct eth_account account{}; ///< Internal Ethereum account data.
	uint8_t pubKey33[33];
};

/**
 * @brief Ethereum account manager class for creating and managing Ethereum accounts.
 */
class EthAccountManager : public RefCounted {
	GDCLASS(EthAccountManager, RefCounted);

protected:
	/**
	 * @brief Bind methods to the Godot scripting system.
	 */
	static void _bind_methods();

public:
	EthAccountManager() = default;
	~EthAccountManager() = default;

	/**
	 * @brief Enum for decryption errors.
	 */
	enum DecryptError {
		SUCCESS = 0,
		
		EMPTY_PASSWORD,
		INVALID_PASSWORD,
		INVALID_PRIVATE_KEY,
	
		INVALID_KEYSTORE_FORMAT,
		INVALID_CRYPTO_SECTION,
		
		UNSUPPORTED_CIPHER,
		UNSUPPORTED_KDF,
		UNSUPPORTED_PRF,
		
		INVALID_PBKDF2_PARAMS,
		PBKDF2_ITERATIONS_TOO_LOW,
		PBKDF2_DERIVATION_FAILED,
		
		RNG_INIT_FAILED,
		SALT_GENERATION_FAILED,
		IV_GENERATION_FAILED,
		
		MAC_CALCULATION_FAILED,
		
		CIPHER_SETUP_FAILED,
		CIPHER_KEY_FAILED,
		CIPHER_IV_FAILED,
		ENCRYPTION_FAILED,
		DECRYPTION_FAILED,
		FINAL_ENCRYPTION_FAILED,
		FINAL_DECRYPTION_FAILED,
		
		ADDRESS_GENERATION_FAILED
	};

	/**
	 * @brief Create a new Ethereum account.
	 * @return The newly created account.
	 */
	static Ref<EthAccount> create(const PackedByteArray &entropy = {});

	/**
	 * @brief Create an Ethereum account from a private key.
	 * @param privkey Byte array of the account's private key.
	 * @return The account created from the private key.
	 */
	static Ref<EthAccount> privateKeyToAccount(const PackedByteArray &privkey);

	/**
	 * @brief Create an Ethereum account from a private key.
	 * @param privkey Byte array of the account's private key.
	 * @return The account created from the private key.
	 */
	static Dictionary encrypt(const String &p_privkey, const String &p_password, const Dictionary &options);

	/**
	 * @brief Decrypt an Ethereum keystore file.
	 * @param keystore The keystore data as a Dictionary.
	 * @param password The password to decrypt the keystore.
	 * @return The account from the keystore.
	 */
	static Dictionary decrypt(const Dictionary &keystore, const String &password);
};

#endif // _ETH_ACCOUNT_WRAPPER_H
