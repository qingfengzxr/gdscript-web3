extends Node

const CONTRACT_ABI := """
[{"inputs":[{"internalType":"string","name":"name","type":"string"},{"internalType":"string","name":"symbol","type":"string"},{"internalType":"uint8","name":"decimals","type":"uint8"}],"stateMutability":"nonpayable","type":"constructor"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"owner","type":"address"},{"indexed":true,"internalType":"address","name":"spender","type":"address"},{"indexed":false,"internalType":"uint256","name":"value","type":"uint256"}],"name":"Approval","type":"event"},{"anonymous":false,"inputs":[{"indexed":true,"internalType":"address","name":"from","type":"address"},{"indexed":true,"internalType":"address","name":"to","type":"address"},{"indexed":false,"internalType":"uint256","name":"value","type":"uint256"}],"name":"Transfer","type":"event"},{"inputs":[{"internalType":"address","name":"","type":"address"},{"internalType":"address","name":"","type":"address"}],"name":"allowance","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"spender","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"approve","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"","type":"address"}],"name":"balanceOf","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"from","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"burn","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[],"name":"decimals","outputs":[{"internalType":"uint8","name":"","type":"uint8"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"to","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"mint","outputs":[],"stateMutability":"nonpayable","type":"function"},{"inputs":[],"name":"name","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"symbol","outputs":[{"internalType":"string","name":"","type":"string"}],"stateMutability":"view","type":"function"},{"inputs":[],"name":"totalSupply","outputs":[{"internalType":"uint256","name":"","type":"uint256"}],"stateMutability":"view","type":"function"},{"inputs":[{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"transfer","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"nonpayable","type":"function"},{"inputs":[{"internalType":"address","name":"sender","type":"address"},{"internalType":"address","name":"recipient","type":"address"},{"internalType":"uint256","name":"amount","type":"uint256"}],"name":"transferFrom","outputs":[{"internalType":"bool","name":"","type":"bool"}],"stateMutability":"nonpayable","type":"function"}]
"""

const CONTRACT_ADDRESS := "0xFDb177bb14C1a0Af11a2Da29fE824C44048e69d9"
const CONTRACT_BYTECODE := "60806040523480156200001157600080fd5b5060405162000f7338038062000f73833981810160405260608110156200003757600080fd5b81019080805160405193929190846401000000008211156200005857600080fd5b838201915060208201858111156200006f57600080fd5b82518660018202830111640100000000821117156200008d57600080fd5b8083526020830192505050908051906020019080838360005b83811015620000c3578082015181840152602081019050620000a6565b50505050905090810190601f168015620000f15780820380516001836020036101000a031916815260200191505b50604052602001805160405193929190846401000000008211156200011557600080fd5b838201915060208201858111156200012c57600080fd5b82518660018202830111640100000000821117156200014a57600080fd5b8083526020830192505050908051906020019080838360005b838110156200018057808201518184015260208101905062000163565b50505050905090810190601f168015620001ae5780820380516001836020036101000a031916815260200191505b50604052602001805190602001909291905050508282828260039080519060200190620001dd929190620002ff565b508160049080519060200190620001f6929190620002ff565b5080600560006101000a81548160ff021916908360ff16021790555050505062000230338260ff16600a0a6064026200023960201b60201c565b505050620003b5565b80600160008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508060008082825401925050819055508173ffffffffffffffffffffffffffffffffffffffff16600073ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef836040518082815260200191505060405180910390a35050565b828054600181600116156101000203166002900490600052602060002090601f01602090048101928262000337576000855562000383565b82601f106200035257805160ff191683800117855562000383565b8280016001018555821562000383579182015b828111156200038257825182559160200191906001019062000365565b5b50905062000392919062000396565b5090565b5b80821115620003b157600081600090555060010162000397565b5090565b610bae80620003c56000396000f3fe608060405234801561001057600080fd5b50600436106100a95760003560e01c806340c10f191161007157806340c10f191461025857806370a08231146102a657806395d89b41146102fe5780639dc29fac14610381578063a9059cbb146103cf578063dd62ed3e14610433576100a9565b806306fdde03146100ae578063095ea7b31461013157806318160ddd1461019557806323b872dd146101b3578063313ce56714610237575b600080fd5b6100b66104ab565b6040518080602001828103825283818151815260200191508051906020019080838360005b838110156100f65780820151818401526020810190506100db565b50505050905090810190601f1680156101235780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b61017d6004803603604081101561014757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610549565b60405180821515815260200191505060405180910390f35b61019d61063b565b6040518082815260200191505060405180910390f35b61021f600480360360608110156101c957600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff16906020019092919080359060200190929190505050610641565b60405180821515815260200191505060405180910390f35b61023f6107d7565b604051808260ff16815260200191505060405180910390f35b6102a46004803603604081101561026e57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506107ea565b005b6102e8600480360360208110156102bc57600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff1690602001909291905050506107f8565b6040518082815260200191505060405180910390f35b610306610810565b6040518080602001828103825283818151815260200191508051906020019080838360005b8381101561034657808201518184015260208101905061032b565b50505050905090810190601f1680156103735780820380516001836020036101000a031916815260200191505b509250505060405180910390f35b6103cd6004803603604081101561039757600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506108ae565b005b61041b600480360360408110156103e557600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803590602001909291905050506108bc565b60405180821515815260200191505060405180910390f35b6104956004803603604081101561044957600080fd5b81019080803573ffffffffffffffffffffffffffffffffffffffff169060200190929190803573ffffffffffffffffffffffffffffffffffffffff1690602001909291905050506109c7565b6040518082815260200191505060405180910390f35b60038054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156105415780601f1061051657610100808354040283529160200191610541565b820191906000526020600020905b81548152906001019060200180831161052457829003601f168201915b505050505081565b600081600260003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167f8c5be1e5ebec7d5bd14f71427d1e84f3dd0314c0f7b2291e5b200ac8c7c3b925846040518082815260200191505060405180910390a36001905092915050565b60005481565b600081600260008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600160008673ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600160008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff168473ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a3600190509392505050565b600560009054906101000a900460ff1681565b6107f482826109ec565b5050565b60016020528060005260406000206000915090505481565b60048054600181600116156101000203166002900480601f0160208091040260200160405190810160405280929190818152602001828054600181600116156101000203166002900480156108a65780601f1061087b576101008083540402835291602001916108a6565b820191906000526020600020905b81548152906001019060200180831161088957829003601f168201915b505050505081565b6108b88282610ab2565b5050565b600081600160003373ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff1681526020019081526020016000206000828254039250508190555081600160008573ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508273ffffffffffffffffffffffffffffffffffffffff163373ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef846040518082815260200191505060405180910390a36001905092915050565b6002602052816000526040600020602052806000526040600020600091509150505481565b80600160008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff168152602001908152602001600020600082825401925050819055508060008082825401925050819055508173ffffffffffffffffffffffffffffffffffffffff16600073ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef836040518082815260200191505060405180910390a35050565b80600160008473ffffffffffffffffffffffffffffffffffffffff1673ffffffffffffffffffffffffffffffffffffffff16815260200190815260200160002060008282540392505081905550806000808282540392505081905550600073ffffffffffffffffffffffffffffffffffffffff168273ffffffffffffffffffffffffffffffffffffffff167fddf252ad1be2c89b69c2b068fc378daa952ba7f163c4a11628f55a4df523b3ef836040518082815260200191505060405180910390a3505056fea264697066735822122038d852141b888f7b1f453d65b7ef83eb01c87f9fbf08b01588412b6b4d3d4cb864736f6c63430007060033"
const NODE_RPC_URL := "http://127.0.0.1"
const NODE_RPC_PORT := "7545"

func deploy_contract():
	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var h = ABIHelper.new()
	var res = h.unmarshal_from_json(CONTRACT_ABI)
	if !res:
		print("unmarshal_from_json failed!")
		return

	var packed = h.pack("", ["GOLD", "GD", 8])
	# 以16进制输出
	print("---> result: ", packed.hex_encode())
	var data = PackedByteArray()
	data.append_array(CONTRACT_BYTECODE.hex_decode())
	data.append_array(packed)

	var op = Optimism.new()
	op.set_rpc_url(NODE_RPC_URL)
	var gas_price = op.suggest_gas_price("001")
	print("----> gas price:", gas_price.get_string())

	# create a legacyTx
	print("----> will create a legacyTx")
	var legacyTx = LegacyTx.new()
	legacyTx.set_nonce(1)
	legacyTx.set_gas_price(gas_price)
	legacyTx.set_gas_limit(800000)
	var value = BigInt.new()
	value.from_string("0") # FIXME： value should support zero
	legacyTx.set_value(value)
	legacyTx.set_data(data)
	print("gd. tx data: ", data.hex_encode())

	var chain_id = BigInt.new()
	chain_id.from_string("1337")
	legacyTx.set_chain_id(chain_id)
	print("----> will set address")
	# legacyTx.set_to_address("0x0000000000000000000000000000000000000000")
	legacyTx.set_to_address("")

	# sign tx
	var secp256k1 = op.get_secp256k1_wrapper()
	var set_sec_key = "38374ec41c40be1ab86b286447487a4c10dc69e4a20df57d7f746602584dc4ce"
	# var set_sec_key = "4d2f96832bcbbc93a27ccea84b57237c0c858dec4e93113eef2862fc19f5ba6b"
	assert(secp256k1.set_secret_key(set_sec_key), "set_secret_key failed!")
	print("pass: set_secret_key success!")

	var sign_result = legacyTx.sign_tx(secp256k1)
	assert(sign_result == 0, "sign tx failed!")
	print("pass: sign tx success!")

	## marshal binary
	var send_tx_data = legacyTx.signedtx_marshal_binary()
	print("gd signed marshal binary result: ", send_tx_data)

	var rpc_result = op.send_transaction(send_tx_data, "002")
	print("rpc_result: ", rpc_result["response_body"])
	return

func name():
	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var h = ABIHelper.new()
	var res = h.unmarshal_from_json(CONTRACT_ABI)
	if !res:
		print("unmarshal_from_json failed!")
		return

	var packed = h.pack("name", [])
	print(packed.hex_encode())

	var op = Optimism.new()
	op.set_rpc_url(NODE_RPC_URL)
	var call_msg = {
		"from": "0x0000000000000000000000000000000000000000",
		"to": CONTRACT_ADDRESS,
		"input": "0x" + packed.hex_encode(),
	}
	var rpc_resp = op.call_contract(call_msg, "", "001")
	print("gd: rpc_result: ", rpc_resp["response_body"].get_string_from_utf8())

	var call_result = JSON.parse_string(rpc_resp["response_body"].get_string_from_utf8())

	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var call_ret = call_result["result"]
	call_ret = call_ret.substr(2, call_ret.length() - 2)
	print("name. gd call ret: ", call_ret)
	print("========= unpack to dictionary ==============\n")
	var result = {}
	var err = h.unpack_into_dictionary("name", call_ret.hex_decode(), result)
	if err != OK:
		assert(false, "unpack_into_dictionary failed!")
	print("call result: ", result)
	return

func totalSupply():
	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var h = ABIHelper.new()
	var res = h.unmarshal_from_json(CONTRACT_ABI)
	if !res:
		print("unmarshal_from_json failed!")
		return

	var packed = h.pack("totalSupply", [])
	print(packed.hex_encode())

	var op = Optimism.new()
	op.set_rpc_url(NODE_RPC_URL)
	var call_msg = {
		"from": "0x0000000000000000000000000000000000000000",
		"to": CONTRACT_ADDRESS,
		"input": "0x" + packed.hex_encode(),
	}
	var rpc_resp = op.call_contract(call_msg, "", "001")
	print("gd: rpc_result: ", rpc_resp["response_body"].get_string_from_utf8())

	var call_result = JSON.parse_string(rpc_resp["response_body"].get_string_from_utf8())

	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var call_ret = call_result["result"]
	call_ret = call_ret.substr(2, call_ret.length() - 2)
	print("name. gd call ret: ", call_ret)
	print("========= unpack to dictionary ==============\n")
	var result = {}
	var err = h.unpack_into_dictionary("totalSupply", call_ret.hex_decode(), result)
	if err != OK:
		assert(false, "unpack_into_dictionary failed!")
	print("totalSupply: ", result["0"])
	return


func balanceOf():
	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var h = ABIHelper.new()
	var res = h.unmarshal_from_json(CONTRACT_ABI)
	if !res:
		print("unmarshal_from_json failed!")
		return

	var packed = h.pack("balanceOf", ["0xeB98753449AD50d30561a66CA48BF69EEcaD4bC3"])
	print(packed.hex_encode())

	var op = Optimism.new()
	op.set_rpc_url(NODE_RPC_URL)
	var call_msg = {
		"from": "0x0000000000000000000000000000000000000000",
		"to": CONTRACT_ADDRESS,
		"input": "0x" + packed.hex_encode(),
	}
	var rpc_resp = op.call_contract(call_msg, "", "001")
	print("gd: rpc_result: ", rpc_resp["response_body"].get_string_from_utf8())

	var call_result = JSON.parse_string(rpc_resp["response_body"].get_string_from_utf8())

	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var call_ret = call_result["result"]
	call_ret = call_ret.substr(2, call_ret.length() - 2)
	print("name. gd call ret: ", call_ret)
	print("========= unpack to dictionary ==============\n")
	var result = {}
	var err = h.unpack_into_dictionary("balanceOf", call_ret.hex_decode(), result)
	if err != OK:
		assert(false, "unpack_into_dictionary failed!")
	print("balanceOf: ", result["0"])
	return

func allowance():
	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var h = ABIHelper.new()
	var res = h.unmarshal_from_json(CONTRACT_ABI)
	if !res:
		print("unmarshal_from_json failed!")
		return

	var params = [
		# owner
		"0x183600Bc8779375D5d369C7c5133f2129e649D35",
		# spender
		"0xeB98753449AD50d30561a66CA48BF69EEcaD4bC3"
	]
	var packed = h.pack("allowance", params)
	print(packed.hex_encode())

	var op = Optimism.new()
	op.set_rpc_url(NODE_RPC_URL)
	var call_msg = {
		"from": "0x0000000000000000000000000000000000000000",
		"to": CONTRACT_ADDRESS,
		"input": "0x" + packed.hex_encode(),
	}
	var rpc_resp = op.call_contract(call_msg, "", "001")
	print("gd: rpc_result: ", rpc_resp["response_body"].get_string_from_utf8())

	var call_result = JSON.parse_string(rpc_resp["response_body"].get_string_from_utf8())

	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var call_ret = call_result["result"]
	call_ret = call_ret.substr(2, call_ret.length() - 2)
	print("name. gd call ret: ", call_ret)
	print("========= unpack to dictionary ==============\n")
	var result = {}
	var err = h.unpack_into_dictionary("totalSupply", call_ret.hex_decode(), result)
	if err != OK:
		assert(false, "unpack_into_dictionary failed!")
	print("totalSupply: ", result["0"])
	return


func transfer():
	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var h = ABIHelper.new()
	var res = h.unmarshal_from_json(CONTRACT_ABI)
	if !res:
		print("unmarshal_from_json failed!")
		return

	var params = [
		# receipient
		"0xeB98753449AD50d30561a66CA48BF69EEcaD4bC3",
		# amount
		"123456"
	]
	var packed = h.pack("transfer", params)
	print("->packed. ", packed.hex_encode())

	# create a legacyTx
	print("----> will create a legacyTx")
	var legacyTx = LegacyTx.new()
	legacyTx.set_nonce(7)
	var gasPrice = BigInt.new()
	# todo: write a function to get gas price & gas limit
	gasPrice.from_string("2000000000")
	print("gasPrice: ", gasPrice.get_string())
	legacyTx.set_gas_price(gasPrice)
	legacyTx.set_gas_limit(300000)
	var value = BigInt.new()
	value.from_string("0") # FIXME： value should support zero
	legacyTx.set_value(value)
	legacyTx.set_data(packed)

	var chain_id = BigInt.new()
	chain_id.from_string("1337")
	legacyTx.set_chain_id(chain_id)
	print("----> will set address")
	legacyTx.set_to_address(CONTRACT_ADDRESS)

	# get op instance
	var op = Optimism.new()
	op.set_rpc_url(NODE_RPC_URL)
	var secp256k1 = op.get_secp256k1_wrapper()
	# todo: if the secret key has 0x prefix. We should can auto deal with it.
	var set_sec_key = "4d2f96832bcbbc93a27ccea84b57237c0c858dec4e93113eef2862fc19f5ba6b"
	assert(secp256k1.set_secret_key(set_sec_key), "set_secret_key failed!")
	print("pass: set_secret_key success!")

	var sign_result = legacyTx.sign_tx(secp256k1)
	assert(sign_result == 0, "sign tx failed!")
	print("pass: sign tx success!")

	## marshal binary
	var send_tx_data = legacyTx.signedtx_marshal_binary()
	print("gd signed marshal binary result: ", send_tx_data)

	var rpc_result = op.send_transaction(send_tx_data, "002")
	print("rpc_result: ", rpc_result["response_body"].get_string_from_utf8())

	return

func approve():
	# create a new instance of the ABIHelper class and unmarshal the ABI JSON string into it
	var h = ABIHelper.new()
	var res = h.unmarshal_from_json(CONTRACT_ABI)
	if !res:
		print("unmarshal_from_json failed!")
		return

	var params = [
		# spender
		"0xeB98753449AD50d30561a66CA48BF69EEcaD4bC3",
		# amount
		"10000012"
	]
	var packed = h.pack("approve", params)
	print("->packed. ", packed.hex_encode())

	# create a legacyTx
	print("----> will create a legacyTx")
	var legacyTx = LegacyTx.new()
	legacyTx.set_nonce(9)
	var gasPrice = BigInt.new()
	# todo: write a function to get gas price & gas limit
	gasPrice.from_string("2000000000")
	print("gasPrice: ", gasPrice.get_string())
	legacyTx.set_gas_price(gasPrice)
	legacyTx.set_gas_limit(300000)
	var value = BigInt.new()
	value.from_string("0") # FIXME： value should support zero
	legacyTx.set_value(value)
	legacyTx.set_data(packed)

	var chain_id = BigInt.new()
	chain_id.from_string("1337")
	legacyTx.set_chain_id(chain_id)
	print("----> will set address")
	legacyTx.set_to_address(CONTRACT_ADDRESS)

	# get op instance
	var op = Optimism.new()
	op.set_rpc_url(NODE_RPC_URL)
	var secp256k1 = op.get_secp256k1_wrapper()
	# todo: if the secret key has 0x prefix. We should can auto deal with it.
	var set_sec_key = "4d2f96832bcbbc93a27ccea84b57237c0c858dec4e93113eef2862fc19f5ba6b"
	assert(secp256k1.set_secret_key(set_sec_key), "set_secret_key failed!")
	print("pass: set_secret_key success!")

	var sign_result = legacyTx.sign_tx(secp256k1)
	assert(sign_result == 0, "sign tx failed!")
	print("pass: sign tx success!")

	## marshal binary
	var send_tx_data = legacyTx.signedtx_marshal_binary()
	print("gd signed marshal binary result: ", send_tx_data)

	var rpc_result = op.send_transaction(send_tx_data, "002")
	print("rpc_result: ", rpc_result["response_body"].get_string_from_utf8())

	return


# Called when the node enters the scene tree for the first time.
func _ready():
	# legacytx_test()
	deploy_contract()
	# name()
	# balanceOf()
	# totalSupply()
	# transfer()
	# approve()
	# allowance()
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
