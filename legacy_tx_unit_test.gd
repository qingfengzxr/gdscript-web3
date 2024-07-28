extends Label

# The test case
func test_expected_behavior():
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
    var rlp_hash = legacyTx.rlp_hash()
    assert(rlp_hash.hex_encode() == "5f32baa8c730fd7f5f2083b2ea5afbf17fc40efbc9d6529fa1e2ca514b0272dd", "rlp_hash result incorrect")
    print("pas: rlp_hash success!")

    # sign test
    var secp256k1 = Secp256k1Wrapper.new()
    assert(secp256k1.initialize(), "secp256k1 initialized failed!")
    print("pass: secp256k1 initialized success!")
    var set_sec_key = "37e17f7c0e6d14ad7bf694051b84b2572d638d875b0bb745bb151754de838d00"
    assert(secp256k1.set_secret_key(set_sec_key), "set_secret_key failed!")
    print("pass: set_secret_key success!")

    print("v: ",legacyTx.get_sign_v())
    print("r: ",legacyTx.get_sign_r())
    print("s: ",legacyTx.get_sign_s())
    print("chain_id: ", legacyTx.get_chain_id().to_int())
    var sign_result = legacyTx.sign_tx(secp256k1)
    assert(sign_result == 0, "sign tx failed!")
    print("pass: sign tx success!")

    print("v: ",legacyTx.get_sign_v().get_string())
    print("r: ",legacyTx.get_sign_r().get_string())
    print("s: ",legacyTx.get_sign_s().get_string())
    var enc_string = legacyTx.signedtx_marshal_binary()
    print("signed marshal binary result: ", enc_string)
    assert(enc_string == "f8708203e88504a817c800825204943535353535353535353535353535353535353535880de0b6b3a76400000182f4f6a06564d364f0e020e351466f4005209a376d15dfba0234e712a8f7fffe801247a8a025bede6451fd2a5a6f44ae791e1729bd4e46cb05bc4e62028c23e541b9b84f8b", "marshalbinary result incorrect")
    print("pass: marshalbinary success!")
    print("------> test legacy tx operations done <------")
    pass


func test_unexpected_behavior():
    pass

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
    test_expected_behavior()
    test_unexpected_behavior()
    pass

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
    pass
