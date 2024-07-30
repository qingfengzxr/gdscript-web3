/* register_types.cpp */

#include "register_types.h"

#include "core/object/class_db.h"
// #include "secp256k1_wrapper.h"
#include "web3.h"
#include "optimism.h"
#include "legacy_tx.h"
#include "big_int.h"
#include "jsonrpc_helper.h"
#include "eth_abi_wrapper.h"

void initialize_web3_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
			return;
	}
	ClassDB::register_class<Web3>();
	ClassDB::register_class<Optimism>();
	ClassDB::register_class<Secp256k1Wrapper>();
	ClassDB::register_class<KeccakWrapper>();
	ClassDB::register_class<LegacyTx>();
	ClassDB::register_class<BigInt>();
	ClassDB::register_class<JsonrpcHelper>();
	ClassDB::register_class<EthABIWrapper>();
}

void uninitialize_web3_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
			return;
	}
   // Nothing to do here in this example.
}


