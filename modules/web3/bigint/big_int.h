#ifndef BIG_INT_H
#define BIG_INT_H

#include <gmp.h>
#include <iostream>

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "core/error/error_macros.h"
#include "core/error/error_list.h"

class BigInt : public RefCounted {
    GDCLASS(BigInt, RefCounted);

protected:
    static void _bind_methods();

public:
    mpz_t m_number;

    BigInt();
    ~BigInt();

	void set_bytes(uint8_t* bytes, size_t size);
    void from_string(String str);
    String get_string();
	bool from_hex(const String &hex_string);
	String to_hex();
	int to_int() const;

    Ref<BigInt> add(const Ref<BigInt> other);
    Ref<BigInt> sub(const Ref<BigInt> other);
    Ref<BigInt> mul(const Ref<BigInt> other);
    Ref<BigInt> div(const Ref<BigInt> other);
    Ref<BigInt> mod(const Ref<BigInt> other);
    Ref<BigInt> abs();

	// return int, meaning:
	// < 0: this < other
	// > 0: this > other
	// = 0: this = other
	int cmp(const Ref<BigInt> other);
	// Returns the sign of m_number as an integer:
	// -1 if m_number is less than 0,
	// 0 if m_number is equal to 0,
	// 1 if m_number is greater than 0.
	int sgn();
	bool is_zero() const;
};

#endif // BIG_INT_H
