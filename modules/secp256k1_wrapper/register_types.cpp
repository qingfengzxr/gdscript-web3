/* register_types.cpp */

#include "register_types.h"

#include "core/object/class_db.h"
#include "modules/secp256k1_wrapper/src/secp256k1_wrapper.h"

void initialize_secp256k1_wrapper_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
			return;
	}
	ClassDB::register_class<Secp256k1Wrapper>();
}

void uninitialize_secp256k1_wrapper_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
			return;
	}
   // Nothing to do here in this example.
}
