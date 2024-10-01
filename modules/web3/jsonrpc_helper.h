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

class JsonrpcHelper : public RefCounted {
	GDCLASS(JsonrpcHelper, RefCounted);

	String m_hostname;
	int m_port;

protected:
	static void _bind_methods();

public:
	enum RPCMethod {
		ETH_BLOCKNUMBER,
		ETH_SENDRAWTRANSACTION,
		// ...
	};

	JsonrpcHelper();
	~JsonrpcHelper();

	String get_hostname() const;
	void set_hostname(const String &hostname);
	int get_port() const;
	void set_port(int port);

	// base request method
	Dictionary call_method(const String &method, const Vector<Variant> &params, const Variant &id, int timeout_ms = 5000);

	String format_output(const String &p_text);
};

#endif // JSONRPC_HELPER_H
