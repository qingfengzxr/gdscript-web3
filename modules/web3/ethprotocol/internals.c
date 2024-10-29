#include <stdio.h>

#include "KeccakHash.h"
#include "internals.h"

#if defined(__unix__) || (defined(__APPLE__) && defined(__MACH__))
#include <strings.h>
#elif defined(_MSC_VER)
#define strncasecmp _strnicmp
#endif

int ethc_rand(uint8_t *bytes, size_t len) {
	FILE *fd;
	int r;

	if (bytes == NULL)
		return -1;

	fd = fopen("/dev/urandom", "r");
	if (fd == NULL)
		return -1;

	r = fread(bytes, 1, len, fd);
	if (r <= 0)
		return -1;

	fclose(fd);
	return 1;
}

int ethc_strncasecmp(const char *s1, const char *s2, size_t len) {
	return strncasecmp(s1, s2, len);
}

int ethc_hexcharb(char ch) {
	if (ch >= '0' && ch <= '9')
		return ch - '0';
	else if (ch >= 'a' && ch <= 'f')
		return ch - 'a' + 10;
	else if (ch >= 'A' && ch <= 'F')
		return ch - 'A' + 10;
	else
		return 0;
}

char ethc_hexchar(uint8_t d) {
	const char *hexchars = "0123456789abcdef";
	return hexchars[d];
}

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#else
#include <time.h>
#endif

#define KECCAK256_LEN 32

#if defined(_WIN32) || defined(_WIN64)
void get_high_precision_time(double *seconds) {
	LARGE_INTEGER frequency, counter;
	QueryPerformanceFrequency(&frequency);
	QueryPerformanceCounter(&counter);
	*seconds = (double)counter.QuadPart / frequency.QuadPart;
}
#else
void get_high_precision_time(double *seconds) {
	struct timespec ts;
	if (clock_gettime(CLOCK_MONOTONIC, &ts) == -1) {
		perror("clock_gettime");
		exit(EXIT_FAILURE);
	}
	*seconds = ts.tv_sec + ts.tv_nsec / 1e9;
}
#endif

void secure_zero(void *v, size_t n) {
	volatile unsigned char *p = v;
	while (n--)
		*p++ = 0;
}

void keccak256(const unsigned char *input, size_t len, unsigned char output[KECCAK256_LEN]) {
	Keccak_HashInstance ctx;
	Keccak_HashInitialize_SHA3_256(&ctx);
	Keccak_HashUpdate(&ctx, input, len * 8); // Length in bits
	Keccak_HashFinal(&ctx, output);
}

void get_random_bytes(size_t num_bytes, void *output_bytes, size_t output_size) {
	static unsigned char curr_state[KECCAK256_LEN] = { 0 };
	static uint64_t nonce = 0;

	if (num_bytes > KECCAK256_LEN || num_bytes > output_size) {
		fprintf(stderr, "Requested too many random bytes\n");
		exit(EXIT_FAILURE);
	}

	double tsc;
	get_high_precision_time(&tsc);
	unsigned char buf[KECCAK256_LEN + KECCAK256_LEN + KECCAK256_LEN + 8]; // 32 bytes from various sources + 8 bytes for nonce
	printf("tsc = %f \n", tsc);

	// Add high precision time as entropy
	memcpy(buf, &tsc, sizeof(tsc));
	secure_zero(&tsc, sizeof(tsc)); // Zero out the timestamp

// Add OS random source entropy
#if defined(_WIN32) || defined(_WIN64)
	// Windows uses CryptGenRandom for random data
	HCRYPTPROV hProvider;
	if (!CryptAcquireContext(&hProvider, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT) ||
			!CryptGenRandom(hProvider, KECCAK256_LEN, buf + sizeof(tsc))) {
		fprintf(stderr, "Failed to get random bytes\n");
		exit(EXIT_FAILURE);
	}
	CryptReleaseContext(hProvider, 0);
#else
	FILE *fp = fopen("/dev/urandom", "rb");
	if (fp == NULL || fread(buf + sizeof(tsc), 1, KECCAK256_LEN, fp) != KECCAK256_LEN) {
		fprintf(stderr, "Failed to read from /dev/urandom: %s\n", strerror(errno));
		if (fp != NULL) {
			fclose(fp);
		}
		exit(EXIT_FAILURE);
	}
	fclose(fp);
#endif

	// Add current state and nonce
	memcpy(buf + KECCAK256_LEN + sizeof(tsc), curr_state, KECCAK256_LEN);
	memcpy(buf + 2 * KECCAK256_LEN + sizeof(tsc), &nonce, sizeof(nonce));
	nonce++;

	// Compute Keccak (SHA-3) 256 hash
	unsigned char hashed[KECCAK256_LEN];
	keccak256(buf, sizeof(buf), hashed);

	// Update the current state with the hash
	memcpy(curr_state, hashed, KECCAK256_LEN);

	// Copy the first `num_bytes` of the hash to output
	memcpy(output_bytes, hashed, num_bytes);

	// Securely clear sensitive data
	secure_zero(buf, sizeof(buf));
	secure_zero(hashed, sizeof(hashed));
}
