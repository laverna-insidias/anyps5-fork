// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/crypto/keys/CryptoKeyEC.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/crypto/openssl/CryptoKeyECOpenSSL.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/crypto/openssl/CryptoAlgorithmECDHOpenSSL.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2017-2019 Apple Inc. All rights reserved.
 * Copyright (C) 2020 Sony Interactive Entertainment Inc.
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
#include "prx/libSceLibreSsl/include/Api.hpp"
#include "prx/libSceNKWebKit/include/CryptoKeyEcExports.hpp"

#include <array>
#include <cstddef>
#include <memory>
#include <new>
#include <optional>
#include <string_view>

namespace {

using NKWebKit::CryptoAbi::ByteVector;
using NKWebKit::CryptoAbi::ByteVectorOrException;
using NKWebKit::CryptoAbi::Key;
using NKWebKit::CryptoAbi::KeyEC;
using NKWebKit::CryptoAbi::KeyECPtr;
using NKWebKit::CryptoAbi::KeyPair;
using NKWebKit::CryptoAbi::KeyPairOrException;
using NKWebKit::CryptoAbi::OptionalByteVector;
using NKWebKit::WtfAbi::String;

constexpr int kNotSupportedError = 6;
constexpr int kInvalidAccessError = 12;
constexpr int kOperationError = 30;
constexpr int kTypePublic = 0;
constexpr int kTypePrivate = 1;
constexpr int kKeyClassEC = 1;
constexpr int kPointConversionUncompressed = 4;

struct Curve {
    std::string_view name;
    int nid;
};

constexpr std::array<Curve, 3> kCurves = {{
    {"P-256", 415},
    {"P-384", 715},
    {"P-521", 716},
}};

struct EcKeyDeleter {
    void operator()(EcKey* key) const { EC_KEY_free_nid_postfix(key); }
};
using EcKeyPtr = std::unique_ptr<EcKey, EcKeyDeleter>;

struct EcPointDeleter {
    void operator()(EcPoint* point) const { EC_POINT_free_nid_postfix(point); }
};
using EcPointPtr = std::unique_ptr<EcPoint, EcPointDeleter>;

std::optional<int> NamedCurve(const String& curve) {
    const auto view = NKWebKit::WtfAbi::ViewOf(curve.impl);
    for (unsigned index = 0; index < kCurves.size(); ++index) {
        const auto name = kCurves[index].name;
        if (view.length != name.size()) {
            continue;
        }
        bool same = true;
        for (unsigned i = 0; i < view.length && same; ++i) {
            same = NKWebKit::WtfAbi::CodeUnitAt(view, i) == static_cast<char16_t>(name[i]);
        }
        if (same) {
            return static_cast<int>(index);
        }
    }
    return std::nullopt;
}

void APS5_VABI Destroy(KeyEC* key) {
    EC_KEY_free_nid_postfix(static_cast<EcKey*>(key->platformKey));
    key->platformKey = nullptr;
}

void APS5_VABI DestroyAndFree(KeyEC* key) {
    Destroy(key);
    _ZN3WTF8fastFreeEPv(key);
}

void* APS5_VABI Algorithm(void*, const KeyEC*) {
    NotImplemented_nid_no_patch("CryptoKeyEC::algorithm: building the KeyAlgorithm variant");
}

int APS5_VABI KeyClass(const KeyEC*) {
    return kKeyClassEC;
}

const void* const Vtable[] = {
    nullptr,
    nullptr,
    reinterpret_cast<const void*>(&Destroy),
    reinterpret_cast<const void*>(&DestroyAndFree),
    reinterpret_cast<const void*>(&Algorithm),
    reinterpret_cast<const void*>(&KeyClass),
};

KeyEC* Create(int identifier, int curve, int type, EcKeyPtr&& platformKey, bool extractable, int usages) {
    auto* key = static_cast<KeyEC*>(_ZN3WTF10fastMallocEm(sizeof(KeyEC)));
    key->base.vtable = &Vtable[2];
    key->base.refCount = 1;
    key->base.algorithmIdentifier = identifier;
    key->base.type = type;
    key->base.extractable = extractable;
    key->base.usages = usages;
    key->platformKey = platformKey.release();
    key->curve = curve;
    return key;
}

EcKey* PlatformKey(const KeyEC& key, const char* function) {
    if (key.base.vtable != &Vtable[2]) {
        NotImplemented_nid_no_patch(function);
    }
    return static_cast<EcKey*>(key.platformKey);
}

EcKeyPtr CreateEcKey(int curve) {
    return EcKeyPtr(EC_KEY_new_by_curve_name_nid_postfix(kCurves[curve].nid));
}

std::optional<KeyPair> GeneratePair(int identifier, int curve, bool extractable, int usages) {
    auto privateEcKey = CreateEcKey(curve);
    if (!privateEcKey || EC_KEY_generate_key_nid_postfix(privateEcKey.get()) <= 0) {
        return std::nullopt;
    }
    auto publicEcKey = CreateEcKey(curve);
    if (!publicEcKey || EC_KEY_set_public_key_nid_postfix(publicEcKey.get(), EC_KEY_get0_public_key_nid_postfix(privateEcKey.get())) <= 0) {
        return std::nullopt;
    }
    auto* publicKey = Create(identifier, curve, kTypePublic, std::move(publicEcKey), true, usages);
    auto* privateKey = Create(identifier, curve, kTypePrivate, std::move(privateEcKey), extractable, usages);
    return KeyPair{&publicKey->base, &privateKey->base};
}

KeyEC* ImportRaw(int identifier, int curve, const ByteVector& keyData, bool extractable, int usages) {
    auto key = CreateEcKey(curve);
    if (!key) {
        return nullptr;
    }
    const EcGroup* group = EC_KEY_get0_group_nid_postfix(key.get());
    EcPointPtr point(EC_POINT_new_nid_postfix(group));
    if (!point || EC_POINT_oct2point_nid_postfix(group, point.get(), keyData.data(), keyData.size, nullptr) <= 0) {
        return nullptr;
    }
    if (EC_KEY_set_public_key_nid_postfix(key.get(), point.get()) <= 0 || EC_KEY_check_key_nid_postfix(key.get()) <= 0) {
        return nullptr;
    }
    return Create(identifier, curve, kTypePublic, std::move(key), extractable, usages);
}

ByteVector ExportRaw(const EcKey* key) {
    const EcGroup* group = EC_KEY_get0_group_nid_postfix(key);
    const EcPoint* point = EC_KEY_get0_public_key_nid_postfix(key);
    if (!group || !point) {
        return {};
    }
    const std::size_t size = EC_POINT_point2oct_nid_postfix(group, point, kPointConversionUncompressed, nullptr, 0, nullptr);
    if (!size) {
        return {};
    }
    ByteVector keyData(static_cast<unsigned>(size));
    keyData.size = static_cast<unsigned>(size);
    if (EC_POINT_point2oct_nid_postfix(group, point, kPointConversionUncompressed, keyData.data(), size, nullptr) != size) {
        return {};
    }
    return keyData;
}

std::optional<ByteVector> DeriveBits(const KeyEC& baseKey, const KeyEC& publicKey) {
    EcKey* base = PlatformKey(baseKey, "CryptoAlgorithmECDH::platformDeriveBits: a base key that this library did not create");
    const EcKey* peer = PlatformKey(publicKey, "CryptoAlgorithmECDH::platformDeriveBits: a public key that this library did not create");
    if (baseKey.curve != publicKey.curve) {
        return std::nullopt;
    }
    const EcPoint* point = EC_KEY_get0_public_key_nid_postfix(peer);
    const int degree = EC_GROUP_get_degree_nid_postfix(EC_KEY_get0_group_nid_postfix(base));
    if (!point || degree <= 0) {
        return std::nullopt;
    }
    const unsigned size = static_cast<unsigned>((degree + 7) / 8);
    ByteVector secret(size);
    secret.size = size;
    const int length = ECDH_compute_key_nid_postfix(secret.data(), size, point, base, nullptr);
    if (length <= 0) {
        return std::nullopt;
    }
    secret.size = static_cast<unsigned>(length);
    return secret;
}

}  // namespace

extern "C" {

KeyPairOrException* APS5_VABI _ZN7WebCore11CryptoKeyEC12generatePairENS_25CryptoAlgorithmIdentifierERKN3WTF6StringEbi(KeyPairOrException* result, int identifier, const String& curve, bool extractable, int usages) {
    const auto namedCurve = NamedCurve(curve);
    if (!namedCurve) {
        return new (result) KeyPairOrException(kNotSupportedError);
    }
    auto pair = GeneratePair(identifier, *namedCurve, extractable, usages);
    if (!pair) {
        return new (result) KeyPairOrException(kOperationError);
    }
    return new (result) KeyPairOrException(*pair);
}

KeyECPtr* APS5_VABI _ZN7WebCore11CryptoKeyEC9importRawENS_25CryptoAlgorithmIdentifierERKN3WTF6StringEONS2_6VectorIhLm0ENS2_15CrashOnOverflowELm16ENS2_10FastMallocEEEbi(KeyECPtr* result, int identifier, const String& curve, ByteVector&& keyData, bool extractable, int usages) {
    const auto namedCurve = NamedCurve(curve);
    return new (result) KeyECPtr(namedCurve ? ImportRaw(identifier, *namedCurve, keyData, extractable, usages) : nullptr);
}

ByteVectorOrException* APS5_VABI _ZNK7WebCore11CryptoKeyEC9exportRawEv(ByteVectorOrException* result, const KeyEC* self) {
    if (!self) {
        throw std::invalid_argument("CryptoKeyEC::exportRaw: null key");
    }
    if (self->base.type != kTypePublic) {
        return new (result) ByteVectorOrException(kInvalidAccessError);
    }
    ByteVector keyData = ExportRaw(PlatformKey(*self, "CryptoKeyEC::exportRaw: a key that this library did not create"));
    if (!keyData.size) {
        return new (result) ByteVectorOrException(kOperationError);
    }
    return new (result) ByteVectorOrException(std::move(keyData));
}

OptionalByteVector* APS5_VABI _ZN7WebCore19CryptoAlgorithmECDH18platformDeriveBitsERKNS_11CryptoKeyECES3_(OptionalByteVector* result, const KeyEC& baseKey, const KeyEC& publicKey) {
    auto secret = DeriveBits(baseKey, publicKey);
    if (!secret) {
        return new (result) OptionalByteVector();
    }
    return new (result) OptionalByteVector(std::move(*secret));
}

}
