extends Label

# The test case
func test_big_int_operations():
	print("------> start test big int operations <------")
	var a = BigInt.new()
	a.from_string("12345678901234567890")
	var b = BigInt.new()
	b.from_string("98765432109876543210")

	var sum = a.add(b)
	assert(sum.get_string() == "111111111011111111100", "sum operation failed!")
	print("pass: sum operation")

	var sub = a.sub(b)
	assert(sub.get_string() == "-86419753208641975320", "sub operation failed!")
	print("pass: sub operation")

	var div = a.div(b)
	assert(div.get_string() == "0", "div operation failed!")
	print("pass: div operation")

	var mul = a.mul(b)
	print("mul: ", mul.get_string())
	assert(mul.get_string() == "1219326311370217952237463801111263526900", "mul operation failed!")
	print("pass: mul operation")

	var mod = b.mod(a)
	print("mod: ", mod.get_string())
	assert(mod.get_string() == "900000000090", "mod operation failed!")
	print("pass: mod operation")

	var cmp = a.cmp(b)
	assert(cmp == -1, "cmp operation failed!")
	print("pass: cmp operation")

	print("------> test big int operations done <------")
	pass

# Called when the node enters the scene tree for the first time.
func _ready() -> void:
	test_big_int_operations()
	pass

# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta: float) -> void:
	pass
