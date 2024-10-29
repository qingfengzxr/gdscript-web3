#ifndef ETHC_COMMON_H
#define ETHC_COMMON_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define ETHC_EXPORT __declspec(dllexport)
#elif __GNUC__
#define ETHC_EXPORT __attribute__((visibility("default")))
#else
#define ETHC_EXPORT
#endif

#define EC_ADDRESS_KEY_LEN 20

/** The length of a private key used for EC signing */
#define EC_PRIVATE_KEY_LEN 32
/** The length of a public key used for EC signing */
#define EC_PUBLIC_KEY_LEN 33
/** The length of an x-only public key used for EC signing */
#define EC_XONLY_PUBLIC_KEY_LEN 32
/** The length of an uncompressed public key */
#define EC_PUBLIC_KEY_UNCOMPRESSED_LEN 65
/** The length of a message hash to EC sign */
#define EC_MESSAGE_HASH_LEN 32
/** The length of a compact signature produced by EC signing */
#define EC_SIGNATURE_LEN 64
/** The length of a compact recoverable signature produced by EC signing */
#define EC_SIGNATURE_RECOVERABLE_LEN 65
/** The maximum encoded length of a DER signature (High-R, High-S), excluding sighash byte */
#define EC_SIGNATURE_DER_MAX_LEN 72
/** The maximum encoded length of a DER signature created with `EC_FLAG_GRIND_R` (Low-R, Low-S), excluding sighash byte */
#define EC_SIGNATURE_DER_MAX_LOW_R_LEN 70
/** The length of a secp256k1 scalar value */
#define EC_SCALAR_LEN 32

#ifdef __cplusplus
}
#endif

#endif /* ACCOUNT_UTILS_H */
