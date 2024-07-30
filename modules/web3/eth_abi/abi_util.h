#ifndef ETH_ABI_UTIL_H
#define ETH_ABI_UTIL_H

#include "core/string/ustring.h"
#include "core/object/ref_counted.h"
#include "eth_abi/abi.h"
#include <iostream>


uint8_t hexCharToUint8(char c);

void hexStringToPackedByteArray(const char* hex, PackedByteArray& result);

char* packedByteArrayToHexString(const PackedByteArray& a);

PackedByteArray uint8PtrToPackedByteArray(const uint8_t* data, size_t size);

int getArrayArgSize(const char* argName);

#endif
