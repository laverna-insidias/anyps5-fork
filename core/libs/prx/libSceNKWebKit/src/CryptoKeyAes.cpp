// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/crypto/keys/CryptoKeyAES.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "prx/libc/include/General.hpp"
#include "prx/libSceNKWebKit/include/CryptoKeyAesExports.hpp"

#include <new>
#include <utility>

namespace {

using NKWebKit::CryptoAbi::ByteVector;
using NKWebKit::CryptoAbi::KeyAES;
using NKWebKit::CryptoAbi::KeyAESPtr;

constexpr int kTypeSecret = 2;
constexpr int kKeyClassAES = 0;

bool LengthIsValid(unsigned bytes) {
    return bytes == 16 || bytes == 24 || bytes == 32;
}

void APS5_VABI Destroy(KeyAES* key) {
    key->key.~ByteVector();
}

void APS5_VABI DestroyAndFree(KeyAES* key) {
    Destroy(key);
    _ZN3WTF8fastFreeEPv(key);
}

void* APS5_VABI Algorithm(void*, const KeyAES*) {
    NotImplemented_nid_no_patch("CryptoKeyAES::algorithm: building the KeyAlgorithm variant");
}

int APS5_VABI KeyClass(const KeyAES*) {
    return kKeyClassAES;
}

const void* const Vtable[] = {
    nullptr,
    nullptr,
    reinterpret_cast<const void*>(&Destroy),
    reinterpret_cast<const void*>(&DestroyAndFree),
    reinterpret_cast<const void*>(&Algorithm),
    reinterpret_cast<const void*>(&KeyClass),
};

}  // namespace

extern "C" {

KeyAESPtr* APS5_VABI _ZN7WebCore12CryptoKeyAES9importRawENS_25CryptoAlgorithmIdentifierEON3WTF6VectorIhLm0ENS2_15CrashOnOverflowELm16ENS2_10FastMallocEEEbi(KeyAESPtr* result, int algorithm, ByteVector&& keyData, bool extractable, int usages) {
    if (!LengthIsValid(keyData.size)) {
        return new (result) KeyAESPtr(nullptr);
    }
    auto* key = static_cast<KeyAES*>(_ZN3WTF10fastMallocEm(sizeof(KeyAES)));
    key->base.vtable = &Vtable[2];
    key->base.refCount = 1;
    key->base.algorithmIdentifier = algorithm;
    key->base.type = kTypeSecret;
    key->base.extractable = extractable;
    key->base.usages = usages;
    new (&key->key) ByteVector(std::move(keyData));
    return new (result) KeyAESPtr(key);
}

}
