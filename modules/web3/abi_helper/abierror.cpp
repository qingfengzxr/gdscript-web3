#include "abierror.h"

ABIError* NewABIError(const String &name, ABIArguments inputs) {
    // TODO: Error will be implemented in the future.
    // just some dummy code to make it compile
    String str = "error " + name + "(";
    String sig = name + "(";
    PackedByteArray id;

    return memnew(ABIError(name, inputs, str, sig, id));
}
