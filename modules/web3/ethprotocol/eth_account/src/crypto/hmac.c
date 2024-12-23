#include "hmac.h"

#include <thirdparty/mbedtls/include/mbedtls/pkcs5.h>
#include <thirdparty/mbedtls/include/mbedtls/sha256.h>
#include <thirdparty/mbedtls/include/mbedtls/sha512.h>

void hmac_sha512(const unsigned char *key, size_t key_len,
		const unsigned char *msg, size_t msg_len,
		unsigned char *output) {
	const mbedtls_md_info_t *md_info;
	mbedtls_md_context_t ctx;

	mbedtls_md_init(&ctx);

	md_info = mbedtls_md_info_from_type(MBEDTLS_MD_SHA512);

	if (md_info == NULL) {
		printf("SHA512 is not supported.\n");
		return;
	}

	mbedtls_md_setup(&ctx, md_info, 1);
	mbedtls_md_hmac_starts(&ctx, key, key_len);

	mbedtls_md_hmac_update(&ctx, msg, msg_len);

	mbedtls_md_hmac_finish(&ctx, output);

	mbedtls_md_free(&ctx);
}