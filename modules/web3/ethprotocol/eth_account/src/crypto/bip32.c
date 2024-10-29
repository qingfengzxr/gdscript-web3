#include "bip32.h"
#include "account_utils.h"
#include "eth_ecdsa.h"
#include "ethc-common.h"
#include "hmac.h"
#include "libsecp256k1/include/secp256k1.h"

#include "thirdparty/mbedtls/include/mbedtls/pkcs5.h"
#include "thirdparty/mbedtls/include/mbedtls/ripemd160.h"
#include "thirdparty/mbedtls/include/mbedtls/sha256.h"
#include "thirdparty/mbedtls/include/mbedtls/sha512.h"

#define BIP32_ALL_DEFINED_FLAGS (BIP32_FLAG_KEY_PRIVATE | \
		BIP32_FLAG_KEY_PUBLIC |                           \
		BIP32_FLAG_SKIP_HASH |                            \
		BIP32_FLAG_KEY_TWEAK_SUM |                        \
		BIP32_FLAG_STR_WILDCARD |                         \
		BIP32_FLAG_STR_BARE |                             \
		BIP32_FLAG_ALLOW_UPPER |                          \
		BIP32_FLAG_STR_MULTIPATH)
unsigned char HMAC_KEY[] = {
	'B', 'i', 't', 'c', 'o', 'i', 'n', ' ', 's', 'e', 'e', 'd'
};

#define ENDIAN_CAST
#define ENDIAN_TYPE

typedef uint32_t ENDIAN_TYPE beint32_t;

#define BSWAP_32(val) \
	((((uint32_t)(val) & 0x000000ff) << 24) | (((uint32_t)(val) & 0x0000ff00) << 8) | (((uint32_t)(val) & 0x00ff0000) >> 8) | (((uint32_t)(val) & 0xff000000) >> 24))

#define CPU_TO_BE32(native) ((ENDIAN_CAST beint32_t)BSWAP_32(native))

static char *byteArrayToHexString(const uint8_t *src, size_t len) {
	static const char hex_digits[] = "0123456789abcdef";
	char *hex = malloc(len * 2 + 1);

	for (size_t i = 0; i < len; ++i) {
		hex[i * 2] = hex_digits[(src[i] >> 4) & 0x0F];
		hex[i * 2 + 1] = hex_digits[src[i] & 0x0F];
	}
	hex[len * 2] = '\0';

	return hex;
}

static bool child_is_hardened(uint32_t child_num) {
	return child_num >= BIP32_INITIAL_HARDENED_CHILD;
}

static bool version_is_valid(uint32_t ver, uint32_t flags) {
	if (ver == BIP32_VER_MAIN_PRIVATE || ver == BIP32_VER_TEST_PRIVATE)
		return true;

	return flags == BIP32_FLAG_KEY_PUBLIC &&
			(ver == BIP32_VER_MAIN_PUBLIC || ver == BIP32_VER_TEST_PUBLIC);
}

static bool version_is_mainnet(uint32_t ver) {
	return ver == BIP32_VER_MAIN_PRIVATE || ver == BIP32_VER_MAIN_PUBLIC;
}

static bool is_hardened_indicator(char c, bool allow_upper, uint32_t *features) {
	if (c == '\'' || c == 'h' || (allow_upper && c == 'H')) {
		*features |= BIP32_PATH_IS_HARDENED;
		return true;
	}
	return false;
}

static int path_from_str_n(const char *str, size_t str_len,
		uint32_t child_num, uint32_t multi_index,
		uint32_t *features, uint32_t flags,
		uint32_t *child_path, uint32_t child_path_len,
		size_t *written) {
	const bool allow_upper = flags & BIP32_FLAG_ALLOW_UPPER;
	size_t start, multi_start, num_multi = 0, i = 0;
	uint64_t v;

	*features = 0;

	if (!str || !str_len || child_num >= BIP32_INITIAL_HARDENED_CHILD ||
			(flags & ~BIP32_ALL_DEFINED_FLAGS) ||
			(!(flags & BIP32_FLAG_STR_WILDCARD) && child_num) ||
			(!(flags & BIP32_FLAG_STR_MULTIPATH) && multi_index) ||
			!child_path || !child_path_len || !written)
		goto fail;

	*written = 0;

	if (flags & BIP32_FLAG_STR_BARE) {
		if (i < str_len && str[i] == '/')
			goto fail; /* bare path must start with a number */
		*features |= BIP32_PATH_IS_BARE;
	} else {
		start = i;
		if (i < str_len && (str[i] == 'm' || (allow_upper && str[i] == 'M')))
			++i; /* Skip */
		if (i < str_len && str[i] == '/')
			++i; /* Skip */
		if (start == i)
			*features |= BIP32_PATH_IS_BARE;
	}

	while (i < str_len) {
		size_t multi;
		bool is_wildcard = false, is_multi = false;
		start = i;
		v = 0;
		if (str[i] == '<' && (flags & BIP32_FLAG_STR_MULTIPATH)) {
			/* Multi-path expression - skip to the required multi_index */
			++i;
			if (i < str_len && (str[i] < '0' || str[i] > '9'))
				goto fail; /* Missing initial child */
			*features |= BIP32_PATH_IS_MULTIPATH;
			is_multi = true;
			for (multi = 0; i < str_len && multi < multi_index; ++multi) {
				multi_start = i;
				while (i < str_len && str[i] >= '0' && str[i] <= '9')
					++i;
				if (i == multi_start)
					goto fail; /* Missing multi-item number */
				if (i < str_len && is_hardened_indicator(str[i], allow_upper, features))
					++i;
				if (i == str_len || str[i] != ';')
					goto fail; /* Malformed multi or incorrect index */
				++i;
				++num_multi;
			}
			if (multi < multi_index && i == start + 1)
				goto fail; /* No number found */
		}
		while (i < str_len && str[i] >= '0' && str[i] <= '9') {
			v = v * 10 + (str[i] - '0');
			if (v >= BIP32_INITIAL_HARDENED_CHILD)
				goto fail; /* Derivation index too large */
			++i;
		}
		if (i == start) {
			/* No number found */
			if (is_multi)
				goto fail; /* Must have a number for multi */
			if (str[i] == '/') {
				if (i && (str[i - 1] < '0' || str[i - 1] > '9') &&
						!is_hardened_indicator(str[i - 1], allow_upper, features) &&
						str[i - 1] != '*' && str[i - 1] != '>') {
					/* Only valid after number/wildcard/hardened/multi-end */
					goto fail;
				}
				++i;
				if (i == str_len || str[i] == '/')
					goto fail; /* Trailing slash, invalid */
				continue;
			}
			if (!(is_wildcard = str[i] == '*'))
				goto fail; /* Unknown character */

			/* Wildcard */
			if (!(flags & BIP32_FLAG_STR_WILDCARD))
				goto fail; /* Wildcard not allowed, or previously seen */
			flags &= ~BIP32_FLAG_STR_WILDCARD; /* Only allow one wildcard */
			*features |= BIP32_PATH_IS_WILDCARD;
			*features |= (*written << BIP32_PATH_WILDCARD_SHIFT);
			if (i && str[i - 1] != '/')
				goto fail; /* Must follow a slash */
			++i;
			v = child_num; /* Use the given child number for the wildcard value */
		}
		if (i < str_len && is_hardened_indicator(str[i], allow_upper, features)) {
			v |= BIP32_INITIAL_HARDENED_CHILD;
			++i;
		}
		if (is_multi) {
			/* Multi-path expression - skip remaining multi-path items */
			++num_multi; /* For the item we just read */
			while (i < str_len && str[i] != '>') {
				if (str[i] == ';') {
					++i;
					++num_multi;
				}
				multi_start = i;
				while (i < str_len && str[i] >= '0' && str[i] <= '9')
					++i;
				if (i == multi_start)
					goto fail; /* Missing multi-item number */
				if (i < str_len && is_hardened_indicator(str[i], allow_upper, features))
					++i;
			}
			if (i == str_len || str[i] != '>')
				goto fail; /* Malformed multi or incorrect index */
			++i;
			if (num_multi < 2 || num_multi >= 255)
				goto fail;
			*features |= (num_multi << BIP32_PATH_MULTI_SHIFT);
			flags &= ~BIP32_FLAG_STR_MULTIPATH; /* Only allow one multi-path */
		}

		if (is_wildcard && i != str_len && str[i] != '/')
			goto fail; /* Wildcard followed by something other than a slash */
		if (*written == child_path_len) {
			/* Continue counting the resulting length, but don't write any more */
			child_path = NULL;
		}
		if (child_path)
			child_path[*written] = v;
		++*written;
	}

	if (*written && *written <= BIP32_PATH_MAX_LEN &&
			(!child_num || (*features & BIP32_PATH_IS_WILDCARD)) &&
			(!multi_index || (*features & BIP32_PATH_IS_MULTIPATH))) {
		*features |= (*written << BIP32_PATH_LEN_SHIFT);
		return WEB3_OK;
	}

fail:
	*features = 0;
	if (written)
		*written = 0;
	return WEB3_EINVAL;
}

int bip32_path_from_str_n(const char *str, size_t str_len,
		uint32_t child_num, uint32_t multi_index,
		uint32_t flags,
		uint32_t *child_path, uint32_t child_path_len,
		size_t *written) {
	uint32_t features;
	return path_from_str_n(str, str_len, child_num, multi_index, &features,
			flags, child_path, child_path_len, written);
}

int bip32_path_from_str(const char *str, uint32_t child_num,
		uint32_t multi_index, uint32_t flags,
		uint32_t *child_path, uint32_t child_path_len,
		size_t *written) {
	uint32_t features;
	return path_from_str_n(str, str ? strlen(str) : 0, child_num,
			multi_index, &features, flags,
			child_path, child_path_len, written);
}

int bip32_path_str_n_get_features(const char *str, size_t str_len,
		uint32_t *value_out) {
	uint32_t dummy, child_num = 0, multi_index = 0;
	uint32_t flags = BIP32_FLAG_STR_WILDCARD | BIP32_FLAG_ALLOW_UPPER |
			BIP32_FLAG_STR_MULTIPATH;
	size_t written;
	return path_from_str_n(str, str_len, child_num, multi_index, value_out,
			flags, &dummy, 1, &written);
}

int bip32_path_str_get_features(const char *str, uint32_t *value_out) {
	return bip32_path_str_n_get_features(str, str ? strlen(str) : 0, value_out);
}

static bool key_is_private(const struct ext_key *hdkey) {
	return hdkey->priv_key[0] == BIP32_FLAG_KEY_PRIVATE;
}

/* Compute a public key from a private key */
static int key_compute_pub_key(struct ext_key *key_out) {
	return eth_ecdsa_pubkey_get_with_compressed(key_out->pub_key, key_out->priv_key + 1);
}

static bool is_valid_seed_len(size_t len) {
	return len == BIP32_ENTROPY_LEN_512 || len == BIP32_ENTROPY_LEN_256 ||
			len == BIP32_ENTROPY_LEN_128;
}

/* Wipe a key and return failure for the caller to propigate */
static int wipe_key_fail(struct ext_key *key_out) {
	memset(key_out, 0, sizeof(*key_out));
	return WEB3_EINVAL;
}

int bip32_key_from_private_key(uint32_t version,
		const unsigned char *priv_key, size_t priv_key_len,
		struct ext_key *key_out) {
	const secp256k1_context *ctx;

	if (key_out)
		memset(key_out, 0, sizeof(*key_out));

	ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
	if (ctx == NULL)
		return -1;

	if (!version_is_valid(version, BIP32_FLAG_KEY_PRIVATE) ||
			!priv_key || priv_key_len != EC_PRIVATE_KEY_LEN || !key_out)
		return WEB3_EINVAL;

	/* Check that the generated private key is valid */
	if (!secp256k1_ec_seckey_verify(ctx, priv_key)) {
		return WEB3_ERROR; /* Invalid private key */
	}

	key_out->version = version;
	/* Copy the private key and set its prefix */
	key_out->priv_key[0] = BIP32_FLAG_KEY_PRIVATE;
	memcpy(key_out->priv_key + 1, priv_key, priv_key_len);
	/* Compute the public key */
	if (key_compute_pub_key(key_out) != 1)
		return wipe_key_fail(key_out);

	/* Returned key is partial; it must be further initialized for deriving */
	return WEB3_OK;
}

int bip32_key_from_seed_custom(const unsigned char *bytes, size_t bytes_len,
		uint32_t version,
		const unsigned char *hmac_key, size_t hmac_key_len,
		uint32_t flags, struct ext_key *key_out) {
	int ret;

	if (key_out)
		memset(key_out, 0, sizeof(*key_out));

	if (!bytes || !is_valid_seed_len(bytes_len) ||
			(hmac_key == NULL) != (hmac_key_len == 0) ||
			(flags & ~BIP32_FLAG_SKIP_HASH) || !key_out)
		return WEB3_EINVAL;

	if (!hmac_key) {
		hmac_key = HMAC_KEY; /* Use the default BIP32 hmac key */
		hmac_key_len = sizeof(HMAC_KEY);
	}

	unsigned char u8[64];
	/* Generate private key and chain code */
	hmac_sha512(hmac_key, hmac_key_len, bytes, bytes_len, u8);

	ret = bip32_key_from_private_key(version, u8, EC_PRIVATE_KEY_LEN, key_out);

	if (ret == WEB3_OK) {
		/* Copy the chain code and set other members */
		memcpy(key_out->chain_code, u8 + 32, 32);
		key_out->depth = 0; /* Master key, depth 0 */
		key_out->child_num = 0;
	}
	memset(u8, 0, sizeof(u8));
	return ret;
}

int bip32_key_from_seed(const unsigned char *bytes, size_t bytes_len,
		uint32_t version, uint32_t flags,
		struct ext_key *key_out) {
	return bip32_key_from_seed_custom(bytes, bytes_len, version,
			NULL, 0, flags, key_out);
}

/* BIP32: Child Key Derivations
 *
 * The spec doesn't have a simple table of derivations, its:
 *
 * Parent   Child    Hardened  Status  Path  In Spec
 * private  private  no        OK      m/n   Y
 * private  private  yes       OK      m/nH  Y
 * private  public   no        OK      -     N
 * private  public   yes       OK      -     N
 * public   private  no        FAIL   (N/A) (N/A)
 * public   private  yes       FAIL   (N/A) (N/A)
 * public   public   no        OK      M/n   N
 * public   public   yes       FAIL    M/nH (N/A)
 *
 * The spec path nomenclature only expresses derivations where the parent
 * and desired child type match. For private->public the derivation is
 * described in terms of private-private and public->public, but there are
 * no test vectors or paths describing these values to validate against.
 * Further, there are no public-public vectors in the BIP32 spec either.
 */
int bip32_key_from_parent(const struct ext_key *hdkey, uint32_t child_num,
		uint32_t flags, struct ext_key *key_out) {
	//struct sha512 sha;
	const secp256k1_context *ctx;
	const bool we_are_private = hdkey && key_is_private(hdkey);
	const bool derive_private = !(flags & BIP32_FLAG_KEY_PUBLIC);
	const bool hardened = child_is_hardened(child_num);

	if (flags & ~BIP32_ALL_DEFINED_FLAGS)
		return WEB3_EINVAL; /* These flags are not defined yet */

	if (!hdkey || !key_out)
		return WEB3_EINVAL;

	ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
	if (ctx == NULL)
		return -1;

	if (!we_are_private && (derive_private || hardened))
		return wipe_key_fail(key_out); /* Unsupported derivation */

	if (hdkey->depth == 0xff)
		return wipe_key_fail(key_out); /* Maximum depth reached */

	/*
	 *  Private parent -> private child:
	 *    CKDpriv((kpar, cpar), i) -> (ki, ci)
	 *
	 *  Private parent -> public child:
	 *    N(CKDpriv((kpar, cpar), i) -> (ki, ci))
	 *  As we always calculate the public key, we can derive a public
	 *  child by deriving a private one and stripping its private key.
	 *
	 * Public parent -> non hardened public child
	 *    CKDpub((Kpar, cpar), i) -> (Ki, ci)
	 */

	/* NB: We use the key_outs' priv_key+child_num to hold 'Data' here */
	if (hardened) {
		/* Hardened: Data = 0x00 || ser256(kpar) || ser32(i)) */
		memcpy(key_out->priv_key, hdkey->priv_key, sizeof(hdkey->priv_key));
	} else {
		/* Non Hardened Private: Data = serP(point(kpar)) || ser32(i)
		 * Non Hardened Public : Data = serP(kpar) || ser32(i)
		 *   point(kpar) when par is private is the public key.
		 */
		memcpy(key_out->priv_key, hdkey->pub_key, sizeof(hdkey->pub_key));
	}

	/* This is the '|| ser32(i)' part of the above */
	key_out->child_num = CPU_TO_BE32(child_num);
	unsigned char u8[64] = {};
	/* I = HMAC-SHA512(Key = cpar, Data) */
	hmac_sha512(hdkey->chain_code, sizeof(hdkey->chain_code),
			key_out->priv_key, sizeof(key_out->priv_key) + sizeof(key_out->child_num), u8);

	memcpy(key_out->chain_code, u8 + 32, 32);

	if (we_are_private) {
		/* The returned child key ki is parse256(IL) + kpar (mod n)
		 * In case parse256(IL) ≥ n or ki = 0, the resulting key is invalid
		 * (NOTE: seckey_tweak_add checks both conditions)
		 */
		memcpy(key_out->priv_key, hdkey->priv_key, sizeof(hdkey->priv_key));

		if (seckey_tweak_add(key_out->priv_key + 1, u8) ||
				key_compute_pub_key(key_out) != 1)
			goto fail;
	} else {
		/* The returned child key ki is point(parse256(IL) + kpar)
		 * In case parse256(IL) ≥ n or Ki is the point at infinity, the
		 * resulting key is invalid (NOTE: pubkey_tweak_add checks both
		 * conditions)
		 */
		secp256k1_pubkey pub_key;
		size_t len = sizeof(key_out->pub_key);

		/* FIXME: Out of bounds on pubkey_tweak_add */
		if (!pubkey_parse(&pub_key, hdkey->pub_key, sizeof(hdkey->pub_key)) ||
				!secp256k1_ec_pubkey_tweak_add(ctx, &pub_key, u8) ||
				!pubkey_serialize(key_out->pub_key, &len, &pub_key,
						SECP256K1_EC_COMPRESSED) ||
				len != sizeof(key_out->pub_key)) {
			goto fail;
		}
	}

	if (derive_private) {
		if (version_is_mainnet(hdkey->version))
			key_out->version = BIP32_VER_MAIN_PRIVATE;
		else
			key_out->version = BIP32_VER_TEST_PRIVATE;

	} else {
		if (version_is_mainnet(hdkey->version))
			key_out->version = BIP32_VER_MAIN_PUBLIC;
		else
			key_out->version = BIP32_VER_TEST_PUBLIC;

		bip32_key_strip_private_key(key_out);
	}

	key_out->depth = hdkey->depth + 1;
	key_out->child_num = child_num;
	// if (flags & BIP32_FLAG_SKIP_HASH) {
	//     memset(&key_out->parent160,0,sizeof(key_out->parent160));
	// 	memset(&key_out->hash160,0,sizeof(key_out->hash160));
	// }
	// else {
	//     memcpy(key_out->parent160, hdkey->hash160, sizeof(hdkey->hash160));
	//     key_compute_hash160(key_out);
	// }
	memset(u8, 0, sizeof(u8));
	return WEB3_OK;
fail:
	memset(u8, 0, sizeof(u8));
	return wipe_key_fail(key_out);
}

int bip32_key_from_parent_path(const struct ext_key *hdkey,
		const uint32_t *child_path, size_t child_path_len,
		uint32_t flags, struct ext_key *key_out) {
	struct ext_key tmp[2];
	size_t i, tmp_idx = 0, private_until = 0;
	int ret;

	if (flags & ~BIP32_ALL_DEFINED_FLAGS)
		return WEB3_EINVAL; /* These flags are not defined yet */

	if (!hdkey || !child_path || !child_path_len || child_path_len > BIP32_PATH_MAX_LEN || !key_out)
		return WEB3_EINVAL;

	if (flags & BIP32_FLAG_KEY_PUBLIC) {
		/* Public derivation: Check for intermediate hardened keys */
		for (i = 0; i < child_path_len; ++i) {
			if (child_is_hardened(child_path[i]))
				private_until = i + 1; /* Derive privately until this index */
		}
		if (private_until && !key_is_private(hdkey))
			return WEB3_EINVAL; /* Unsupported derivation */
	}

	for (i = 0; i < child_path_len; ++i) {
		struct ext_key *derived = &tmp[tmp_idx];
		uint32_t derivation_flags = flags;

		if (private_until && i < private_until - 1) {
			/* Derive privately until we reach the last hardened child */
			derivation_flags &= ~BIP32_FLAG_KEY_PUBLIC;
			derivation_flags |= BIP32_FLAG_KEY_PRIVATE;
		}
		if (i + 2 < child_path_len)
			derivation_flags |= BIP32_FLAG_SKIP_HASH; /* Skip hash for internal keys */

		ret = bip32_key_from_parent(hdkey, child_path[i], derivation_flags, derived);
		if (ret != WEB3_OK)
			break;

		hdkey = derived; /* Derived becomes next parent */
		tmp_idx = !tmp_idx; /* Use free slot in tmp for next derived */
	}

	if (ret == WEB3_OK)
		memcpy(key_out, hdkey, sizeof(*key_out));

	memset(tmp, 0, sizeof(*tmp));
	return ret;
}

int bip32_key_from_parent_path_str_n(const struct ext_key *hdkey,
		const char *str, size_t str_len,
		uint32_t child_num, uint32_t flags,
		struct ext_key *key_out) {
	uint32_t path[BIP32_PATH_MAX_LEN], *path_p = path, features;
	const uint32_t multi_index = 0; /* Multi-path not supported */
	size_t written;
	if (flags & BIP32_FLAG_STR_MULTIPATH)
		return WEB3_EINVAL; /* Multi-path is not supported for this call */
	int ret = path_from_str_n(str, str_len, child_num, multi_index, &features,
			flags, path_p, BIP32_PATH_MAX_LEN, &written);

	if (ret == WEB3_OK)
		ret = bip32_key_from_parent_path(hdkey, path, written, flags, key_out);

	return ret;
}

int bip32_key_from_parent_path_str(const struct ext_key *hdkey,
		const char *str,
		uint32_t child_num, uint32_t flags,
		struct ext_key *key_out) {
	return bip32_key_from_parent_path_str_n(hdkey, str, str ? strlen(str) : 0,
			child_num, flags, key_out);
}

int bip32_key_strip_private_key(struct ext_key *key_out) {
	if (!key_out)
		return WEB3_EINVAL;
	key_out->priv_key[0] = BIP32_FLAG_KEY_PUBLIC;
	memset(key_out->priv_key + 1, 0, sizeof(key_out->priv_key) - 1);
	return WEB3_OK;
}