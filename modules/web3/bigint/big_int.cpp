#include "big_int.h"


BigInt::BigInt(){
	mpz_init(m_number);
}

BigInt::~BigInt() {
	mpz_clear(m_number);
}

void BigInt::from_string(String str) {
	mpz_init_set_str(m_number, str.utf8().get_data(), 10);
}

String BigInt::get_string() {
	char *str = mpz_get_str(NULL, 10, m_number);
	String result = String(str);
	free(str);
	return result;
}

Ref<BigInt> BigInt::add(const Ref<BigInt> other) {
	Ref<BigInt> result = Ref<BigInt>(memnew(BigInt));
	mpz_add(result->m_number, this->m_number, other->m_number);
	return result;
}

Ref<BigInt> BigInt::sub(const Ref<BigInt> other) {
    Ref<BigInt> result = Ref<BigInt>(memnew(BigInt));
    mpz_sub(result->m_number, this->m_number, other->m_number);
    return result;
}

Ref<BigInt> BigInt::mul(const Ref<BigInt> other) {
    Ref<BigInt> result = Ref<BigInt>(memnew(BigInt));
    mpz_mul(result->m_number, this->m_number, other->m_number);
    return result;
}

Ref<BigInt> BigInt::div(const Ref<BigInt> other) {
    Ref<BigInt> result = Ref<BigInt>(memnew(BigInt));
	mpz_tdiv_q(result->m_number, this->m_number, other->m_number);
    return result;
}

Ref<BigInt> BigInt::mod(const Ref<BigInt> other) {
    Ref<BigInt> result = Ref<BigInt>(memnew(BigInt));
    mpz_mod(result->m_number, this->m_number, other->m_number);
    return result;
}

int BigInt::cmp(const Ref<BigInt> other) {
    return mpz_cmp(this->m_number, other->m_number);
}

void BigInt::_bind_methods() {
	ClassDB::bind_method(D_METHOD("from_string"), &BigInt::from_string);
	ClassDB::bind_method(D_METHOD("get_string"), &BigInt::get_string);
	ClassDB::bind_method(D_METHOD("add"), &BigInt::add);
	ClassDB::bind_method(D_METHOD("sub"), &BigInt::sub);
	ClassDB::bind_method(D_METHOD("mul"), &BigInt::mul);
	ClassDB::bind_method(D_METHOD("div"), &BigInt::div);
	ClassDB::bind_method(D_METHOD("mod"), &BigInt::mod);
	ClassDB::bind_method(D_METHOD("cmp"), &BigInt::cmp);
}
