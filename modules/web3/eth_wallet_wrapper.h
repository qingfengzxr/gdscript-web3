#ifndef ETH_WALLET_H
#define ETH_WALLET_H

#include "core/error/error_macros.h"
#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/variant.h"
#include "eth_account_wrapper.h"
#include "bip32.h"

/**
 * @brief The EthWallet class manages Ethereum wallet functionalities.
 *
 * This class provides features to manage Ethereum accounts, including
 * mnemonic generation, key initialization, account addition and removal,
 * encryption and decryption of wallet data, and saving/loading wallet state.
 */
class EthWallet : public RefCounted {
    GDCLASS(EthWallet, RefCounted);

protected:

	static void _bind_methods();

private:

	Vector<Ref<EthAccount>> accounts_;

	PackedStringArray mnemonic;

	PackedStringArray passphrase;

	ETH_EXT_KEY rootKey = {};
	ETH_EXT_KEY masterKey = {};

	uint8_t rootSeed[64];
	/**
	 * @brief Generates a mnemonic phrase from entropy.
	 *
	 * This function generates a mnemonic phrase based on the provided entropy.
	 * If the entropy is null or less than the expected length, it will generate
	 * a random sequence of bytes to create the mnemonic.
	 *
	 * @param entropy Pointer to an array of bytes representing the entropy.
	 * @param lens The length of the entropy array.
	 * @return A pointer to a character array containing the mnemonic phrase,
	 *         or nullptr if an error occurs during generation.
	 */
	char * generate_mnemonic(const uint8_t* entropy, size_t lens);

	/**
	 * @brief Initializes the keys for the EthWallet using the provided mnemonic and passphrase.
	 *
	 * This method derives the root seed and keys from the given BIP-39 mnemonic
	 * and passphrase, necessary for interacting with Ethereum wallets.
	 *
	 * @param mnemonic_str The BIP-39 mnemonic string.
	 * @param passphrase The passphrase string associated with the mnemonic.
	 * @return true if the keys were successfully initialized, false otherwise.
	 */
	bool init_keys(const char* mnemonic_str, const char* passphrase);

	/**
	 * @fn bool validate_mnemonic(const std::string& mnemonic)
	 *
	 * @brief Validates the given mnemonic phrase for correctness.
	 *
	 * This function checks whether the provided mnemonic phrase adheres to
	 * the required standards and formatting rules. It ensures that the mnemonic
	 * is valid as per the BIP-39 specification, which is commonly used for
	 * generating deterministic cryptocurrency wallets.
	 *
	 * @param mnemonic_str The mnemonic phrase to be validated.
	 * @return True if the mnemonic is valid, false otherwise.
	 */
	bool validate_mnemonic(const char* mnemonic_str);

public:
	/**
	 * @class EthWallet
	 */
	EthWallet() = default;

	EthWallet(int account_counts, const PackedByteArray &entropy, const String &passphrase, Error *r_error);

	EthWallet(const PackedStringArray &mnemonic, const String &passphrase, Error *r_error);

	/**
	 * @brief Adds a new Ethereum wallet account using the provided private key.
	 *
	 * This method either generates a new private key based on the master key
	 * and current account index or uses the provided private key to create
	 * and store a new Ethereum wallet account.
	 *
	 * @param privateKey The private key for the new Ethereum account. If the array is empty,
	 *                a new private key is generated.
	 * @return True if the account was successfully added, false otherwise.
	 */
	bool add(const PackedByteArray &privateKey = {});

	/**
	 * @brief Removes an address from the address book.
	 *
	 * This method takes an address as a parameter and removes it from the
	 * internally maintained address book if it exists.
	 *
	 * @param address The address to be removed from the address book.
	 * @return bool Returns true if the address was successfully removed,
	 *              false if the address was not found in the address book.
	 */
	bool remove_address(PackedByteArray address);

	/**
	 * @brief Removes an account from the wallet at a specified index.
	 *
	 * This method attempts to remove an account located at the given index
	 * within the wallet's list of accounts. If the index is valid, the account
	 * is removed and the method returns true. If the index is out of range, the
	 * method returns false without modifying the wallet.
	 *
	 * @param index The index of the account to be removed.
	 * @return True if the account was successfully removed; false if the index is invalid.
	 */
	bool remove(uint64_t index);

	bool encrypt(PackedStringArray password);

	bool decrypt();

	/**
	 * @brief Clears the data stored in the Ethereum wallet.
	 *
	 * This method removes all accounts, wipes mnemonic data,
	 * and resets both the root and master keys to their default states.
	 *
	 * @return Always returns true indicating the operation was successful.
	 */
	bool clear();

	/**
	 * @brief Retrieves the list of Ethereum accounts.
	 *
	 * This method returns an array containing all the Ethereum accounts
	 * currently managed by the EthWallet instance. Each element in the
	 * array is an account object.
	 *
	 * @return An array of Ethereum account objects.
	 */
	Array get_accounts() const;

	/**
	 * @brief Generates or retrieves the mnemonic phrase for a wallet.
	 *
	 * This method is responsible for providing the mnemonic phrase associated
	 * with a wallet. Mnemonic phrases can be used for wallet recovery and
	 * import into other wallet software.
	 *
	 * @return The mnemonic phrase as a string.
	 */
	PackedStringArray get_mnemonic();


	bool save();

	bool load();
};

/**
 * @class EthWalletManager
 *
 * @brief Provides functionality to manage multiple Ethereum wallets.
 *
 * The EthWalletManager class facilitates the creation, loading, and management
 * of multiple Ethereum wallets. This class offers methods to handle wallet
 * operations such as listing all wallets, retrieving a specific wallet,
 * and performing batch transactions. It serves as an interface to interact
 * with various EthWallet instances, easing the process of wallet administration.
 */
class EthWalletManager : public RefCounted {
    GDCLASS(EthWalletManager, RefCounted);

protected:
    static void _bind_methods();

public:
/**
	 * @brief Creates a new Ethereum wallet.
	 *
	 * This method generates a new Ethereum wallet based on the provided
	 * account count, entropy, and passphrase. If the wallet generation fails,
	 * an empty wallet reference is returned.
	 *
	 * @param strength The number of accounts in the new wallet.
	 * @param entropy Packed byte array representing the entropy used to generate
	 *                the mnemonic phrase.
	 * @param passphrase A passphrase to further secure the wallet.
	 * @return Returns a reference to the newly created EthWallet object. Returns
	 *         an empty reference if wallet creation fails.
	 */
	static Ref<EthWallet> create(int strength = 0, const PackedByteArray& entropy = {}, const String& passphrase = "");

	/**
	 * @brief Creates an Ethereum wallet from the provided mnemonic phrase and passphrase.
	 *
	 * This method instantiates an Ethereum wallet using the provided mnemonic
	 * phrase and optional passphrase. It returns a reference to the created wallet
	 * or an empty reference if creation fails.
	 *
	 * @param mnemonic An array of strings representing the mnemonic phrases.
	 * @param passphrase An optional passphrase to add an extra layer of security.
	 * @return A Ref<EthWallet> instance representing the created wallet, or an empty Ref if creation fails.
	 */
	static Ref<EthWallet> from_mnemonic(const PackedStringArray& mnemonic, const String& passphrase = "");

	/**
	 * Loads an Ethereum wallet instance.
	 *
	 * This method instantiates a new EthWallet object, loads the wallet data, and
	 * returns a reference to the loaded wallet. It is primarily used for retrieving
	 * and initializing an Ethereum wallet within the EthWalletManager context.
	 *
	 * @return A reference to the loaded EthWallet instance.
	 */
	static Ref<EthWallet> load();

	/**
	 * @brief Saves the provided Ethereum wallet.
	 *
	 * This method saves the given EthWallet object. If the wallet reference is invalid,
	 * the method will print an error message and return false.
	 *
	 * @param hd_wallet A reference to the EthWallet object to be saved.
	 * @return Returns true if the wallet was saved successfully, false otherwise.
	 */
	static bool save(Ref<EthWallet> hd_wallet);

};

#endif //ETH_WALLET_H
