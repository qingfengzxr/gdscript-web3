extends Label


func send_transaction():
	## Create Transaction
	## 1. create a LegacyTx object
	var legacyTx = LegacyTx.new()
	legacyTx.set_nonce(2)
	var gasPrice = BigInt.new()
	gasPrice.from_string("20000000000")
	legacyTx.set_gas_price(gasPrice)
	legacyTx.set_gas_limit(210160)
	var value = BigInt.new()
	value.from_string("101")
	legacyTx.set_value(value)
	var data = "gdweb3".to_utf8_buffer().hex_encode()
	legacyTx.set_data(data.hex_decode())

	var chain_id = BigInt.new()
	chain_id.from_string("11155111")
	legacyTx.set_chain_id(chain_id)
	legacyTx.set_to_address("0xE85f5c8053C1fcdf2b7b517D0DC7C3cb36c81ABF")

	## 2. sign the LegacyTx
	## init a Optimism instance
	#var secp256k1 = Secp256k1Wrapper.new()
	## ! don't forget initialize secp256k1
	#assert(secp256k1.initialize(), "secp256k1 initialized failed!")
	## we also can create a Optimism instance to get secp256k1 instance
	## In this way, not necessary to call initialize()
	var op = Optimism.new()
	var secp256k1 = op.get_secp256k1_wrapper()
	## set private key
	var set_sec_key = "35fdfb4a77113345e1f9bee90824e0a0ecea1de7a7065e2ddc346ddf168adce3"
	assert(secp256k1.set_secret_key(set_sec_key), "set_secret_key failed!")
	## sign tx, set LegacyTx with signature
	var sign_result = legacyTx.sign_tx(secp256k1)
	assert(sign_result == 0, "sign tx failed!")
	## marshal binary
	var enc_string = legacyTx.signedtx_marshal_binary()
	print("signed marshal binary result: ", enc_string)

	## 3. call eth_sendRawTransaction rpc function
	#var op = Optimism.new()
	var send_tx_data = enc_string
	var blockNumber = op.block_number("000")
	print("rpc_blocknumber: ", blockNumber["response_body"].get_string_from_utf8())

	return 
	var rpc_result = op.send_transaction(send_tx_data, "001")
	print("rpc_result: ", rpc_result["response_body"].get_string_from_utf8())
	return

func test_expected_behavior():
	print("------> start test jsonrpc request operations <------")
	send_transaction()
	print("------> test jsonrpc request done <------")
	pass

# Called when the node enters the scene tree for the first time.
func _ready():
	test_expected_behavior()
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
