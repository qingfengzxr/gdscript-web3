extends Node2D


# Called when the node enters the scene tree for the first time.
func _ready():
	#first example: use c++ module
	var jsonrpcHelper = JsonrpcHelper.new()
	var output = jsonrpcHelper.eth_block_number()
	print("example output: ", output)
	return

	#second example: use HTTPRequest scene
	# Create an HTTP request node and connect its completion signal.
	var http_request = HTTPRequest.new()
	add_child(http_request)
	# bind call back function
	http_request.request_completed.connect(self._http_request_completed)

	# Perform a POST request. The URL below returns JSON as of writing.
	# Note: Don't make simultaneous requests using a single HTTPRequest node.
	# The snippet below is provided for reference only.
	var body = JSON.new().stringify({"jsonrpc": "2.0", "method": "eth_blockNumber", "params": [], "id": "get_01"})
	var error = http_request.request("https://optimism.llamarpc.com", ['Content-Type: application/json'], HTTPClient.METHOD_POST, body)
	if error != OK:
		push_error("An error occurred in the HTTP request.")
	pass # Replace with function body.

# Called when the HTTP request is completed.
func _http_request_completed(result, response_code, headers, body):
	print("response_code: ", response_code, "\n")
	print("result: ", result, "\n")
	print("headers: ", headers, "\n")
	print("body: ", body, "\n")
	var json = JSON.new()
	json.parse(body.get_string_from_utf8())
	var response = json.get_data()

	# Will print the user agent string used by the HTTPRequest node (as recognized by httpbin.org).
	#print(response.headers["User-Agent"])
	print("response: ", response)

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
