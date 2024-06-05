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

	mpz_t m_number;

protected:
	static void _bind_methods();

public:
	BigInt();
	~BigInt();

	void test_add();
	// TODO: need to support return BigInt object
};

#endif // BIG_INT_H
