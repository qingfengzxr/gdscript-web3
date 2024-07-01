#ifndef JSONRPC_HELPER_H
#define JSONRPC_HELPER_H


#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"
#include "core/error/error_macros.h"
#include "core/error/error_list.h"

#include "core/io/http_client.h"
#include "scene/main/http_request.h"
#include "core/io/json.h"
#include "modules/jsonrpc/jsonrpc.h"

class JsonrpcHelper : public JSONRPC {
	GDCLASS(JsonrpcHelper, JSONRPC);

protected:
	static void _bind_methods();

public:
	JsonrpcHelper();
	~JsonrpcHelper();

	String eth_block_number();
	String format_output(const String &p_text);
};

#endif // JSONRPC_HELPER_H
