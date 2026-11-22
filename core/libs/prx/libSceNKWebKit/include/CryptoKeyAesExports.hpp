#pragma once

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/CryptoKeyEcExports.hpp"

namespace NKWebKit::CryptoAbi {

struct KeyAESPtr {
    KeyAES* ptr;

    explicit KeyAESPtr(KeyAES* value) : ptr(value) {}
    KeyAESPtr(const KeyAESPtr&) = delete;
    KeyAESPtr& operator=(const KeyAESPtr&) = delete;
    ~KeyAESPtr() { Deref(ptr ? &ptr->base : nullptr); }
};

static_assert(sizeof(KeyAESPtr) == 8);

}  // namespace NKWebKit::CryptoAbi

extern "C" {

NKWebKit::CryptoAbi::KeyAESPtr* APS5_VABI _ZN7WebCore12CryptoKeyAES9importRawENS_25CryptoAlgorithmIdentifierEON3WTF6VectorIhLm0ENS2_15CrashOnOverflowELm16ENS2_10FastMallocEEEbi(NKWebKit::CryptoAbi::KeyAESPtr* result, int algorithm, NKWebKit::CryptoAbi::ByteVector&& keyData, bool extractable, int usages);

}
