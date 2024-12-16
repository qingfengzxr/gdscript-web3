#include "common.h"

bool has_hex_prefix(const String &hex_str) {
    return hex_str.begins_with("0x");
}
