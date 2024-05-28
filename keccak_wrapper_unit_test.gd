extends Label

# The test case
func test_expected_behavior():
	print("------> start test keccak wrapper expected behavior <------")
	var keccak = KeccakWrapper.new()
	
	var data = "Hello, web3!"
	var hash = keccak.keccak256_hash(data.to_utf8_buffer())
	print("get keccak hash: ", hash.hex_encode())
	var right_hash = "8610ca36ff04079aa5f6430c4c103c4950df5b31c0620db1d965c825b523083d"
	assert(hash.hex_encode() == right_hash, "get keccak 256 hash no equal except right result")
	print("pass: keccak256 hash")
	print("------> test keccak wrapper expected behavior <------")
	pass

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	test_expected_behavior()
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
