extends Label

func _hd_wallet_import_test():
	var address_const = [
	"c75185ff30635988d4ae44ab544fc66130932568",
	"fce4b4e710f93dbbb219555f71a62b4cebcfa907",
	"f85ccde06eab0391d976efff4d9de02eca09c566",
	"fb30288ec7c57819547b9b0f3cdf3941cad66790",
	"dee895839eb69fe79336c76b20045f6ca2f37f6a"
]
	var m = ["oil", "bamboo", "reject", "omit", "gentle", "boss", "useless", "fog", "genuine", "primary", "divorce", "abstract"]
	var wallet = EthWalletManager.from_mnemonic(m)
	var counts = 5
	for index in counts:
		wallet.add()

	var accounts = wallet.get_accounts()

	if accounts.size() > 0:
		for i in range(accounts.size()):
			var account = accounts[i]
		# Ensure the account is of type EthAccount
			if account is EthAccount:
				var eth_account = account as EthAccount  # Cast to EthAccount
				var address = eth_account.get_address()  # Call the method of EthAccount
				assert(address.hex_encode() == address_const[i])  # Check if it matches using the index
				print("Account Address:", address.hex_encode())
			else:
				print("Object at index", i, "is not of type EthAccount")
	else:
		print("No accounts found in the wallet.")

	pass

func _hd_wallet_create_test():

	var counts = 5
	var wallet = EthWalletManager.create(counts)
	print("HD Wallet Mnemonic : ", wallet.get_mnemonic())
	for index in counts:
		wallet.add()

	var accounts = wallet.get_accounts()

	if accounts.size() > 0:
		for i in range(accounts.size()):
			var account = accounts[i]
		# Ensure the account is of type EthAccount
			if account is EthAccount:
				var eth_account = account as EthAccount  # Cast to EthAccount
				var address = eth_account.get_address()  # Call the method of EthAccount
				print("Account Address:", address.hex_encode())
			else:
				print("Object at index", i, "is not of type EthAccount")
	else:
		print("No accounts found in the wallet.")
	pass

func _hd_wallet_encrypt_test():
	# Test function for wallet encryption and decryption
	# This function creates a wallet, encrypts it with a password,
	# then decrypts it and verifies that all accounts are preserved correctly
	var counts = 10  # Create 10 accounts for testing
	var wallet = EthWalletManager.create(counts)  # Create a new wallet with 10 accounts
	var keystone = EthWalletManager.encrypt(wallet,"test123")  # Encrypt the wallet with password "test123"
	print("keystone json :", keystone)  # Print the encrypted wallet keystone data

	var de_wallet = EthWalletManager.decrypt(keystone,"test123")  # Decrypt the wallet using the same password
	assert(de_wallet.get_accounts().size() == wallet.get_accounts().size())  # Verify account count matches
	for i in range(de_wallet.get_accounts().size()):
		var address_1 = wallet.get_accounts()[i].get_address().hex_encode()  # Get original address
		var address_2 = de_wallet.get_accounts()[i].get_address().hex_encode()  # Get decrypted address
		print("original  address : ", address_1)  # original address
		print("decrypt   address : ", address_2)  # decrypted address
		assert(address_1 == address_2)  # Verify addresses match
	pass


# Called when the node enters the scene tree for the first time.
func _ready():
	_hd_wallet_import_test();
	print("--------------------------------------------------------------")
	_hd_wallet_create_test();
	print("--------------------------------------------------------------")
	_hd_wallet_encrypt_test();
	pass # Replace with function body.


# Called every frame. 'delta' is the elapsed time since the previous frame.
func _process(delta):
	pass
