#include "jsonrpc_helper.h"
#include <string>
#include <iostream>

JsonrpcHelper::JsonrpcHelper() {
	// use https as default
	m_port = 7545; // todo: need to let user set port
	// just use for test.
	// m_hostname = "https://optimism.llamarpc.com";
    m_hostname = "https://rpc-sepolia.rockx.com";
}

JsonrpcHelper::~JsonrpcHelper() {
	;
}

String JsonrpcHelper::get_hostname() const {
    return m_hostname;
}

void JsonrpcHelper::set_hostname(const String &hostname) {
    m_hostname = hostname;
}

int JsonrpcHelper::get_port() const {
    return m_port;
}

void JsonrpcHelper::set_port(int port) {
    m_port = port;
}

Dictionary JsonrpcHelper::call_method(const String &method, const Vector<Variant> &params, const Variant &id, int timeout_ms) {
    HTTPClient *client = HTTPClient::create();

    Dictionary call_result;
    call_result["success"] = true;

    JSONRPC* jsonrpc = new JSONRPC();
    Dictionary request = jsonrpc->make_request(method, params, id);
    String msg = Variant(request).to_json_string();

    printf("Debug! request msg: %s\n", msg.utf8().get_data());

    if (m_hostname == "" || m_port == 0) {
        ERR_PRINT("hostname or port not set.");
        call_result["success"] = false;
        call_result["errmsg"] = String("hostname or port not set. host: {0}, port: {1}").format(varray(m_hostname, m_port));
        return call_result;
    }

    Error err = client->connect_to_host(m_hostname, m_port, nullptr);
    if (err != OK) {
        ERR_PRINT("Error connect_to_host.");
        call_result["success"] = false;
        call_result["errmsg"] = String("fail for connect host: {0}, port: {1}").format(varray(m_hostname, m_port));
        return call_result;
    }

    // Start the timer
    uint64_t start_time = OS::get_singleton()->get_ticks_msec();

    // wait connect, it's necessary to wait connect done.
    while (client->get_status() == HTTPClient::STATUS_CONNECTING ||
           client->get_status() == HTTPClient::STATUS_RESOLVING) {
        client->poll();

        // Check if the timeout has been reached
        uint64_t current_time = OS::get_singleton()->get_ticks_msec();
        if (current_time - start_time > timeout_ms) {
            ERR_PRINT("Connection timeout.");
            call_result["success"] = false;
            call_result["errmsg"] = "Connection timeout.";
            return call_result;
        }
    }

    if (client->get_status() != HTTPClient::STATUS_CONNECTED) {
        String errmsg = "fail for connect. status: " + String::num_int64(client->get_status());
        ERR_PRINT(errmsg);
        call_result["success"] = false;
        call_result["errmsg"] = errmsg;
        return call_result;
    }

    Vector<String> headers;
    headers.push_back("Content-Type: application/json");
    headers.push_back("Content-Length: " + itos(msg.utf8().length()));
    // headers.push_back("Accept-Encoding: gzip, deflate"); // TODO: maybe needn't

    // request need to be uint8_t array
    CharString charstr = msg.utf8();
    Vector<uint8_t> uint8_array;
    for (int i = 0; i < charstr.length(); ++i) {
        uint8_array.push_back(static_cast<uint8_t>(charstr[i]));
    }

    // send post request
    err = client->request(HTTPClient::Method::METHOD_POST, "/", headers, uint8_array.ptr(), uint8_array.size());
    if (err != OK) {
        ERR_PRINT("Error sending request.");
        call_result["success"] = false;
        call_result["errmsg"] = String("fail for sending request. err: {0}").format(varray(err));
        return call_result;
    }

    // waiting response
    while (client->get_status() == HTTPClient::STATUS_REQUESTING) {
        client->poll();

        // Check if the timeout has been reached
        uint64_t current_time = OS::get_singleton()->get_ticks_msec();
        if (current_time - start_time > timeout_ms) {
            ERR_PRINT("Request timeout.");
            call_result["success"] = false;
            call_result["errmsg"] = "Request timeout.";
            return call_result;
        }
    }

    if (client->get_status() != HTTPClient::STATUS_BODY &&
        client->get_status() != HTTPClient::STATUS_CONNECTED) {
        String errmsg = "Error response. status: " + String::num_int64(client->get_status());
        ERR_PRINT(errmsg);
        call_result["success"] = false;
        call_result["errmsg"] = errmsg;
        return call_result;
    }

    // read response body data
    PackedByteArray response_body;
    while (client->get_status() == HTTPClient::STATUS_BODY) {
        client->poll();
        PackedByteArray chunk = client->read_response_body_chunk();
        if (chunk.size() == 0) {
            // waiting more package
            OS::get_singleton()->delay_usec(500); // 500us
            continue;
        }
        response_body.append_array(chunk);
    }

    // change Vector<uint8_t> to String
    String response_body_str;
    if (response_body.size() > 0) {
        response_body_str = String::utf8((const char*)response_body.ptr(), response_body.size());
    }

    // TODO: Maybe returning the specified type is a better implementation
    call_result["response_body"] = String::utf8((const char*)response_body.ptr(), response_body.size());
    // example output: Response body: {"jsonrpc":"2.0","id":1,"result":"0x74751e4"}
    printf("Debug! Response body: %s\n", response_body_str.utf8().get_data());
    return call_result;
}

void JsonrpcHelper::_bind_methods() {
    ClassDB::bind_method(D_METHOD("get_hostname"), &JsonrpcHelper::get_hostname);
    ClassDB::bind_method(D_METHOD("set_hostname", "hostname"), &JsonrpcHelper::set_hostname);
    ClassDB::bind_method(D_METHOD("get_port"), &JsonrpcHelper::get_port);
    ClassDB::bind_method(D_METHOD("set_port", "port"), &JsonrpcHelper::set_port);

    ClassDB::bind_method(D_METHOD("call_method", "method", "params", "id"), &JsonrpcHelper::call_method);

    ADD_PROPERTY(PropertyInfo(Variant::STRING, "hostname"), "set_hostname", "get_hostname");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "port"), "set_port", "get_port");
}
