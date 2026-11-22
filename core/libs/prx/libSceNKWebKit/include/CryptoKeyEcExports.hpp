#pragma once

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/CryptoAlgorithmAesCbcExports.hpp"
#include "prx/libSceNKWebKit/src/WtfStringAbi.hpp"

#include <atomic>
#include <cstddef>
#include <utility>

namespace NKWebKit::CryptoAbi {

struct KeyEC {
    Key base;
    void* platformKey;
    int curve;
};

inline void Deref(Key* key) {
    if (key && std::atomic_ref<unsigned>(key->refCount).fetch_sub(1) == 1) {
        reinterpret_cast<void(APS5_VABI*)(Key*)>(key->vtable[1])(key);
    }
}

struct KeyECPtr {
    KeyEC* ptr;

    explicit KeyECPtr(KeyEC* value) : ptr(value) {}
    KeyECPtr(const KeyECPtr&) = delete;
    KeyECPtr& operator=(const KeyECPtr&) = delete;
    ~KeyECPtr() { Deref(ptr ? &ptr->base : nullptr); }
};

struct KeyPair {
    Key* publicKey;
    Key* privateKey;
};

struct KeyPairOrException {
    union {
        KeyPair value;
        Exception exception;
    };
    bool hasValue;

    explicit KeyPairOrException(KeyPair pair) : value(pair), hasValue(true) {}
    explicit KeyPairOrException(int code) : exception{code, {}}, hasValue(false) {}
    KeyPairOrException(const KeyPairOrException&) = delete;
    KeyPairOrException& operator=(const KeyPairOrException&) = delete;
    ~KeyPairOrException() {
        if (hasValue) {
            Deref(value.privateKey);
            Deref(value.publicKey);
        } else {
            exception.~Exception();
        }
    }
};

struct OptionalByteVector {
    union {
        ByteVector value;
    };
    bool hasValue;

    OptionalByteVector() : hasValue(false) {}
    explicit OptionalByteVector(ByteVector&& result) : value(std::move(result)), hasValue(true) {}
    OptionalByteVector(const OptionalByteVector&) = delete;
    OptionalByteVector& operator=(const OptionalByteVector&) = delete;
    ~OptionalByteVector() {
        if (hasValue) {
            value.~ByteVector();
        }
    }
};

static_assert(offsetof(KeyEC, platformKey) == 32);
static_assert(offsetof(KeyEC, curve) == 40);
static_assert(sizeof(KeyEC) == 48);
static_assert(sizeof(KeyECPtr) == 8);
static_assert(sizeof(KeyPair) == 16);
static_assert(sizeof(KeyPairOrException) == 24);
static_assert(offsetof(KeyPairOrException, hasValue) == 16);
static_assert(sizeof(OptionalByteVector) == 24);
static_assert(offsetof(OptionalByteVector, hasValue) == 16);

}  // namespace NKWebKit::CryptoAbi

extern "C" {

NKWebKit::CryptoAbi::KeyPairOrException* APS5_VABI _ZN7WebCore11CryptoKeyEC12generatePairENS_25CryptoAlgorithmIdentifierERKN3WTF6StringEbi(NKWebKit::CryptoAbi::KeyPairOrException* result, int identifier, const NKWebKit::WtfAbi::String& curve, bool extractable, int usages);
NKWebKit::CryptoAbi::KeyECPtr* APS5_VABI _ZN7WebCore11CryptoKeyEC9importRawENS_25CryptoAlgorithmIdentifierERKN3WTF6StringEONS2_6VectorIhLm0ENS2_15CrashOnOverflowELm16ENS2_10FastMallocEEEbi(NKWebKit::CryptoAbi::KeyECPtr* result, int identifier, const NKWebKit::WtfAbi::String& curve, NKWebKit::CryptoAbi::ByteVector&& keyData, bool extractable, int usages);
NKWebKit::CryptoAbi::ByteVectorOrException* APS5_VABI _ZNK7WebCore11CryptoKeyEC9exportRawEv(NKWebKit::CryptoAbi::ByteVectorOrException* result, const NKWebKit::CryptoAbi::KeyEC* self);
NKWebKit::CryptoAbi::OptionalByteVector* APS5_VABI _ZN7WebCore19CryptoAlgorithmECDH18platformDeriveBitsERKNS_11CryptoKeyECES3_(NKWebKit::CryptoAbi::OptionalByteVector* result, const NKWebKit::CryptoAbi::KeyEC& baseKey, const NKWebKit::CryptoAbi::KeyEC& publicKey);

}
