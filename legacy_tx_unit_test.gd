extends Label

# The test case
func test_legacy_tx_operations():
	print("------> start test legacy tx operations <------")
	var legacyTx = LegacyTx.new()
	legacyTx.set_nonce(1000)
	var gasPrice = BigInt.new()
	gasPrice.from_string("20000000000")
	legacyTx.set_gas_price(gasPrice)
	legacyTx.set_gas_limit(20996)
	var value = BigInt.new()
	value.from_string("1000000000000000000")
	legacyTx.set_value(value)
	var data = "01"
	legacyTx.set_data(data.hex_decode())
	
	var chain_id = BigInt.new()
	chain_id.from_string("31337")
	legacyTx.set_chain_id(chain_id)
	legacyTx.set_to_address("0x3535353535353535353535353535353535353535")
	var a = legacyTx.rlp_hash()
	print("------> test legacy tx operations done <------")
	pass

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	test_legacy_tx_operations()
	pass

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
