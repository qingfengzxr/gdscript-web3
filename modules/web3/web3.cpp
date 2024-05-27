#include "web3.h"


Ref<Optimism> Web3::get_op_instance() {
	return m_op;
}

void Web3::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_op_instance"), &Web3::get_op_instance);
}

Web3::Web3() {
    m_op = Ref<Optimism>(memnew(Optimism));
}

Web3::~Web3() {
	;
}
