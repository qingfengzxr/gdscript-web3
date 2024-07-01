#include "jsonrpc_helper.h"
#include <string>
#include <iostream>

JsonrpcHelper::JsonrpcHelper() {
	;
}

JsonrpcHelper::~JsonrpcHelper() {
	;
}

String JsonrpcHelper::eth_block_number() {
    HTTPClient *client = HTTPClient::create();

	Vector<String> p_params	= Vector<String>();
	Dictionary request = make_request("eth_blockNumber", p_params, 1);
	String msg = Variant(request).to_json_string();

	printf("request msg: %s\n", msg.utf8().get_data());

	String host_name = "https://optimism.llamarpc.com";
    Error err = client->connect_to_host(host_name, 443, nullptr); // if use http, port is 80
    if (err != OK) {
		ERR_PRINT("Error connect_to_host.");
        return "";
    }

    // wait connect, it's necessray to wait connect done.
    while (client->get_status() == HTTPClient::STATUS_CONNECTING ||
           client->get_status() == HTTPClient::STATUS_RESOLVING) {
        client->poll();
    }

    if (client->get_status() != HTTPClient::STATUS_CONNECTED) {
		String errmsg = "Error connect 2. status: " + String::num_int64(client->get_status());
		ERR_PRINT(errmsg);
        return "";
    }

    Vector<String> headers;
    headers.push_back("Content-Type: application/json");
    headers.push_back("Content-Length: " + itos(msg.utf8().length()));
	headers.push_back("Accept-Encoding: gzip, deflate"); // TODO: maybe needn't

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
        return "";
    }

    // waiting response
    while (client->get_status() == HTTPClient::STATUS_REQUESTING) {
        client->poll();
    }

    if (client->get_status() != HTTPClient::STATUS_BODY &&
        client->get_status() != HTTPClient::STATUS_CONNECTED) {
		String errmsg = "Error response. status: " + String::num_int64(client->get_status());
		ERR_PRINT(errmsg);
        return "";
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

	// example output: Response body: {"jsonrpc":"2.0","id":1,"result":"0x74751e4"}
    printf("Debug! Response body: %s\n", response_body_str.utf8().get_data());
	return response_body_str;
}


void JsonrpcHelper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("eth_block_number"), &JsonrpcHelper::eth_block_number);
}
