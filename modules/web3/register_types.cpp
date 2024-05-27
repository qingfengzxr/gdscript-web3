/* register_types.cpp */

#include "register_types.h"

#include "core/object/class_db.h"
// #include "secp256k1_wrapper.h"
#include "web3.h"
#include "optimism.h"

void initialize_web3_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
			return;
	}
	ClassDB::register_class<Web3>();
	ClassDB::register_class<Optimism>();
	ClassDB::register_class<Secp256k1Wrapper>();
	ClassDB::register_class<KeccakWrapper>();
}

void uninitialize_web3_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
			return;
	}
   // Nothing to do here in this example.
}


