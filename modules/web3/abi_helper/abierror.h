#ifndef ABIERROR_H
#define ABIERROR_H 1

#include "core/object/ref_counted.h"
#include "core/string/ustring.h"
#include "core/variant/array.h"
#include "core/variant/variant.h"

#include "abiargument.h"

struct ABIError {
    String name;
    ABIArguments inputs;
    String str;

    // Sig contains the string signature according to the ABI spec.
    // e.g. error foo(uint32 a, int b) = "foo(uint32,int256)"
    // Please note that "int" is substitute for its canonical representation "int256"
    String sig;

    // ID returns the canonical representation of the error's signature used by the
    // abi definition to identify event names and types.
    PackedByteArray id;


    ABIError(const String& name_, const ABIArguments& inputs_, const String& str_, const String& sig_, const PackedByteArray& id_)
        : name(name_), inputs(inputs_), str(str_), sig(sig_), id(id_) {}
};

ABIError* NewABIError(const String &name, ABIArguments inputs);

#endif // ABIERROR_H
