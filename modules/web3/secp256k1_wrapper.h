#ifndef SECP256K1_WRAPPER_H
#define SECP256K1_WRAPPER_H

#include <iostream>
#include <vector>
#include <iomanip>
#include <fstream>
#include <sstream>

// compiler options
#ifdef __SIZEOF_INT128__
#  define HAVE___INT128
#  define USE_FIELD_5X52
#  define USE_SCALAR_4X64
#else
#  define USE_FIELD_10X26
#  define USE_SCALAR_8X32
#endif

#ifndef NDEBUG
#  define NDEBUG
#endif

#define USE_ENDOMORPHISM
#define USE_NUM_NONE
#define USE_FIELD_INV_BUILTIN
#define USE_SCALAR_INV_BUILTIN

extern "C" {
    #include "secp256k1.h"
	#include "secp256k1_recovery.h"
	#include "scalar.h"
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
#include "core/variant/variant.h"
#include "core/error/error_macros.h"
#include "core/error/error_list.h"

// must call initialize() after create a Secp256k1Wrapper instance, otherwise some function will panic
class Secp256k1Wrapper : public RefCounted {
	GDCLASS(Secp256k1Wrapper, RefCounted);

protected:
	static void _bind_methods();

public:
    Secp256k1Wrapper();
    ~Secp256k1Wrapper();

	bool initialize();

	bool set_secret_key(const String &key);
	PackedByteArray get_secret_key() const;
	Error save_secret_key(const String &path);

	bool set_public_key(const String &key);
	PackedByteArray  get_public_key() const;
	Error save_public_key(const String &path);
	PackedByteArray recover_pubkey(const PackedByteArray &message, const PackedByteArray &signature);

	bool generate_key_pair();
	bool compute_public_key_from_seckey();
	PackedByteArray sign(const PackedByteArray &message);
	bool verify(const PackedByteArray &message, const PackedByteArray &signature);

private:
    secp256k1_context* m_ctx;
    PackedByteArray m_secret_key;
    PackedByteArray m_public_key;
	static int fill_random(unsigned char* data, size_t size);
};

#endif // SECP256K1_WRAPPER_H
