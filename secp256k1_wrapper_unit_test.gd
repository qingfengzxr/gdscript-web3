extends Label

# The test case
func test_expected_behavior():
	print("------> start test secp256k1 wrapper expected behavior <------")
	var secp256k1 = Secp256k1Wrapper.new()
	assert(secp256k1.initialize(), "secp256k1 initialized failed!")
	print("pass: secp256k1 initialized success!")

	# unitest for key pair operation
	assert(secp256k1.generate_key_pair(), "generate key pair failed!")
	print("pass: generate key pair success!")

	var set_sec_key = "37e17f7c0e6d14ad7bf694051b84b2572d638d875b0bb745bb151754de838d00"
	assert(secp256k1.set_secret_key(set_sec_key), "set_secret_key failed!")
	print("pass: set_secret_key success!")

	var get_sec_key = secp256k1.get_secret_key()
	assert(get_sec_key.hex_encode() == set_sec_key, "get_sec_key no equal set_sec_key!")
	print("pass: get_sec_key equal set_sec_key!")
	print("get_sec_key: ", get_sec_key.hex_encode())

	assert(secp256k1.compute_public_key_from_seckey(), "compute_public_key_from_seckey failed!")
	print("pass: compute_public_key_from_seckey success!")

	var get_pub_key = secp256k1.get_public_key()
	print("get_pub_key: ", get_pub_key.hex_encode())
	var right_pub_key = "04e7c1e50c2525c456113c01d8b3043ef46562562d1342efa9dfc9cda60f610829901c92ac79b40d044373a8d39c293f96e2c4a1bff23aa45deb8a7d60badccfc0"
	assert(get_pub_key.hex_encode() == right_pub_key, "get_pub_key no equal right_pub_key!")
	print("pass: get_pub_key equal right_pub_key!")

	# unitest for sign and verify
	var data = "Hello, world!"
	print("data sha256: ", data.sha256_buffer().hex_encode())

	# recover pubkey
	var sig = "450553de9c19d47acfeda0f32b2406ee9dabc13e8aca0abef8d313c163c1b45f5d5be96f8c34cd8e05de68bc7617ef069e606312f47a79a5b68b190918f087d300"
	var rpubkey = secp256k1.recover_pubkey(data.sha256_buffer(), sig.hex_decode())
	assert(rpubkey.hex_encode() == right_pub_key, "recover pub key no equal right_pub_key!")
	print("recover pubkey: ", rpubkey.hex_encode())

	# set public key
	secp256k1.set_public_key(rpubkey.hex_encode())
	get_pub_key = secp256k1.get_public_key()
	print("get_pub_key 2: ", get_pub_key.hex_encode())

	var signature = secp256k1.sign(data.sha256_buffer())
	var right_sign = "450553de9c19d47acfeda0f32b2406ee9dabc13e8aca0abef8d313c163c1b45f5d5be96f8c34cd8e05de68bc7617ef069e606312f47a79a5b68b190918f087d300"
	print("signatures: ", signature.hex_encode())
	assert(signature.hex_encode() == right_sign, "sign no equal right_sign!")
	print("pass: sign equal right_sign!")

	var verify_result = secp256k1.verify(data.sha256_buffer(), signature)
	assert(verify_result, "verify failed!")
	print("pass: verify success!")
	print("------> test secp256kq wrapper expected behavior done <------")
	pass

func test_unexpected_behavior():
	pass

# Run the test case
func _ready() -> void:
	test_expected_behavior()
	test_unexpected_behavior()

