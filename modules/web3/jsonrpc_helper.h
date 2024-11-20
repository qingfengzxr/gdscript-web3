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
	String m_path_url;
	int m_port;

protected:
	static void _bind_methods();

public:
	JsonrpcHelper();
	~JsonrpcHelper();

	/**
	 * @brief Gets the hostname.
	 * @return The hostname as a String.
	 */
	String get_hostname() const;

	/**
	 * @brief Sets the hostname.
	 * @param hostname The hostname to set.
	 */
	void set_hostname(const String &hostname);

	/**
	 * @brief Gets the path URL.
	 * @return The path URL as a String.
	 */
	String get_path_url() const;

	/**
	 * @brief Sets the path URL.
	 * @param path_url The path URL to set.
	 */
	void set_path_url(const String &path_url);

	/**
	 * @brief Gets the port number.
	 * @return The port number as an integer.
	 */
	int get_port() const;

	/**
	 * @brief Sets the port number.
	 * @param port The port number to set.
	 */
	void set_port(int port);

	/**
	 * @brief Makes a JSON-RPC call to the specified method with given parameters.
	 * @param method The name of the method to call.
	 * @param params The parameters to pass to the method.
	 * @param id The ID of the request.
	 * @param timeout_ms The timeout for the request in milliseconds (default is 20000 ms).
	 * @return A Dictionary containing the response from the JSON-RPC call.
	 */
	Dictionary call_method(const String &method, const Vector<Variant> &params, const Variant &id, int timeout_ms = 20000);
};

#endif // JSONRPC_HELPER_H
