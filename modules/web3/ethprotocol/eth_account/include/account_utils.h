
#ifndef ACCOUNT_UTILS_H
#define ACCOUNT_UTILS_H

#include <stdlib.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Return codes */
#define WEB3_OK 0 /** Success */
#define WEB3_ERROR -1 /** General error */
#define WEB3_EINVAL -2 /** Invalid argument */
#define WEB3_ENOMEM -3 /** malloc() failed */

char *m_strdup(const char *str);

#ifdef __cplusplus
}
#endif
#endif