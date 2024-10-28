#include "bip39.h"

#include "thirdparty/mbedtls/include/mbedtls/pkcs5.h"
#include "thirdparty/mbedtls/include/mbedtls/sha256.h"

#include "mnemonic.h"
#include "wordlist.h"
#include "account_utils.h"

#include "../wordlists/english.c"

#define BUILD_MINIMAL 1

#ifndef BUILD_MINIMAL
#include "wordlists/chinese_simplified.c"
#include "wordlists/chinese_traditional.c"
#include "wordlists/french.c"
#include "wordlists/italian.c"
#include "wordlists/japanese.c"
#include "wordlists/spanish.c"
#endif

/** Valid entropy lengths */
#define BIP39_ENTROPY_LEN_128 16
#define BIP39_ENTROPY_LEN_160 20
#define BIP39_ENTROPY_LEN_192 24
#define BIP39_ENTROPY_LEN_224 28
#define BIP39_ENTROPY_LEN_256 32
#define BIP39_ENTROPY_LEN_288 36
#define BIP39_ENTROPY_LEN_320 40

/** The required size of the output buffer for `bip39_mnemonic_to_seed` */
#define BIP39_SEED_LEN_512 64

/** Maximum entropy size (including up to 2 bytes for checksum) */
#define BIP39_ENTROPY_MAX_LEN 42

/** The number of words in a BIP39 compliant wordlist */
#define BIP39_WORDLIST_LEN 2048

static const struct {
	const char name[4];
	const struct words *words;
} lookup[] = {
	{ "en", &en_words },
#ifndef BUILD_MINIMAL
	{ "es", &es_words },
	{ "fr", &fr_words },
	{ "it", &it_words },
	{ "jp", &jp_words },
	{ "zhs", &zhs_words },
	{ "zht", &zht_words },
/* FIXME: Should 'zh' map to traditional or simplified? */
#endif
};

/*
int bip39_get_languages(char **output)
{
	if (!output)
		return WALLY_EINVAL;
#ifndef BUILD_MINIMAL
	*output = wally_strdup("en es fr it jp zhs zht");
#else
	*output = wally_strdup("en");
#endif
	return *output ? WALLY_OK : WALLY_ENOMEM;
}*/

int bip39_get_wordlist(const char *lang, struct words **output) {
	size_t i;

	if (!output) {
		return 1;
	}

	*output = (struct words *)&en_words; /* Fallback to English if not found */

	if (lang) {
		for (i = 0; i < sizeof(lookup) / sizeof(lookup[0]); ++i) {
			if (!strcmp(lang, lookup[i].name)) {
				*output = (struct words *)lookup[i].words;
				break;
			}
		}
	}
	return 0;
}

const char *bip39_get_word_by_index(const struct words *w, size_t idx) {
	return wordlist_lookup_index(w ? w : &en_words, idx);
}

int bip39_get_word(const struct words *w, size_t idx,
		char **output) {
	const char *word;

	if (output) {
		*output = NULL;
	}

	w = w ? w : &en_words;

	if (!output || !(word = wordlist_lookup_index(w, idx))) {
		return 1;
	}

	*output = m_strdup(word);
	return *output ? 0 : 2;
}

/* Convert an input entropy length to a mask for checksum bits. As it
 * returns 0 for bad lengths, it serves as a validation function too.
 */
static size_t len_to_mask(size_t len) {
	switch (len) {
		case BIP39_ENTROPY_LEN_128:
			return 0xf0;
		case BIP39_ENTROPY_LEN_160:
			return 0xf8;
		case BIP39_ENTROPY_LEN_192:
			return 0xfc;
		case BIP39_ENTROPY_LEN_224:
			return 0xfe;
		case BIP39_ENTROPY_LEN_256:
			return 0xff;
		case BIP39_ENTROPY_LEN_288:
			return 0x80ff;
		case BIP39_ENTROPY_LEN_320:
			return 0xC0ff;
	}
	return 0;
}

size_t bip39_checksum(const unsigned char *bytes, size_t bytes_len, size_t mask) {
	unsigned char sha_output[32]; // mbedtls_sha256 output is always 32 bytes
	size_t ret;

	// Compute SHA-256 hash of the input bytes
	mbedtls_sha256(bytes, bytes_len, sha_output, 0); // 0 for SHA-256 (not SHA-224)

	// Combine the first two bytes into a 16-bit integer
	ret = sha_output[0] | (sha_output[1] << 8);

	// Return the result masked with the input mask
	return ret & mask;
}

int bip39_mnemonic_from_bytes(const struct words *w,
		const unsigned char *bytes, size_t bytes_len,
		char **output) {
	unsigned char tmp_bytes[BIP39_ENTROPY_MAX_LEN];
	size_t checksum, mask;

	if (output) {
		*output = NULL;
	}

	if (!bytes || !bytes_len || !output) {
		return 1;
	}

	w = w ? w : &en_words;

	if (w->bits != 11u || !(mask = len_to_mask(bytes_len))) {
		return 1;
	}

	memcpy(tmp_bytes, bytes, bytes_len);
	checksum = bip39_checksum(bytes, bytes_len, mask);
	tmp_bytes[bytes_len] = checksum & 0xff;
	if (mask > 0xff) {
		tmp_bytes[++bytes_len] = (checksum >> 8) & 0xff;
	}
	*output = mnemonic_from_bytes(w, tmp_bytes, bytes_len + 1);
	memset(tmp_bytes, 0, sizeof(tmp_bytes));
	return *output ? 0 : 2;
}

static bool checksum_ok(const unsigned char *bytes, size_t idx, size_t mask) {
	/* The checksum is stored after the data to sum */
	size_t calculated = bip39_checksum(bytes, idx, mask);
	size_t stored = bytes[idx];
	if (mask > 0xff) {
		stored |= (bytes[idx + 1] << 8);
	}
	return (stored & mask) == calculated;
}

int bip39_mnemonic_to_bytes(const struct words *w, const char *mnemonic,
		unsigned char *bytes_out, size_t len,
		size_t *written) {
	unsigned char tmp_bytes[BIP39_ENTROPY_MAX_LEN];
	size_t mask, tmp_len;
	int ret;

	/* Ideally we would infer the wordlist here. Unfortunately this cannot
	 * work reliably because the default word lists overlap. In combination
	 * with being sorted lexographically, this means the default lists
	 * were poorly chosen. But we are stuck with them now.
	 *
	 * If the caller doesn't know which word list to use, they should iterate
	 * over the available ones and try any resulting list that the mnemonic
	 * validates against.
	 */
	w = w ? w : &en_words;

	if (written) {
		*written = 0;
	}

	if (w->bits != 11u || !mnemonic || !bytes_out) {
		return 1;
	}

	ret = mnemonic_to_bytes(w, mnemonic, tmp_bytes, sizeof(tmp_bytes), &tmp_len);

	if (!ret) {
		/* Remove checksum bytes from the output length */
		--tmp_len;
		if (tmp_len > BIP39_ENTROPY_LEN_256) {
			--tmp_len; /* Second byte required */
		}

		if (tmp_len > sizeof(tmp_bytes)) {
			ret = 1; /* Too big for biggest supported entropy */
		} else {
			if (tmp_len <= len) {
				if (!(mask = len_to_mask(tmp_len)) ||
						!checksum_ok(tmp_bytes, tmp_len, mask)) {
					tmp_len = 0;
					ret = 1; /* Bad checksum */
				} else {
					memcpy(bytes_out, tmp_bytes, tmp_len);
				}
			}
		}
	}

	memset(tmp_bytes, 0, sizeof(tmp_bytes));
	if (!ret && written) {
		*written = tmp_len;
	}
	return ret;
}

int bip39_mnemonic_validate(const struct words *w, const char *mnemonic) {
	unsigned char buf[BIP39_ENTROPY_MAX_LEN];
	size_t len;
	int ret = bip39_mnemonic_to_bytes(w, mnemonic, buf, sizeof(buf), &len);
	memset(buf, 0, sizeof(buf));
	return ret;
}

int bip39_mnemonic_to_seed(const char *mnemonic, const char *passphrase,
		unsigned char *bytes_out, size_t len,
		size_t *written) {
	const uint32_t bip9_cost = 2048u;
	const char *prefix = "mnemonic";
	const size_t prefix_len = strlen(prefix);
	const size_t passphrase_len = passphrase ? strlen(passphrase) : 0;
	const size_t salt_len = prefix_len + passphrase_len;
	unsigned char *salt;
	int ret;

	if (written) {
		*written = 0;
	}

	if (!mnemonic || !bytes_out || len != BIP39_SEED_LEN_512) {
		return 1;
	}

	salt = malloc(salt_len);

	if (!salt) {
		return 2;
	}

	memcpy(salt, prefix, prefix_len);
	if (passphrase_len) {
		memcpy(salt + prefix_len, passphrase, passphrase_len);
	}

	/*ret = wally_pbkdf2_hmac_sha512((unsigned char *)mnemonic, strlen(mnemonic),
								   salt, salt_len, 0,
								   bip9_cost, bytes_out, len);*/
	ret = mbedtls_pkcs5_pbkdf2_hmac_ext(MBEDTLS_MD_SHA512, (unsigned char *)mnemonic, strlen(mnemonic), salt, salt_len, bip9_cost, len, bytes_out);

	if (!ret && written) {
		*written = BIP39_SEED_LEN_512; /* Succeeded */
	}

	memset(salt, 0, salt_len);
	free(salt);

	return ret;
}

int bip39_mnemonic_to_seed512(const char *mnemonic, const char *passphrase,
		unsigned char *bytes_out, size_t len) {
	size_t written;
	return bip39_mnemonic_to_seed(mnemonic, passphrase, bytes_out, len, &written);
}
