#ifndef ETH_HMAC_H
#define ETH_HMAC_H

#include <stddef.h>

void hmac_sha512(const unsigned char *key, size_t key_len,
		const unsigned char *msg, size_t msg_len,
		unsigned char *output);
#endif // ETH_HMAC_H