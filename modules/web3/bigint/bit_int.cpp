#include "big_int.h"


BigInt::BigInt(){
	mpz_init(m_number);
}

BigInt::~BigInt() {
	mpz_clear(m_number);
}

void BigInt::test_add() {
    mpz_t m_a, m_b, sum;
    char *sum_str;

    mpz_init_set_str(m_a, "1234567890123456789012345678901234567890", 10);
    mpz_init_set_str(m_b, "9876543210987654321098765432109876543210", 10);
    mpz_init(sum);

    mpz_add(sum, m_a, m_b);

    sum_str = mpz_get_str(NULL, 10, sum);
    std::cout << "=====> Sum: " << sum_str << "\n";

    free(sum_str);
}

void BigInt::_bind_methods() {
	ClassDB::bind_method(D_METHOD("test_add"), &BigInt::test_add);
}
