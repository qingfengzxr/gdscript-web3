extends Label

# The test case
func test_expected_behavior():
    print("------> start test abi wrapper expected behavior <------")
    var abi = EthABIWrapper.new()
    assert(abi != null, "abi initialized failed!")
    print("pass: abi initialized success!")

    # test int type
    var arg_int = 0x32
    var enc_int = abi.encode("int", arg_int).hex_encode()
    var dec_int = abi.decode("int", enc_int)
    assert(enc_int == "0000000000000000000000000000000000000000000000000000000000000032", "int encode not as expected!")
    print("pass: int encode and decode success!")
    print("raw int: ", arg_int, ", decode: ", dec_int, ", encode: ", enc_int)

    # test array type
    var arg_array = [1, 0xff, -34567]
    var enc_array = abi.encode("int[]", arg_array).hex_encode()
    var dec_array = abi.decode("int[]", enc_array)
    assert(enc_array == "00000000000000000000000000000000000000000000000000000000000000200000000000000000000000000000000000000000000000000000000000000003000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff78f9", "array encode not as expected!")
    print("pass: array encode and decode success!")
    print("raw array: ", arg_array, ", decode: ", dec_array, ", encode: ", enc_array)

    return # todo: some bug in the following code

    # test address type
    var arg_address = "0x415F71C759868930B12243fBb0fb0feA4AA4435b"
    var enc_address = abi.encode("address", arg_address).hex_encode()
    var dec_address = abi.decode("address", enc_address)
    assert(enc_address == "000000000000000000000000415f71c759868930b12243fbb0fb0fea4aa4435b", "address encode not as expected!")
    print("pass: address encode and decode success!")
    print("raw address: ", arg_address, ", decode: ", dec_address, ", encode: ", enc_address)

    # test bytes type
    var arg_bytes = [0x77, 0x23, 0xe4, 0x8a]
    var enc_bytes = abi.encode("bytes", arg_bytes).hex_encode()
    var dec_bytes = abi.decode("bytes", enc_bytes)
    assert(enc_bytes == "000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000047723e48a00000000000000000000000000000000000000000000000000000000", "address encode not as expected!")
    print("pass: bytes encode and decode success!")
    print("raw bytes: ", arg_bytes, ", decode: ", dec_bytes, ", encode: ", enc_bytes)

    # test func call
    var func_name = "foo(int, int, bool, int[], address)"
    var args = [1, 2, true, [-3, 0x12, 345], '0x1aecd309e0a30d8d36ebac07f97ed3cbc7ac1b53']
    var enc_call = abi.encode_call(func_name, args).hex_encode()
    var dec_call = abi.decode_call(enc_call, ['int', 'int', 'bool', 'int[3]', 'address'])
    var func_selector = dec_call[0]
    assert(func_selector == "326cb625", "function selector not as expected!")
    assert(enc_call == "326cb62500000000000000000000000000000000000000000000000000000000000000010000000000000000000000000000000000000000000000000000000000000002000000000000000000000000000000000000000000000000000000000000000100000000000000000000000000000000000000000000000000000000000000a00000000000000000000000001aecd309e0a30d8d36ebac07f97ed3cbc7ac1b530000000000000000000000000000000000000000000000000000000000000003fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd00000000000000000000000000000000000000000000000000000000000000120000000000000000000000000000000000000000000000000000000000000159", "func call encode result not as expected!")
    print("raw bytes: ", enc_call, ", decode: ", dec_call)
    print("------> test abi wrapper expected behavior done <------")


func test_unexpected_behavior():
    pass

# Run the test case
func _ready() -> void:
    test_expected_behavior()
    # test_unexpected_behavior()
