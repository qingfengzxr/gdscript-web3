#include "secp256k1_wrapper.h"

Secp256k1Wrapper::Secp256k1Wrapper() {
	;
}

Secp256k1Wrapper::~Secp256k1Wrapper() {
	secp256k1_context_destroy(ctx);
}

bool Secp256k1Wrapper::initialize() {
	ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);

	unsigned char randomize[32];

	if (!fill_random(randomize, sizeof(randomize))) {
		std::cout << "Failed to generate randomness" << std::endl;
		return false;
	}

	int return_val = secp256k1_context_randomize(ctx, randomize);
	if (return_val != 1) {
		std::cout << "Failed to randomize context" << std::endl;
		return false;
	}

	return true;
}

bool Secp256k1Wrapper::set_secret_key(const std::string& key) {
	std::string hex_key = key;
	if (hex_key.substr(0, 2) == "0x") {
		hex_key = hex_key.substr(2);
	}

	secret_key.fill(0);

	// Check if the hex_key has an even number of characters
	if (hex_key.length() % 2 != 0) {
		std::cout << "Invalid private key format" << std::endl;
		return false;
	}

	// Convert each pair of characters in hex_key to a byte and store it in secret_key
	for (size_t i = 0; i < hex_key.length(); i += 2) {
		std::string byte_str = hex_key.substr(i, 2);
		unsigned char byte = static_cast<unsigned char>(std::stoi(byte_str, nullptr, 16));
		secret_key[i / 2] = byte;
	}

	return true;
}

const std::array<unsigned char, 32> Secp256k1Wrapper::get_secret_key() const {
	return secret_key;
}

bool Secp256k1Wrapper::set_public_key(const std::string& key) {
	std::string hex_key = key;
	if (hex_key.substr(0, 2) == "0x") {
		hex_key = hex_key.substr(2);
	}

	// Check if the hex_key has an even number of characters
	if (hex_key.length() % 2 != 0) {
		std::cout << "Invalid public key format" << std::endl;
		return false;
	}

	// Clear the public_key array
	for (size_t i = 0; i < sizeof(public_key); i++) {
		public_key.data[i] = 0;
	}

	// Convert each pair of characters in hex_key to a byte and store it in public_key
	for (size_t i = 0; i < hex_key.length(); i += 2) {
		std::string byte_str = hex_key.substr(i, 2);
		unsigned char byte = static_cast<unsigned char>(std::stoi(byte_str, nullptr, 16));
		public_key.data[i / 2] = byte;
	}

	return true;
}


const secp256k1_pubkey  Secp256k1Wrapper::get_public_key() const {
	return public_key;
}

bool Secp256k1Wrapper::save_public_key(const std::string& path) {
	std::ofstream file(path, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Failed to open file" << std::endl;
		return false;
	}

	std::stringstream hex_stream;
	hex_stream << std::hex << std::setfill('0');
	for (const auto& byte : public_key.data) {
		hex_stream << std::setw(2) << static_cast<int>(byte);
	}
	std::string hex_key = hex_stream.str();

	if (hex_key.substr(0, 2) != "0x") {
		hex_key = "0x" + hex_key;
	}

	file << hex_key;
	file.close();
	return true;
}

bool Secp256k1Wrapper::save_secret_key(const std::string& path) {
	std::ofstream file(path, std::ios::binary);
	if (!file.is_open()) {
		std::cout << "Failed to open file" << std::endl;
		return false;
	}

	std::stringstream hex_stream;
	hex_stream << std::hex << std::setfill('0');
	for (const auto& byte : secret_key) {
		hex_stream << std::setw(2) << static_cast<int>(byte);
	}
	std::string hex_key = hex_stream.str();

	if (hex_key.substr(0, 2) != "0x") {
		hex_key = "0x" + hex_key;
	}

	file << hex_key;
	file.close();
	return true;
}

void Secp256k1Wrapper::print_secret_key() {
	std::cout << "Private key: ";
	for (const auto& byte : secret_key) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
	}
	std::cout << std::endl;
}

void Secp256k1Wrapper::print_public_key() {
	std::cout << "Public key: ";
	for (const auto& byte : public_key.data) {
		std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
	}
	std::cout << std::endl;
}

bool Secp256k1Wrapper::generate_key_pair() {
	unsigned char seckey[32];
	unsigned char compressed_pubkey[33];

	/* If the secret key is zero or out of range (bigger than secp256k1's
	* order), we try to sample a new key. Note that the probability of this
	* happening is negligible. */
	while (1) {
		if (!fill_random(seckey, sizeof(seckey))) {
			printf("Failed to generate randomness\n");
			return 1;
		}
		if (secp256k1_ec_seckey_verify(ctx, seckey)) {
			break;
		}
	}

	if (!secp256k1_ec_pubkey_create(ctx, &public_key, seckey)) {
		std::cout << "Failed to create public key" << std::endl;
		return false;
	}

	std::copy(seckey, seckey + sizeof(seckey), secret_key.begin());
	return true;
}

bool Secp256k1Wrapper::compute_public_key_from_seckey() {
	if (secret_key.size() != 32) {
		std::cout << "Invalid private key size" << std::endl;
		return false;
	}

	unsigned char seckey[32];
	std::copy(secret_key.begin(), secret_key.end(), seckey);

	if (!secp256k1_ec_seckey_verify(ctx, seckey)) {
		std::cout << "Invalid secret key" << std::endl;
		return false;
	}

	if (!secp256k1_ec_pubkey_create(ctx, &public_key, seckey)) {
		std::cout << "Failed to create public key" << std::endl;
		return false;
	}

	return true;
}


bool Secp256k1Wrapper::sign(const std::vector<unsigned char>& message, secp256k1_ecdsa_signature& signature) {
	if (secret_key.size() != 32) {
		std::cout << "Invalid private key size" << std::endl;
		return false;
	}

	// if (message.size() != 32) {
	//     std::cout << "Invalid message size" << std::endl;
	//     return false;
	// }

	unsigned char seckey[32];
	unsigned char serialized_signature[64];

	std::copy(secret_key.begin(), secret_key.end(), seckey);

	print_hex(seckey, sizeof(seckey));

	if (!secp256k1_ec_seckey_verify(ctx, seckey)) {
		std::cout << "Invalid secret key" << std::endl;
		return false;
	}

	int return_val = secp256k1_ecdsa_sign(ctx, &signature, message.data(), seckey, NULL, NULL);
	if (return_val != 1) {
		std::cout << "Failed to sign message" << std::endl;
		return false;
	}

	return true;
}

bool Secp256k1Wrapper::verify(const std::vector<unsigned char>& message, const secp256k1_ecdsa_signature& signature) {
	// if (message.size() != 32) {
	//     std::cout << "Invalid message size" << std::endl;
	//     return false;
	// }

	int return_val = secp256k1_ecdsa_verify(ctx, &signature, message.data(), &public_key);
	if (return_val != 1) {
		std::cout << "Signature verification failed" << std::endl;
		return false;
	}

	return true;
}


int Secp256k1Wrapper::fill_random(unsigned char* data, size_t size) {
	#if defined(_WIN32)
		NTSTATUS res = BCryptGenRandom(NULL, data, size, BCRYPT_USE_SYSTEM_PREFERRED_RNG);
		if (res != STATUS_SUCCESS || size > ULONG_MAX) {
			return 0;
		} else {
			return 1;
		}
	#elif defined(__linux__) || defined(__FreeBSD__)
		/* If `getrandom(2)` is not available you should fallback to /dev/urandom */
		ssize_t res = getrandom(data, size, 0);
		if (res < 0 || (size_t)res != size ) {
			return 0;
		} else {
			return 1;
		}
	#elif defined(__APPLE__) || defined(__OpenBSD__)
		/* If `getentropy(2)` is not available you should fallback to either
		* `SecRandomCopyBytes` or /dev/urandom */
		int res = getentropy(data, size);
		if (res == 0) {
			return 1;
		} else {
			return 0;
		}
	#endif

	return 0;
}

void Secp256k1Wrapper::_bind_methods() {
	ClassDB::bind_method(D_METHOD("initialize"), &Secp256k1Wrapper::initialize);

	ClassDB::bind_method(D_METHOD("set_secret_key", "key"), &Secp256k1Wrapper::set_secret_key);
	ClassDB::bind_method(D_METHOD("get_secret_key"), &Secp256k1Wrapper::get_secret_key);
	ClassDB::bind_method(D_METHOD("save_secret_key", "path"), &Secp256k1Wrapper::save_secret_key);
	ClassDB::bind_method(D_METHOD("print_secret_key"), &Secp256k1Wrapper::print_secret_key);

	ClassDB::bind_method(D_METHOD("set_public_key", "key"), &Secp256k1Wrapper::set_public_key);
	ClassDB::bind_method(D_METHOD("get_public_key"), &Secp256k1Wrapper::get_public_key);
	ClassDB::bind_method(D_METHOD("save_public_key", "path"), &Secp256k1Wrapper::save_public_key);
	ClassDB::bind_method(D_METHOD("print_public_key"), &Secp256k1Wrapper::print_public_key);

	ClassDB::bind_method(D_METHOD("generate_key_pair"), &Secp256k1Wrapper::generate_key_pair);
	ClassDB::bind_method(D_METHOD("compute_public_key_from_seckey"), &Secp256k1Wrapper::compute_public_key_from_seckey);

	// TODO: Add support for signing and verifying messages
}
