#include "bip32.h"

#include "account_utils.h"
#include "ethc-common.h"

#include "thirdparty/mbedtls/include/mbedtls/sha256.h"
#include "thirdparty/mbedtls/include/mbedtls/sha512.h"
#include "thirdparty/mbedtls/include/mbedtls/pkcs5.h"
#include "thirdparty/mbedtls/include/mbedtls/ripemd160.h"

#include "libsecp256k1/include/secp256k1.h"

#include "hmac.h"

#include "eth_ecdsa.h"


char* byteArrayToHexString(const uint8_t* src,size_t len) {
	static const char hex_digits[] = "0123456789abcdef";
	char* hex = malloc(len * 2 + 1);

	for (size_t i = 0; i < len; ++i) {
		hex[i * 2] = hex_digits[(src[i] >> 4) & 0x0F];
		hex[i * 2 + 1] = hex_digits[src[i] & 0x0F];
	}
	hex[len * 2] = '\0';

	return hex;
}

#define BIP32_ALL_DEFINED_FLAGS (BIP32_FLAG_KEY_PRIVATE | \
                                 BIP32_FLAG_KEY_PUBLIC | \
                                 BIP32_FLAG_SKIP_HASH | \
                                 BIP32_FLAG_KEY_TWEAK_SUM | \
                                 BIP32_FLAG_STR_WILDCARD | \
                                 BIP32_FLAG_STR_BARE | \
                                 BIP32_FLAG_ALLOW_UPPER | \
                                 BIP32_FLAG_STR_MULTIPATH)
unsigned char HMAC_KEY[] = {
    'B', 'i', 't', 'c', 'o', 'i', 'n', ' ', 's', 'e', 'e', 'd'
};
#define ENDIAN_CAST
#define ENDIAN_TYPE
typedef uint32_t ENDIAN_TYPE beint32_t;

#define BSWAP_32(val)					\
	((((uint32_t)(val) & 0x000000ff) << 24)		\
	 | (((uint32_t)(val) & 0x0000ff00) << 8)		\
	 | (((uint32_t)(val) & 0x00ff0000) >> 8)		\
	 | (((uint32_t)(val) & 0xff000000) >> 24))

#define CPU_TO_BE32(native) ((ENDIAN_CAST beint32_t)BSWAP_32(native))

static bool child_is_hardened(uint32_t child_num)
{
    return child_num >= BIP32_INITIAL_HARDENED_CHILD;
}

static bool version_is_valid(uint32_t ver, uint32_t flags)
{
    if (ver == BIP32_VER_MAIN_PRIVATE || ver == BIP32_VER_TEST_PRIVATE)
        return true;

    return flags == BIP32_FLAG_KEY_PUBLIC &&
           (ver == BIP32_VER_MAIN_PUBLIC || ver == BIP32_VER_TEST_PUBLIC);
}

static bool version_is_mainnet(uint32_t ver)
{
    return ver == BIP32_VER_MAIN_PRIVATE || ver == BIP32_VER_MAIN_PUBLIC;
}

static bool is_hardened_indicator(char c, bool allow_upper, uint32_t *features)
{
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
                           size_t *written)
{
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
                          size_t *written)
{
    uint32_t features;
    return path_from_str_n(str, str_len, child_num, multi_index, &features,
                           flags, child_path, child_path_len, written);
}

int bip32_path_from_str(const char *str, uint32_t child_num,
                        uint32_t multi_index, uint32_t flags,
                        uint32_t *child_path, uint32_t child_path_len,
                        size_t *written)
{
    uint32_t features;
    return path_from_str_n(str, str ? strlen(str) : 0, child_num,
                           multi_index, &features, flags,
                           child_path, child_path_len, written);
}

int bip32_path_str_n_get_features(const char *str, size_t str_len,
                                  uint32_t *value_out)
{
    uint32_t dummy, child_num = 0, multi_index = 0;
    uint32_t flags = BIP32_FLAG_STR_WILDCARD | BIP32_FLAG_ALLOW_UPPER |
                     BIP32_FLAG_STR_MULTIPATH;
    size_t written;
    return path_from_str_n(str, str_len, child_num, multi_index, value_out,
                           flags, &dummy, 1, &written);
}

int bip32_path_str_get_features(const char *str, uint32_t *value_out)
{
    return bip32_path_str_n_get_features(str, str ? strlen(str) : 0, value_out);
}

static bool key_is_private(const struct ext_key *hdkey)
{
    return hdkey->priv_key[0] == BIP32_FLAG_KEY_PRIVATE;
}

/* Compute a public key from a private key */
static int key_compute_pub_key(struct ext_key *key_out)
{
	return eth_ecdsa_pubkey_get_with_compressed(key_out->pub_key,key_out->priv_key + 1);
                                      sizeof(key_out->pub_key));
}

static void key_compute_hash160(struct ext_key *key_out)
{
    // wally_hash160(key_out->pub_key, sizeof(key_out->pub_key),
    //               key_out->hash160, sizeof(key_out->hash160));
}

int bip32_key_free(const struct ext_key *hdkey)
{
    if (!hdkey)
        return WEB3_EINVAL;
	memset((void*)hdkey, 0, sizeof(*hdkey));
	free((void*)hdkey);
    return WEB3_OK;
}

// static bool is_valid_parent160_len(size_t len) {
//     /* Allow partial as well as full fingerprints */
//     return len == BIP32_KEY_FINGERPRINT_LEN || len == key_size(parent160);
// }

static bool is_valid_seed_len(size_t len) {
    return len == BIP32_ENTROPY_LEN_512 || len == BIP32_ENTROPY_LEN_256 ||
           len == BIP32_ENTROPY_LEN_128;
}

/* Wipe a key and return failure for the caller to propigate */
static int wipe_key_fail(struct ext_key *key_out)
{
	memset(key_out, 0, sizeof(*key_out));
    return WEB3_EINVAL;
}

int bip32_key_from_private_key(uint32_t version,
                               const unsigned char *priv_key, size_t priv_key_len,
                               struct ext_key *key_out)
{
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
                               uint32_t flags, struct ext_key *key_out)
{
    //struct sha512 sha;
    int ret;

    if (key_out)
        memset(key_out,0,sizeof(*key_out));
//        wally_clear(key_out, sizeof(*key_out));

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
    hmac_sha512(hmac_key,hmac_key_len,bytes,bytes_len,u8);
    //hmac_sha512_impl(&sha, hmac_key, hmac_key_len, bytes, bytes_len);

    ret = bip32_key_from_private_key(version, u8, EC_PRIVATE_KEY_LEN, key_out);

    //ret = bip32_key_from_private_key(version, sha.u.u8, EC_PRIVATE_KEY_LEN, key_out);
    if (ret == WEB3_OK) {
        /* Copy the chain code and set other members */
        //memcpy(key_out->chain_code, sha.u.u8 + sizeof(sha) / 2, sizeof(sha) / 2);
        memcpy(key_out->chain_code, u8 + 32, 32);
        //memcpy(key_out->chain_code, sha.u.u8 + sizeof(sha) / 2, sizeof(sha) / 2);
        key_out->depth = 0; /* Master key, depth 0 */
        key_out->child_num = 0;
        // if (!(flags & BIP32_FLAG_SKIP_HASH))
        //     key_compute_hash160(key_out);
    }
    memset(u8,0,sizeof(u8));
    //wally_clear(&sha, sizeof(sha));
    return ret;
}

int bip32_key_from_seed(const unsigned char *bytes, size_t bytes_len,
                        uint32_t version, uint32_t flags,
                        struct ext_key *key_out)
{
    return bip32_key_from_seed_custom(bytes, bytes_len, version,
                                      NULL, 0, flags, key_out);
}
//
// #define ALLOC_KEY() \
//     if (!output) \
//         return WEB3_EINVAL; \
//     *output = wally_calloc(sizeof(struct ext_key)); \
//     if (!*output) \
//         return WEB3_ENOMEM
//
// int bip32_key_from_seed_custom_alloc(const unsigned char *bytes, size_t bytes_len,
//                                      uint32_t version,
//                                      const unsigned char *hmac_key, size_t hmac_key_len,
//                                      uint32_t flags, struct ext_key **output)
// {
//     int ret;
//
//     ALLOC_KEY();
//     ret = bip32_key_from_seed_custom(bytes, bytes_len, version,
//                                      hmac_key, hmac_key_len, flags, *output);
//     if (ret != WEB3_OK) {
//     	free(*output);
//         *output = NULL;
//     }
//     return ret;
// }
//
// int bip32_key_from_seed_alloc(const unsigned char *bytes, size_t bytes_len,
//                               uint32_t version, uint32_t flags,
//                               struct ext_key **output)
// {
//     return bip32_key_from_seed_custom_alloc(bytes, bytes_len, version,
//                                             NULL, 0, flags, output);
// }
//
// static unsigned char *copy_out(unsigned char *dest,
//                                const void *src, size_t len)
// {
//     memcpy(dest, src, len);
//     return dest + len;
// }
//
// static bool key_is_valid(const struct ext_key *hdkey)
// {
//     bool is_private = key_is_private(hdkey);
//     bool is_master = !hdkey->depth;
//     uint8_t ver_flags = is_private ? BIP32_FLAG_KEY_PRIVATE : BIP32_FLAG_KEY_PUBLIC;
//
//     if (!version_is_valid(hdkey->version, ver_flags))
//         return false;
//
//     if (mem_is_zero(hdkey->chain_code, sizeof(hdkey->chain_code)) ||
//         (hdkey->pub_key[0] != 0x2 && hdkey->pub_key[0] != 0x3) ||
//         mem_is_zero(hdkey->pub_key + 1, sizeof(hdkey->pub_key) - 1))
//         return false;
//
//     if (hdkey->priv_key[0] != BIP32_FLAG_KEY_PUBLIC &&
//         hdkey->priv_key[0] != BIP32_FLAG_KEY_PRIVATE)
//         return false;
//
//     if (is_private &&
//         mem_is_zero(hdkey->priv_key + 1, sizeof(hdkey->priv_key) - 1))
//         return false;
//
//     if (is_master &&
//         !mem_is_zero(hdkey->parent160, sizeof(hdkey->parent160)))
//         return false;
//
//     return true;
// }
//
// int bip32_key_serialize(const struct ext_key *hdkey, uint32_t flags,
//                         unsigned char *bytes_out, size_t len)
// {
//     const bool serialize_private = !(flags & BIP32_FLAG_KEY_PUBLIC);
//     unsigned char *out = bytes_out;
//     uint32_t tmp32;
//     beint32_t tmp32_be;
//
//     if (flags & ~BIP32_FLAG_KEY_PUBLIC)
//         return WEB3_EINVAL; /* Only this flag makes sense here */
//
//     /* Validate our arguments and then the input key */
//     if (!hdkey ||
//         (serialize_private && !key_is_private(hdkey)) ||
//         !key_is_valid(hdkey) ||
//         !bytes_out || len != BIP32_SERIALIZED_LEN)
//         return WEB3_EINVAL;
//
//     tmp32 = hdkey->version;
//     if (!serialize_private) {
//         /* Change version if serializing the public part of a private key */
//         if (tmp32 == BIP32_VER_MAIN_PRIVATE)
//             tmp32 = BIP32_VER_MAIN_PUBLIC;
//         else if (tmp32 == BIP32_VER_TEST_PRIVATE)
//             tmp32 = BIP32_VER_TEST_PUBLIC;
//     }
//     tmp32_be = cpu_to_be32(tmp32);
//     out = copy_out(out, &tmp32_be, sizeof(tmp32_be));
//
//     *out++ = hdkey->depth;
//
//     /* Save the first 32 bits of the parent key (aka fingerprint) only */
//     out = copy_out(out, hdkey->parent160, BIP32_KEY_FINGERPRINT_LEN);
//
//     tmp32_be = cpu_to_be32(hdkey->child_num);
//     out = copy_out(out, &tmp32_be, sizeof(tmp32_be));
//
//     out = copy_out(out, hdkey->chain_code, sizeof(hdkey->chain_code));
//
//     if (serialize_private)
//         copy_out(out, hdkey->priv_key, sizeof(hdkey->priv_key));
//     else
//         copy_out(out, hdkey->pub_key, sizeof(hdkey->pub_key));
//
//     return WEB3_OK;
// }
//
// static const unsigned char *copy_in(void *dest,
//                                     const unsigned char *src, size_t len)
// {
//     memcpy(dest, src, len);
//     return src + len;
// }
//
// int bip32_key_unserialize(const unsigned char *bytes, size_t bytes_len,
//                           struct ext_key *key_out)
// {
//     if (!bytes || bytes_len != BIP32_SERIALIZED_LEN || !key_out)
//         return WEB3_EINVAL;
//
// 	memset(key_out, 0, sizeof(*key_out));
//
//     bytes = copy_in(&key_out->version, bytes, sizeof(key_out->version));
//     key_out->version = be32_to_cpu(key_out->version);
//     if (!version_is_valid(key_out->version, BIP32_FLAG_KEY_PUBLIC))
//         return wipe_key_fail(key_out);
//
//     bytes = copy_in(&key_out->depth, bytes, sizeof(key_out->depth));
//
//     /* We only have a partial fingerprint available. Copy it, but the
//      * user will need to call bip32_key_set_parent() (FIXME: Implement)
//      * later if they want it to be fully populated.
//      */
//     bytes = copy_in(key_out->parent160, bytes, BIP32_KEY_FINGERPRINT_LEN);
//     bytes = copy_in(&key_out->child_num, bytes, sizeof(key_out->child_num));
//     key_out->child_num = be32_to_cpu(key_out->child_num);
//     bytes = copy_in(key_out->chain_code, bytes, sizeof(key_out->chain_code));
//
//     if (bytes[0] == BIP32_FLAG_KEY_PRIVATE) {
//         if (key_out->version == BIP32_VER_MAIN_PUBLIC ||
//             key_out->version == BIP32_VER_TEST_PUBLIC)
//             return wipe_key_fail(key_out); /* Private key data in public key */
//
//         copy_in(key_out->priv_key, bytes, sizeof(key_out->priv_key));
//         if (key_compute_pub_key(key_out) != WEB3_OK)
//             return wipe_key_fail(key_out);
//     } else {
//         if (key_out->version == BIP32_VER_MAIN_PRIVATE ||
//             key_out->version == BIP32_VER_TEST_PRIVATE)
//             return wipe_key_fail(key_out); /* Public key data in private key */
//
//         copy_in(key_out->pub_key, bytes, sizeof(key_out->pub_key));
//         bip32_key_strip_private_key(key_out);
//     }
//
//     key_compute_hash160(key_out);
//     return WEB3_OK;
// }
//
// int bip32_key_unserialize_alloc(const unsigned char *bytes, size_t bytes_len,
//                                 struct ext_key **output)
// {
//     int ret;
//
//     ALLOC_KEY();
//     ret = bip32_key_unserialize(bytes, bytes_len, *output);
//     if (ret != WEB3_OK) {
//     	free(*output);
//         *output = NULL;
//     }
//     return ret;
// }

#ifdef BUILD_ELEMENTS
static int bip32_seckey_tweak_add(const unsigned char *tweak, size_t tweak_len,
                                  struct ext_key *key_out)
{
    if (!tweak || tweak_len != sizeof(key_out->pub_key_tweak_sum) || !key_out)
        return WEB3_EINVAL;

    if (!mem_is_zero(key_out->pub_key_tweak_sum, tweak_len))
        return seckey_tweak_add(key_out->pub_key_tweak_sum, tweak) ? WEB3_OK : WEB3_EINVAL;

    /* tweak sum is zero: start with the tweak */
    memcpy(key_out->pub_key_tweak_sum, tweak, tweak_len);
    return WEB3_OK;
}
#endif /* BUILD_ELEMENTS */

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
                          uint32_t flags, struct ext_key *key_out)
{
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

	// printf("key_out->priv_key %s\n",byteArrayToHexString(key_out->priv_key,sizeof(key_out->priv_key) + sizeof(key_out->child_num)));
	// printf("hdkey->chain_code %s\n",byteArrayToHexString(hdkey->chain_code,sizeof(hdkey->chain_code)));
 //    printf("hmac_sha512 %s\n",byteArrayToHexString(u8,sizeof(u8)));

    // hmac_sha512_impl(&sha, hdkey->chain_code, sizeof(hdkey->chain_code),
    //                  key_out->priv_key,
    //                  sizeof(key_out->priv_key) + sizeof(key_out->child_num));

    /* Split I into two 32-byte sequences, IL and IR
     * The returned chain code ci is IR (i.e. the 2nd half of our hmac sha512)
     */
    // memcpy(key_out->chain_code, sha.u.u8 + sizeof(sha) / 2,
    //        sizeof(key_out->chain_code));
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
#ifndef WALLY_ABI_NO_ELEMENTS
    memset(key_out->pub_key_tweak_sum, 0,
           sizeof(key_out->pub_key_tweak_sum));
#endif /* WALLY_ABI_NO_ELEMENTS */
#ifdef BUILD_ELEMENTS
    if (flags & BIP32_FLAG_KEY_TWEAK_SUM) {
        memcpy(key_out->pub_key_tweak_sum,
               hdkey->pub_key_tweak_sum, sizeof(hdkey->pub_key_tweak_sum));
        if (bip32_seckey_tweak_add(sha.u.u8, SHA256_LEN, key_out) != WEB3_OK)
            goto fail;
    }
#endif /* BUILD_ELEMENTS */

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
    if (flags & BIP32_FLAG_SKIP_HASH) {
	    memset(&key_out->parent160,0,sizeof(key_out->parent160));
    	memset(&key_out->hash160,0,sizeof(key_out->hash160));
    }
        // wally_clear_2(&key_out->parent160, sizeof(key_out->parent160),
        //               &key_out->hash160, sizeof(key_out->hash160));
    else {
        memcpy(key_out->parent160, hdkey->hash160, sizeof(hdkey->hash160));
        key_compute_hash160(key_out);
    }
	memset(u8, 0, sizeof(u8));
    return WEB3_OK;
fail:
	memset(u8, 0, sizeof(u8));
    return wipe_key_fail(key_out);
}

// int bip32_key_from_parent_alloc(const struct ext_key *hdkey,
//                                 uint32_t child_num, uint32_t flags,
//                                 struct ext_key **output)
// {
//     int ret;
//
//     ALLOC_KEY();
//     ret = bip32_key_from_parent(hdkey, child_num, flags, *output);
//     if (ret != WEB3_OK) {
//     	free(*output);
//         *output = NULL;
//     }
//     return ret;
// }
//
int bip32_key_from_parent_path(const struct ext_key *hdkey,
                               const uint32_t *child_path, size_t child_path_len,
                               uint32_t flags, struct ext_key *key_out)
{
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

        hdkey = derived;    /* Derived becomes next parent */
        tmp_idx = !tmp_idx; /* Use free slot in tmp for next derived */
    }

    if (ret == WEB3_OK)
        memcpy(key_out, hdkey, sizeof(*key_out));

	memset(tmp,0,sizeof(*tmp));
    return ret;
}
//
// int bip32_key_from_parent_path_alloc(const struct ext_key *hdkey,
//                                      const uint32_t *child_path, size_t child_path_len,
//                                      uint32_t flags,
//                                      struct ext_key **output)
// {
//     int ret;
//
//     ALLOC_KEY();
//     ret = bip32_key_from_parent_path(hdkey, child_path, child_path_len,
//                                      flags, *output);
//     if (ret != WEB3_OK) {
//     	free(*output);
//         *output = NULL;
//     }
//     return ret;
// }
//
// #ifndef WALLY_ABI_NO_ELEMENTS
// int bip32_key_with_tweak_from_parent_path(const struct ext_key *hdkey,
//                                           const uint32_t *child_path,
//                                           size_t child_path_len,
//                                           uint32_t flags,
//                                           struct ext_key *output)
// {
// #ifndef BUILD_ELEMENTS
//     return WEB3_ERROR;
// #else
//     const secp256k1_context *ctx;
//     secp256k1_pubkey pub_key;
//     size_t len = EC_PUBLIC_KEY_LEN;
//     int ret;
//
//     if (!(ctx = secp_ctx()))
//         return WEB3_ENOMEM;
//
//     if (!(flags & (BIP32_FLAG_KEY_TWEAK_SUM | BIP32_FLAG_KEY_PUBLIC)))
//         return WEB3_EINVAL;
//
//     if ((ret = bip32_key_from_parent_path(hdkey, child_path,
//                                           child_path_len, flags, output)) != WEB3_OK)
//         return ret;
//
//     if (!pubkey_parse(&pub_key, hdkey->pub_key, sizeof(hdkey->pub_key)) ||
//         !pubkey_tweak_add(ctx, &pub_key, output->pub_key_tweak_sum) ||
//         !pubkey_serialize(output->pub_key, &len, &pub_key, PUBKEY_COMPRESSED))
//         return wipe_key_fail(output);
//
//     return WEB3_OK;
// #endif /* BUILD_ELEMENTS */
// }
//
// int bip32_key_with_tweak_from_parent_path_alloc(const struct ext_key *hdkey,
//                                                 const uint32_t *child_path, size_t child_path_len,
//                                                 uint32_t flags,
//                                                 struct ext_key **output)
// {
// #ifndef BUILD_ELEMENTS
//     return WEB3_ERROR;
// #else
//     int ret;
//
//     ALLOC_KEY();
//     ret = bip32_key_with_tweak_from_parent_path(hdkey, child_path, child_path_len,
//                                                 flags, *output);
//     if (ret != WEB3_OK) {
//         free(*output);
//         *output = NULL;
//     }
//     return ret;
// #endif /* BUILD_ELEMENTS */
// }
// #endif /* WALLY_ABI_NO_ELEMENTS */
//
int bip32_key_from_parent_path_str_n(const struct ext_key *hdkey,
                                     const char *str, size_t str_len,
                                     uint32_t child_num, uint32_t flags,
                                     struct ext_key *key_out)
{
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
                                   struct ext_key *key_out)
{
    return bip32_key_from_parent_path_str_n(hdkey, str, str ? strlen(str) : 0,
                                            child_num, flags, key_out);
}
//
// int bip32_key_from_parent_path_str_n_alloc(const struct ext_key *hdkey,
//                                            const char *str, size_t str_len,
//                                            uint32_t child_num, uint32_t flags,
//                                            struct ext_key **output)
// {
//     int ret;
//
//     ALLOC_KEY();
//     ret = bip32_key_from_parent_path_str_n(hdkey, str, str_len, child_num, flags, *output);
//     if (ret != WEB3_OK) {
//         free(*output);
//         *output = NULL;
//     }
//     return ret;
// }
//
// int bip32_key_from_parent_path_str_alloc(const struct ext_key *hdkey,
//                                          const char *str,
//                                          uint32_t child_num, uint32_t flags,
//                                          struct ext_key **output)
// {
//     return bip32_key_from_parent_path_str_n_alloc(hdkey, str, str ? strlen(str) : 0,
//                                                   child_num, flags, output);
// }
//
//
// int bip32_key_init_alloc(uint32_t version, uint32_t depth, uint32_t child_num,
//                          const unsigned char *chain_code, size_t chain_code_len,
//                          const unsigned char *pub_key, size_t pub_key_len,
//                          const unsigned char *priv_key, size_t priv_key_len,
//                          const unsigned char *hash160, size_t hash160_len,
//                          const unsigned char *parent160, size_t parent160_len,
//                          struct ext_key **output)
// {
//     int ret;
//
//     ALLOC_KEY();
//     ret = bip32_key_init(version, depth, child_num, chain_code, chain_code_len,
//                          pub_key, pub_key_len, priv_key, priv_key_len,
//                          hash160, hash160_len, parent160, parent160_len, *output);
//     if (ret != WEB3_OK) {
//         wally_free((void *)*output);
//         *output = NULL;
//     }
//     return ret;
// }
//
// int bip32_key_init(uint32_t version, uint32_t depth, uint32_t child_num,
//                    const unsigned char *chain_code, size_t chain_code_len,
//                    const unsigned char *pub_key, size_t pub_key_len,
//                    const unsigned char *priv_key, size_t priv_key_len,
//                    const unsigned char *hash160, size_t hash160_len,
//                    const unsigned char *parent160, size_t parent160_len,
//                    struct ext_key *key_out)
// {
//     if (!key_out)
//         return WEB3_EINVAL;
//
//     switch (version) {
//     case BIP32_VER_MAIN_PRIVATE:
//     case BIP32_VER_TEST_PRIVATE:
//         if (!priv_key || priv_key_len != key_size(priv_key) - 1)
//             return WEB3_EINVAL;
//         break;
//     case BIP32_VER_MAIN_PUBLIC:
//     case BIP32_VER_TEST_PUBLIC:
//         if (!pub_key || pub_key_len != key_size(pub_key))
//             return WEB3_EINVAL;
//         break;
//     }
//
//     if (!chain_code || chain_code_len != key_size(chain_code))
//         return WEB3_EINVAL;
//
//     if ((priv_key && priv_key_len != key_size(priv_key) - 1) || (!priv_key && priv_key_len) ||
//         (pub_key && pub_key_len != key_size(pub_key)) || (!pub_key && pub_key_len) ||
//         (hash160 && hash160_len != key_size(hash160)) || (!hash160 && hash160_len) ||
//         (parent160 && !is_valid_parent160_len(parent160_len)) ||
//         (!parent160 && parent160_len) || depth > 0xff)
//         return WEB3_EINVAL;
//
//     wally_clear(key_out, sizeof(*key_out));
//     key_out->version = version;
//     key_out->depth = depth;
//     key_out->child_num = child_num;
//
//     memcpy(key_out->chain_code, chain_code, key_size(chain_code));
//     if (priv_key && version != BIP32_VER_MAIN_PUBLIC && version != BIP32_VER_TEST_PUBLIC)
//         memcpy(key_out->priv_key + 1, priv_key, key_size(priv_key) - 1);
//     else
//         key_out->priv_key[0] = BIP32_FLAG_KEY_PUBLIC;
//     if (pub_key)
//         memcpy(key_out->pub_key, pub_key, key_size(pub_key));
//     else if (version == BIP32_VER_MAIN_PRIVATE || version == BIP32_VER_TEST_PRIVATE) {
//         /* Compute the public key if not given */
//         int ret = key_compute_pub_key(key_out);
//         if (ret != WEB3_OK) {
//             wally_clear(key_out, sizeof(*key_out));
//             return ret;
//         }
//     }
//     if (hash160)
//         memcpy(key_out->hash160, hash160, key_size(hash160));
//     else
//         key_compute_hash160(key_out);
//     if (parent160)
//         memcpy(key_out->parent160, parent160, parent160_len);
//
//     return WEB3_OK;
// }
//
// int bip32_key_to_base58(const struct ext_key *hdkey,
//                         uint32_t flags,
//                         char **output)
// {
//     int ret;
//     unsigned char bytes[BIP32_SERIALIZED_LEN];
//
//     if ((ret = bip32_key_serialize(hdkey, flags, bytes, sizeof(bytes))))
//         return ret;
//
//     ret = wally_base58_from_bytes(bytes, BIP32_SERIALIZED_LEN, BASE58_FLAG_CHECKSUM, output);
//
//     wally_clear(bytes, sizeof(bytes));
//     return ret;
// }
//
// int bip32_key_from_base58_n(const char *base58, size_t base58_len,
//                             struct ext_key *output)
// {
//     int ret;
//     unsigned char bytes[BIP32_SERIALIZED_LEN + BASE58_CHECKSUM_LEN];
//     size_t written;
//
//     if ((ret = wally_base58_n_to_bytes(base58, base58_len, BASE58_FLAG_CHECKSUM,
//                                        bytes, sizeof(bytes), &written)))
//         return ret;
//
//     if (written != BIP32_SERIALIZED_LEN)
//         ret = WEB3_EINVAL;
//     else
//         ret = bip32_key_unserialize(bytes, BIP32_SERIALIZED_LEN, output);
//
//     wally_clear(bytes, sizeof(bytes));
//     return ret;
// }
//
// int bip32_key_from_base58(const char *base58,
//                           struct ext_key *output)
// {
//     return bip32_key_from_base58_n(base58, base58 ? strlen(base58) : 0, output);
// }
//
// int bip32_key_from_base58_n_alloc(const char *base58, size_t base58_len,
//                                   struct ext_key **output)
// {
//     int ret;
//
//     ALLOC_KEY();
//     ret = bip32_key_from_base58_n(base58, base58_len, *output);
//     if (ret != WEB3_OK) {
//         free(*output);
//         *output = NULL;
//     }
//     return ret;
// }
//
// int bip32_key_from_base58_alloc(const char *base58,
//                                 struct ext_key **output)
// {
//     return bip32_key_from_base58_n_alloc(base58, base58 ? strlen(base58) : 0, output);
// }
//
int bip32_key_strip_private_key(struct ext_key *key_out)
{
    if (!key_out)
        return WEB3_EINVAL;
    key_out->priv_key[0] = BIP32_FLAG_KEY_PUBLIC;
    //wally_clear(key_out->priv_key + 1, sizeof(key_out->priv_key) - 1);
    return WEB3_OK;
}
//
// int bip32_key_get_fingerprint(struct ext_key *hdkey,
//                               unsigned char *bytes_out, size_t len)
// {
//     /* Validate our arguments and then the input key */
//     if (!hdkey ||
//         !key_is_valid(hdkey) ||
//         !bytes_out || len != BIP32_KEY_FINGERPRINT_LEN)
//         return WEB3_EINVAL;
//
//     /* Derive hash160 if needed. */
//     if (mem_is_zero(hdkey->hash160, sizeof(hdkey->hash160))) {
//         key_compute_hash160(hdkey);
//     }
//
//     /* Fingerprint is first 32 bits of the key hash. */
//     memcpy(bytes_out, hdkey->hash160, BIP32_KEY_FINGERPRINT_LEN);
//     return WEB3_OK;
// }


#if defined (SWIG_JAVA_BUILD) || defined (SWIG_PYTHON_BUILD) || defined (SWIG_JAVASCRIPT_BUILD) || defined(WASM_BUILD)

/* Getters for ext_key values */

static int getb_impl(const struct ext_key *hdkey,
                     const unsigned char *src, size_t src_len,
                     unsigned char *bytes_out, size_t len)
{
    if (!hdkey || !bytes_out || len != src_len)
        return WEB3_EINVAL;
    memcpy(bytes_out, src, len);
    return WEB3_OK;
}

#define GET_B(name) \
    int bip32_key_get_ ## name(const struct ext_key *hdkey, unsigned char *bytes_out, size_t len) { \
        return getb_impl(hdkey, hdkey->name, sizeof(hdkey->name), bytes_out, len); \
    }

GET_B(chain_code)
GET_B(parent160)
GET_B(hash160)
GET_B(pub_key)
#ifndef WALLY_ABI_NO_ELEMENTS
#ifndef BUILD_ELEMENTS
int bip32_key_get_pub_key_tweak_sum(const struct ext_key *hdkey, unsigned char *bytes_out, size_t len) {
    return WEB3_ERROR;
}
#else
GET_B(pub_key_tweak_sum)
#endif /* BUILD_ELEMENTS */
#endif /* WALLY_ABI_NO_ELEMENTS */

int bip32_key_get_priv_key(const struct ext_key *hdkey, unsigned char *bytes_out, size_t len) {
    return getb_impl(hdkey, hdkey->priv_key + 1, sizeof(hdkey->priv_key) - 1, bytes_out, len);
}


#define GET_I(name) \
    int bip32_key_get_ ## name(const struct ext_key *hdkey, size_t *written) { \
        if (written) *written = 0; \
        if (!hdkey || !written) return WEB3_EINVAL; \
        *written = hdkey->name; \
        return WEB3_OK; \
    }

GET_I(depth)
GET_I(child_num)
GET_I(version)

#endif /* SWIG_JAVA_BUILD/SWIG_PYTHON_BUILD/SWIG_JAVASCRIPT_BUILD/WASM_BUILD */
