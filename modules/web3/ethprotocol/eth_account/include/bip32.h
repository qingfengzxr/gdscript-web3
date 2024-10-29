#ifndef LIBWALLY_CORE_BIP32_H
#define LIBWALLY_CORE_BIP32_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*** bip32-entropy-length BIP32 Seed Entropy Lengths */
#define BIP32_ENTROPY_LEN_128 16 /** 128 bits */
#define BIP32_ENTROPY_LEN_256 32 /** 256 bits */
#define BIP32_ENTROPY_LEN_512 64 /** 512 bits */

/** Length of a BIP32 key fingerprint */
#define BIP32_KEY_FINGERPRINT_LEN 4

/** Length of an ext_key serialized using BIP32 format */
#define BIP32_SERIALIZED_LEN 78

/** Child number of the first hardened child */
#define BIP32_INITIAL_HARDENED_CHILD 0x80000000

/** The maximum number of path elements allowed in a path */
#define BIP32_PATH_MAX_LEN 255

/* Length of a BIP32 chain code  */
#define WALLY_BIP32_CHAIN_CODE_LEN 32

/* Length of Elements' pubkey tweak sum */
#define WALLY_BIP32_TWEAK_SUM_LEN 32

/*** bip32-flags BIP32 Derivation Flags */
/** Indicate that we want to derive a private key in `bip32_key_from_parent` */
#define BIP32_FLAG_KEY_PRIVATE 0x0
/** Indicate that we want to derive a public key in `bip32_key_from_parent` */
#define BIP32_FLAG_KEY_PUBLIC 0x1
/** Indicate that we want to skip hash calculation when deriving a key in `bip32_key_from_parent` */
#define BIP32_FLAG_SKIP_HASH 0x2
/** Elements: Indicate that we want the pub tweak to be added to the calculation when deriving a key in `bip32_key_from_parent` */
#define BIP32_FLAG_KEY_TWEAK_SUM 0x4
/** Allow a wildcard ``*`` or ``*'``/``*h`` in BIP32 path string expressions */
#define BIP32_FLAG_STR_WILDCARD 0x8
/** Do not allow a leading ``m``/``M`` or ``/`` in BIP32 path string expressions */
#define BIP32_FLAG_STR_BARE 0x10
/** Allow upper as well as lower case ``M``/``H`` in BIP32 path string expressions */
#define BIP32_FLAG_ALLOW_UPPER 0x20
/** Allow a multi-path expression ``<child_num_1;child_num_2;..>`` in BIP32 path string expressions */
#define BIP32_FLAG_STR_MULTIPATH 0x40

/*** bip32-version-codes BIP32 extended key versions */
#define BIP32_VER_MAIN_PUBLIC 0x0488B21E /** Mainnet, public key */
#define BIP32_VER_MAIN_PRIVATE 0x0488ADE4 /** Mainnet, private key */
#define BIP32_VER_TEST_PUBLIC 0x043587CF /** Testnet, public key */
#define BIP32_VER_TEST_PRIVATE 0x04358394 /** Testnet, private key */

/*** path-feature-flags BIP32 path feature flags */
#define BIP32_PATH_LEN_MASK 0x000000ff /** Mask for the path length in bits 0-7 */
#define BIP32_PATH_LEN_SHIFT 0x00000000 /** Shift for the path length in bits 0-7 */
#define BIP32_PATH_MULTI_MASK 0x0000ff00 /** Mask for the number of multi-paths in bits 8-15 */
#define BIP32_PATH_MULTI_SHIFT 0x00000008 /** Shift for the number of multi-paths in bits 8-15 */
#define BIP32_PATH_WILDCARD_MASK 0x00ff0000 /** Mask for the wildcard position in bits 16-23 */
#define BIP32_PATH_WILDCARD_SHIFT 0x00000010 /** Shift for the wildcard position in bits 16-23 */
#define BIP32_PATH_IS_BARE 0x01000000 /** Path is bare */
#define BIP32_PATH_IS_HARDENED 0x02000000 /** Path contains hardened elements */
#define BIP32_PATH_IS_WILDCARD 0x04000000 /** Path contains a wildcard */
#define BIP32_PATH_IS_MULTIPATH 0x08000000 /** Path contains a multi-path expression */

/** An extended key */
typedef struct ext_key {
	/** The chain code for this key */
	unsigned char chain_code[32];
	/** The Hash160 of this keys parent */
	unsigned char parent160[20];
	/** The depth of this key */
	uint8_t depth;
	unsigned char pad1[10];
	/** The private key with prefix byte 0 */
	unsigned char priv_key[33];
	/** The child number of the parent key that this key represents */
	uint32_t child_num;
	/** The Hash160 of this key */
	unsigned char hash160[20];
	/** The version code for this key indicating main/testnet and private/public */
	uint32_t version;
	unsigned char pad2[3];
	/** The public key with prefix byte 0x2 or 0x3 */
	unsigned char pub_key[33];
} ETH_EXT_KEY;

/**
 * Create a new master extended key from entropy.
 *
 * This creates a new master key, i.e. the root of a new HD tree.
 * The entropy passed in may produce an invalid key. If this happens,
 * WALLY_ERROR will be returned and the caller should retry with
 * new entropy.
 *
 * :param bytes: Entropy to use.
 * :param bytes_len: Size of ``bytes`` in bytes. Must be one of the :ref:`bip32-entropy-length`
 * :param version: Either `BIP32_VER_MAIN_PRIVATE` or `BIP32_VER_TEST_PRIVATE`,
 *|     indicating mainnet or testnet/regtest respectively.
 * :param hmac_key: Custom data to HMAC-SHA512 with ``bytes`` before creating the key. Pass
 *|             NULL to use the default BIP32 key of "Bitcoin seed".
 * :param hmac_key_len: Size of ``hmac_key`` in bytes, or 0 if ``hmac_key`` is NULL.
 * :param flags: Either `BIP32_FLAG_SKIP_HASH` to skip hash160 calculation, or 0.
 * :param output: Destination for the resulting master extended key.
 */
int bip32_key_from_seed_custom(
		const unsigned char *bytes,
		size_t bytes_len,
		uint32_t version,
		const unsigned char *hmac_key,
		size_t hmac_key_len,
		uint32_t flags,
		struct ext_key *output);

/**
 * As per `bip32_key_from_seed_custom` With the default BIP32 seed.
 */
int bip32_key_from_seed(
		const unsigned char *bytes,
		size_t bytes_len,
		uint32_t version,
		uint32_t flags,
		struct ext_key *output);

/**
 * Create a new child extended key from a parent extended key.
 *
 * :param hdkey: The parent extended key.
 * :param child_num: The child number to create. Numbers greater
 *|           than or equal to `BIP32_INITIAL_HARDENED_CHILD` represent
 *|           hardened keys that cannot be created from public parent
 *|           extended keys.
 * :param flags: :ref:`bip32-flags` indicating the type of derivation wanted.
 *|       You can not derive a private child extended key from a public
 *|       parent extended key.
 * :param output: Destination for the resulting child extended key.
 */
int bip32_key_from_parent(
		const struct ext_key *hdkey,
		uint32_t child_num,
		uint32_t flags,
		struct ext_key *output);

/**
 * Create a new child extended key from a parent extended key and a path.
 *
 * :param hdkey: The parent extended key.
 * :param child_path: The path of child numbers to create.
 * :param child_path_len: The number of child numbers in ``child_path``.
 * :param flags: :ref:`bip32-flags` indicating the type of derivation wanted.
 * :param output: Destination for the resulting child extended key.
 *
 * .. note:: If ``child_path`` contains hardened child numbers, then ``hdkey``
 *|    must be an extended private key or this function will fail.
 */
int bip32_key_from_parent_path(
		const struct ext_key *hdkey,
		const uint32_t *child_path,
		size_t child_path_len,
		uint32_t flags,
		struct ext_key *output);

/**
 * Create a new child extended key from a parent extended key and a path string.
 *
 * :param hdkey: The parent extended key.
 * :param path_str: The BIP32 path string of child numbers to create.
 * :param child_num: The child number to use if ``path_str`` contains a ``*`` wildcard.
 * :param flags: :ref:`bip32-flags` indicating the type of derivation wanted. Note
 *|    that `BIP32_FLAG_STR_MULTIPATH` is not supported. To derive a multi-path
 *|    key, use `bip32_path_from_str` then `bip32_key_from_parent_path`.
 * :param output: Destination for the resulting child extended key.
 *
 * .. note:: If ``child_path`` contains hardened child numbers, then ``hdkey``
 *|    must be an extended private key or this function will fail.
 */
int bip32_key_from_parent_path_str(
		const struct ext_key *hdkey,
		const char *path_str,
		uint32_t child_num,
		uint32_t flags,
		struct ext_key *output);

/**
 * Create a new child extended key from a parent extended key and a known-length path string.
 *
 * See `bip32_key_from_parent_path_str`.
 */
int bip32_key_from_parent_path_str_n(
		const struct ext_key *hdkey,
		const char *path_str,
		size_t path_str_len,
		uint32_t child_num,
		uint32_t flags,
		struct ext_key *output);

/**
 * Converts a private extended key to a public extended key. Afterwards, only public child extended
 * keys can be derived, and only the public serialization can be created.
 * If the provided key is already public, nothing will be done.
 *
 * :param hdkey: The extended key to convert.
 */
int bip32_key_strip_private_key(
		struct ext_key *hdkey);

/**
 * Convert a BIP32 path string to a path.
 *
 * :param path_str: The BIP32 path string of child numbers to convert from.
 * :param child_num: The child number to use if ``path_str`` contains a ``*`` wildcard.
 * :param multi_index: The multi-path item to use if ``path_str`` contains a ``<>`` multi-path.
 * :param flags: :ref:`bip32-flags` controlling path parsing behaviour.
 * :param child_path_out: Destination for the resulting path.
 * :param child_path_out_len: The number of items in ``child_path_out``.
 * :param written: Destination for the number of items written to ``child_path_out``.
 */
int bip32_path_from_str(
		const char *path_str,
		uint32_t child_num,
		uint32_t multi_index,
		uint32_t flags,
		uint32_t *child_path_out,
		uint32_t child_path_out_len,
		size_t *written);

/**
 * Convert a known-length BIP32 path string to an integer path.
 *
 * See `bip32_path_from_str`.
 */
int bip32_path_from_str_n(
		const char *path_str,
		size_t path_str_len,
		uint32_t child_num,
		uint32_t multi_index,
		uint32_t flags,
		uint32_t *child_path_out,
		uint32_t child_path_out_len,
		size_t *written);

/**
 * Get information about the structure of a BIP32 path string.
 *
 * :param path_str: The BIP32 path string of child numbers to convert from.
 * :param value_out: Destination for the resulting :ref:`path-feature-flags`.
 */
int bip32_path_str_get_features(
		const char *path_str,
		uint32_t *value_out);

/**
 * Get information about the structure of a known_length BIP32 path string.
 *
 * See `bip32_path_str_get_features`.
 */
int bip32_path_str_n_get_features(
		const char *path_str,
		size_t path_str_len,
		uint32_t *value_out);

#ifdef __cplusplus
}
#endif

#endif /* LIBWALLY_CORE_BIP32_H */
