#ifndef SECP256K1_WRAPPER_H
#define SECP256K1_WRAPPER_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>

extern "C" {
    #include "secp256k1.h"
    #include "examples_util.h"
}

extern "C" {
    #if defined(_WIN32)
    /*
    * The defined WIN32_NO_STATUS macro disables return code definitions in
    * windows.h, which avoids "macro redefinition" MSVC warnings in ntstatus.h.
    */
    #define WIN32_NO_STATUS
    #include <windows.h>
    #undef WIN32_NO_STATUS
    #include <ntstatus.h>
    #include <bcrypt.h>
    #elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__)
    #include <sys/random.h>
    #elif defined(__OpenBSD__)
    #include <unistd.h>
    #else
    #error "Couldn't identify the OS"
    #endif
}

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"

class Secp256k1Wrapper : public RefCounted {
	GDCLASS(Secp256k1Wrapper, RefCounted);

protected:
	static void _bind_methods();

public:
    Secp256k1Wrapper();
    ~Secp256k1Wrapper();

	bool initialize();

	bool set_secret_key(const String &key);
	Array get_secret_key() const;
	bool save_secret_key(const String &path);
	void print_secret_key();

	bool set_public_key(const String &key);
	secp256k1_pubkey  get_public_key() const;
	bool save_public_key(const String &path);
	void print_public_key();

	bool generate_key_pair();
	bool compute_public_key_from_seckey();
	bool sign(const std::vector<unsigned char>& message, secp256k1_ecdsa_signature& signature);
	bool verify(const std::vector<unsigned char>& message, const secp256k1_ecdsa_signature& signature);

private:
    secp256k1_context* ctx;
    Array secret_key;
    secp256k1_pubkey public_key;
	static int fill_random(unsigned char* data, size_t size);
};

#endif // SECP256K1_WRAPPER_H
