#ifndef ETHC_INTERNALS_H
#define ETHC_INTERNALS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ethc-common.h"
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>


ETHC_EXPORT int ethc_rand(uint8_t *bytes, size_t len);
ETHC_EXPORT int ethc_strncasecmp(const char *s1, const char *s2, size_t len);
ETHC_EXPORT int ethc_hexcharb(char h);
ETHC_EXPORT char ethc_hexchar(uint8_t d);
ETHC_EXPORT void get_random_bytes(size_t num_bytes, void* output_bytes, size_t output_size);
#ifdef __cplusplus
}
#endif

#endif
