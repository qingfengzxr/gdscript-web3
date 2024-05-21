#include "secp256k1_wrapper.h"

#include "libsecp256k1/src/secp256k1.c"
#include "libsecp256k1/src/modules/recovery/main_impl.h"
#include "libsecp256k1/include/ext.h"

Secp256k1Wrapper::Secp256k1Wrapper() {
	;
}

Secp256k1Wrapper::~Secp256k1Wrapper() {
	secp256k1_context_destroy(ctx);
}

bool Secp256k1Wrapper::initialize() {
	ctx = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);

	unsigned char randomize[32];

	ERR_FAIL_COND_V_MSG(
		fill_random(randomize, sizeof(randomize)) != true,
		false,
		"Failed to generate randomness"
	);

	int return_val = secp256k1_context_randomize(ctx, randomize);
	ERR_FAIL_COND_V_MSG(return_val != 1, FAILED, "Failed to randomize context");
	return true;
}

bool Secp256k1Wrapper::set_secret_key(const String &key) {
	std::string hex_key = key.utf8().get_data();
	if (hex_key.substr(0, 2) == "0x") {
		hex_key = hex_key.substr(2);
	}

	m_secret_key.clear();

	// Check if the hex_key has an even number of characters
	ERR_FAIL_COND_V_MSG(hex_key.length() % 2 != 0, false, "Invalid secret key format");

	// Convert each pair of characters in hex_key to a byte and store it in secret_key
	for (size_t i = 0; i < hex_key.length(); i += 2) {
		std::string byte_str = hex_key.substr(i, 2);
		uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
		m_secret_key.push_back(byte);
	}

	return true;
}

PackedByteArray Secp256k1Wrapper::get_secret_key() const {
	return m_secret_key;
}

bool Secp256k1Wrapper::set_public_key(const String &key) {
	std::string hex_key = key.utf8().get_data();
	if (hex_key.substr(0, 2) == "0x") {
		hex_key = hex_key.substr(2);
	}

	// Check if the hex_key has an even number of characters
	ERR_FAIL_COND_V_MSG(hex_key.length() % 2 != 0, false, "Invalid public key format");

	// Clear the public_key array
	m_public_key.clear();

	// Convert each pair of characters in hex_key to a byte and store it in public_key
	for (size_t i = 0; i < hex_key.length(); i += 2) {
		std::string byte_str = hex_key.substr(i, 2);
		uint8_t byte = static_cast<uint8_t>(std::stoi(byte_str, nullptr, 16));
		m_public_key.push_back(byte);
	}

	return true;
}


PackedByteArray Secp256k1Wrapper::get_public_key() const {
	return m_public_key;
}

Error Secp256k1Wrapper::save_public_key(const String &path) {
    std::ofstream file(path.utf8().get_data(), std::ios::binary);

	ERR_FAIL_COND_V_MSG(!file.is_open(), ERR_FILE_CANT_OPEN, "Failed to open file: " + path);

    std::stringstream hex_stream;
    hex_stream << std::hex << std::setfill('0');
    for (const auto& byte : m_public_key) {
        hex_stream << std::setw(2) << static_cast<int>(byte);
    }
    std::string hex_key = hex_stream.str();

    if (hex_key.substr(0, 2) != "0x") {
        hex_key = "0x" + hex_key;
    }

    file << hex_key;
    file.close();
    return OK;
}

Error Secp256k1Wrapper::save_secret_key(const String &path) {
	std::ofstream file(path.utf8().get_data(), std::ios::binary);

	ERR_FAIL_COND_V_MSG(!file.is_open(), ERR_FILE_CANT_OPEN, "Failed to open file: " + path);

	std::stringstream hex_stream;
	hex_stream << std::hex << std::setfill('0');
	for (const auto& byte : m_secret_key) {
		hex_stream << std::setw(2) << static_cast<int>(byte);
	}
	std::string hex_key = hex_stream.str();

	if (hex_key.substr(0, 2) != "0x") {
		hex_key = "0x" + hex_key;
	}

	file << hex_key;
	file.close();
	return OK;
}

bool Secp256k1Wrapper::generate_key_pair() {
	unsigned char seckey[32];
	secp256k1_pubkey pubkey;

	/* If the secret key is zero or out of range (bigger than secp256k1's
	* order), we try to sample a new key. Note that the probability of this
	* happening is negligible. */
	while (1) {
		ERR_FAIL_COND_V_MSG(!fill_random(seckey, sizeof(seckey)), false, "Failed to generate randomness");

		if (secp256k1_ec_seckey_verify(ctx, seckey)) {
			break;
		}
	}

	/* Compute the public key from a secret key. */
	ERR_FAIL_COND_V_MSG(!secp256k1_ec_pubkey_create(ctx, &pubkey, seckey), false, "Failed to create public key");

	// serialize the public key
	unsigned char serialize_pubkey[65];
	size_t outputlen = 65;
	secp256k1_ec_pubkey_serialize(ctx, serialize_pubkey, &outputlen, &pubkey, SECP256K1_EC_UNCOMPRESSED);

	// save to class member
	m_secret_key.clear();
	for (size_t i = 0; i < sizeof(seckey); ++i) {
		m_secret_key.append(seckey[i]);
	}

	m_public_key.clear();
	for (size_t i = 0; i < sizeof(pubkey); ++i) {
		m_public_key.append(pubkey.data[i]);
	}

	return true;
}

bool Secp256k1Wrapper::compute_public_key_from_seckey() {
	ERR_FAIL_COND_V_MSG(m_secret_key.size() != 32, false, "Invalid secret key size: " + itos(m_secret_key.size()) + " expected 32 bytes");

	unsigned char seckey[32];
	for (int i = 0; i < m_secret_key.size(); ++i) {
		seckey[i] = m_secret_key[i];
	}

	ERR_FAIL_COND_V_MSG(!secp256k1_ec_seckey_verify(ctx, seckey), false, "Invalid secret key");

	secp256k1_pubkey pubkey;
	if (!secp256k1_ec_pubkey_create(ctx, &pubkey, seckey)) {
		ERR_PRINT("Failed to create public key");
		return false;
	}

	// serialize the public key
	unsigned char serialize_pubkey[65];
	size_t outputlen = 65;
	secp256k1_ec_pubkey_serialize(ctx, serialize_pubkey, &outputlen, &pubkey, SECP256K1_EC_UNCOMPRESSED);

	m_public_key.clear();
	for (size_t i = 0; i < sizeof(serialize_pubkey); ++i) {
		m_public_key.append(serialize_pubkey[i]);
	}

	return true;
}


PackedByteArray Secp256k1Wrapper::sign(const PackedByteArray &message) {
	ERR_FAIL_COND_V_MSG(m_secret_key.size() != 32, PackedByteArray(), "Invalid secret key size: " + itos(m_secret_key.size()) + " expected 32 bytes");
	ERR_FAIL_COND_V_MSG(message.size() != 32, PackedByteArray(), "Invalid message size: " + itos(message.size()) + " expected 32 bytes");

	unsigned char seckey[32];
	for (int i = 0; i < m_secret_key.size(); ++i) {
		seckey[i] = m_secret_key[i];
	}

	ERR_FAIL_COND_V_MSG(secp256k1_ec_seckey_verify(ctx, seckey) != 1, PackedByteArray(), "Invalid secret key");

	// convert PackedByteArray data to unsigned char
	unsigned char sign_msg[32];
	for (int i = 0; i < message.size(); ++i) {
		sign_msg[i] = message[i];
	}

	secp256k1_ecdsa_recoverable_signature signature_output;

	int return_val = secp256k1_ecdsa_sign_recoverable(ctx, &signature_output, sign_msg, seckey, NULL, NULL);
	ERR_FAIL_COND_V_MSG(return_val != 1, PackedByteArray(), "Failed to sign message");

	// compact signature output
	int recid;
	unsigned char compact_sig[65];
	unsigned char *sigdata = &compact_sig[0];
	secp256k1_ecdsa_recoverable_signature_serialize_compact(ctx, sigdata, &recid, &signature_output);
	compact_sig[64] = recid;

	// convert compact signature data to PackedByteArray
	PackedByteArray signature;
	signature.clear();
	for (int i = 0; i < 65; ++i) {
		signature.append(compact_sig[i]);
	}

	return signature;
}

bool Secp256k1Wrapper::verify(const PackedByteArray &message, const PackedByteArray &signature) {
	ERR_FAIL_COND_V_MSG(message.size() != 32, false, "Invalid message size: " + itos(message.size()) + " expected 32 bytes");
	ERR_FAIL_COND_V_MSG(signature.size() != 65, false, "Invalid signature size: "+ itos(signature.size()) + " expected 65 bytes");

	// convert PackByteArray date to unsigned char
	unsigned char msg_data[32];
	for (int i = 0; i < message.size(); ++i) {
		msg_data[i] = message[i];
	}

	// convert PackByteArray date to unsigned char
	unsigned char sign_data[65];
	for (int i = 0; i < signature.size(); ++i) {
		sign_data[i] = signature[i];
	}

	// convert PackByteArray data to unsigned char
	unsigned char key_data[65];
	for (int i = 0; i < m_public_key.size(); ++i) {
		key_data[i] = m_public_key[i];
	}

	int return_val = secp256k1_ext_ecdsa_verify(ctx, sign_data, msg_data, key_data, m_public_key.size());
	ERR_FAIL_COND_V_MSG(return_val != 1, false, "Signature verification failed");

	return true;
}

PackedByteArray Secp256k1Wrapper::recover_pubkey(const PackedByteArray &message, const PackedByteArray &signature) {
	ERR_FAIL_COND_V_MSG(message.size() != 32, PackedByteArray(), "Invalid message size: " + itos(message.size()) + " expected 32 bytes");
	ERR_FAIL_COND_V_MSG(signature.size() != 65, PackedByteArray(), "Invalid signature size: "+ itos(signature.size()) + " expected 65 bytes");
	ERR_FAIL_COND_V_MSG(signature[64] >= 4, PackedByteArray(), "Invalid recover id: "+ itos(signature[64]));

	// convert Array date to unsigned char
	unsigned char msg_data[32];
	for (int i = 0; i < message.size(); ++i) {
		msg_data[i] = message[i];
	}

	// convert Array date to unsigned char
	unsigned char sig_data[65];
	for (int i = 0; i < signature.size(); ++i) {
		sig_data[i] = signature[i];
	}

	unsigned char pubkey_out[65];
	int ret_val = secp256k1_ext_ecdsa_recover(ctx, pubkey_out, sig_data, msg_data);
	if (ret_val != 1) {
		ERR_PRINT("Failed to recover public key");
		return PackedByteArray();
	}

	PackedByteArray return_public_key;
	return_public_key.clear();
	for (size_t i = 0; i < sizeof(pubkey_out); ++i) {
		return_public_key.append(pubkey_out[i]);
	}

	return return_public_key;
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

	ClassDB::bind_method(D_METHOD("set_public_key", "key"), &Secp256k1Wrapper::set_public_key);
	ClassDB::bind_method(D_METHOD("get_public_key"), &Secp256k1Wrapper::get_public_key);
	ClassDB::bind_method(D_METHOD("save_public_key", "path"), &Secp256k1Wrapper::save_public_key);

	ClassDB::bind_method(D_METHOD("generate_key_pair"), &Secp256k1Wrapper::generate_key_pair);
	ClassDB::bind_method(D_METHOD("compute_public_key_from_seckey"), &Secp256k1Wrapper::compute_public_key_from_seckey);
	ClassDB::bind_method(D_METHOD("recover_pubkey", "message", "signature"), &Secp256k1Wrapper::recover_pubkey);

	ClassDB::bind_method(D_METHOD("sign", "message"), &Secp256k1Wrapper::sign);
	ClassDB::bind_method(D_METHOD("verify", "message", "signature"), &Secp256k1Wrapper::verify);
}
