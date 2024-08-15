#ifndef ETH_ACCOUNT_WRAPPER_H
#define ETH_ACCOUNT_WRAPPER_H

#include "core/error/error_macros.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "ethprotocol/account.h"


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
	 * @brief Get the mnemonic of the account.
	 * @return Array of mnemonic strings.
	 */
	PackedStringArray get_mnemonic() const;

	/**
	 * @brief Sign data.
	 * @param data Byte array of the data to be signed.
	 * @return Byte array of the signed data.
	 */
	PackedByteArray sign_data(const PackedByteArray &data) const;

	/**
	 * @brief Initialize the account.
	 * @param m_account Account data structure.
	 * @return True if initialization is successful, otherwise false.
	 */
	bool init(const struct eth_account *m_account);

private:
	struct eth_account account {}; ///< Internal Ethereum account data.
	PackedStringArray mnemonic; ///< Mnemonic array.

	/**
	 * @brief Convert a byte array to a hexadecimal string.
	 * @param byte_array Byte array to be converted.
	 * @return Hexadecimal string representation.
	 */
	String convert_to_hex(const PackedByteArray &byte_array) const;
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
	 * @brief Create a new Ethereum account.
	 * @return The newly created account.
	 */
	Ref<EthAccount> create();

	/**
	 * @brief Create a new Ethereum account with entropy.
	 * @param entropy Byte array of entropy used to create the account.
	 * @return The newly created account.
	 */
	Ref<EthAccount> create_with_entropy(const PackedByteArray &entropy);

	/**
	 * @brief Create an Ethereum account from a private key.
	 * @param privkey Byte array of the account's private key.
	 * @return The account created from the private key.
	 */
	Ref<EthAccount> from_private_key(const PackedByteArray &privkey);

	/**
	 * @brief Create an Ethereum account from a mnemonic (to be implemented).
	 * @param mnemonic Array of mnemonic strings.
	 * @return The account created from the mnemonic (to be implemented in the future).
	 */
	Ref<EthAccount> from_mnemonic_key(const PackedStringArray &mnemonic);

	/**
	 * @brief Sign data.
	 * @param account The account used for signing.
	 * @param data Byte array of the data to be signed.
	 * @return Byte array of the signed data.
	 */
	PackedByteArray sign_data(const Ref<EthAccount> &account, const PackedByteArray &data);

private:
	PackedStringArray mnemonic; ///< Internal mnemonic array.
};

#endif // _ETH_ACCOUNT_WRAPPER_H