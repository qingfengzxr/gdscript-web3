#ifndef WEB3_H
#define WEB3_H

#include <iostream>
#include <vector>
#include <iomanip>

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "core/error/error_macros.h"
#include "core/error/error_list.h"

#include "optimism.h"

class Web3 : public RefCounted {
	GDCLASS(Web3, RefCounted);

	Ref<Optimism> m_op;

protected:
	static void _bind_methods();

public:
	Web3();
	~Web3();

	Ref<Optimism> get_op_instance();
};

#endif // WEB3_H
