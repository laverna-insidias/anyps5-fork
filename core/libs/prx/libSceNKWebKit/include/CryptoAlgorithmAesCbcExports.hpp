#pragma once

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/src/WtfStringAbi.hpp"
#include "prx/libSceNKWebKit/src/WtfVector.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace NKWebKit::CryptoAbi {

using ByteVector = NKWebKit::WtfAbi::Vector<unsigned char>;

struct AlgorithmParameters {
    const void* const* vtable;
    NKWebKit::WtfAbi::String name;
    int identifier;
};

struct BufferSource {
    void* object;
    std::uint8_t index;
};

struct AesCbcCfbParams {
    AlgorithmParameters base;
    BufferSource iv;
    mutable ByteVector ivVector;
};

struct Key {
    const void* const* vtable;
    unsigned refCount;
    int algorithmIdentifier;
    int type;
    bool extractable;
    int usages;
};

struct KeyAES {
    Key base;
    ByteVector key;
};

struct Exception {
    int code;
    NKWebKit::WtfAbi::String message;
};

struct ByteVectorOrException {
    union {
        ByteVector value;
        Exception exception;
    };
    bool hasValue;

    explicit ByteVectorOrException(ByteVector&& result) : value(std::move(result)), hasValue(true) {}
    explicit ByteVectorOrException(int code) : exception{code, {}}, hasValue(false) {}
    ByteVectorOrException(const ByteVectorOrException&) = delete;
    ByteVectorOrException& operator=(const ByteVectorOrException&) = delete;
    ~ByteVectorOrException() {
        if (hasValue) {
            value.~ByteVector();
        } else {
            exception.~Exception();
        }
    }
};

static_assert(sizeof(AlgorithmParameters) == 24);
static_assert(offsetof(AlgorithmParameters, identifier) == 16);
static_assert(sizeof(BufferSource) == 16);
static_assert(offsetof(AesCbcCfbParams, iv) == 24);
static_assert(offsetof(AesCbcCfbParams, ivVector) == 40);
static_assert(sizeof(AesCbcCfbParams) == 56);
static_assert(offsetof(Key, refCount) == 8);
static_assert(offsetof(Key, algorithmIdentifier) == 12);
static_assert(offsetof(Key, extractable) == 20);
static_assert(offsetof(Key, usages) == 24);
static_assert(offsetof(KeyAES, key) == 32);
static_assert(sizeof(KeyAES) == 48);
static_assert(sizeof(Exception) == 16);
static_assert(sizeof(ByteVectorOrException) == 24);
static_assert(offsetof(ByteVectorOrException, hasValue) == 16);

}  // namespace NKWebKit::CryptoAbi

extern "C" {

NKWebKit::CryptoAbi::ByteVectorOrException* APS5_VABI _ZN7WebCore22CryptoAlgorithmAES_CBC15platformEncryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(NKWebKit::CryptoAbi::ByteVectorOrException* result, const NKWebKit::CryptoAbi::AesCbcCfbParams& parameters, const NKWebKit::CryptoAbi::KeyAES& key, const NKWebKit::CryptoAbi::ByteVector& plainText, bool padding);
NKWebKit::CryptoAbi::ByteVectorOrException* APS5_VABI _ZN7WebCore22CryptoAlgorithmAES_CBC15platformDecryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(NKWebKit::CryptoAbi::ByteVectorOrException* result, const NKWebKit::CryptoAbi::AesCbcCfbParams& parameters, const NKWebKit::CryptoAbi::KeyAES& key, const NKWebKit::CryptoAbi::ByteVector& cipherText, bool padding);

}
