#ifndef ETH_ABI_UTIL_H
#define ETH_ABI_UTIL_H

#include "core/string/ustring.h"
#include "core/object/ref_counted.h"
#include "abi.h"
#include <iostream>
#include <functional>

uint8_t hexCharToUint8(char c);

void hexStringToPackedByteArray(const char* hex, PackedByteArray& result);

char* packedByteArrayToHexString(const PackedByteArray& a);

PackedByteArray uint8PtrToPackedByteArray(const uint8_t* data, size_t size);

int getArrayArgSize(const char* argName);

String join(const Vector<String>& elements, const String& delimiter);

String ResolveNameConflict(const String& rawName, const std::function<bool(const String&)>& used);

#endif
