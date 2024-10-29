#include "account_utils.h"

static char *m_strdup_n(const char *str, size_t str_len) {
	char *new_str = (char *)malloc(str_len + 1);
	if (new_str) {
		if (str_len) {
			memcpy(new_str, str, str_len);
		}
		new_str[str_len] = '\0';
	}
	return new_str;
}

char *m_strdup(const char *str) {
	return m_strdup_n(str, strlen(str));
}
