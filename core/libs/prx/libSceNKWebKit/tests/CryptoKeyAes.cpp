#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdio>
#include <cstdlib>
#include <new>
#include <string>
#include <utility>

namespace {

using NKWebKit::CryptoAbi::AesCbcCfbParams;
using NKWebKit::CryptoAbi::ByteVector;
using NKWebKit::CryptoAbi::ByteVectorOrException;
using NKWebKit::CryptoAbi::KeyAES;
using NKWebKit::CryptoAbi::KeyAESPtr;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "CryptoKeyAes check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(value) Check((value), __LINE__)

ByteVector Bytes(const std::string& hex) {
    ByteVector bytes;
    for (std::size_t i = 0; i < hex.size(); i += 2) {
        bytes.append(static_cast<unsigned char>(std::stoul(hex.substr(i, 2), nullptr, 16)));
    }
    return bytes;
}

std::string Hex(const ByteVector& bytes) {
    static const char digits[] = "0123456789abcdef";
    std::string text;
    for (unsigned i = 0; i < bytes.size; ++i) {
        text += digits[bytes[i] >> 4];
        text += digits[bytes[i] & 15];
    }
    return text;
}

KeyAES* Import(ByteVector& keyData, int algorithm = 8, bool extractable = true, int usages = 3) {
    alignas(KeyAESPtr) unsigned char storage[sizeof(KeyAESPtr)];
    auto* result = _ZN7WebCore12CryptoKeyAES9importRawENS_25CryptoAlgorithmIdentifierEON3WTF6VectorIhLm0ENS2_15CrashOnOverflowELm16ENS2_10FastMallocEEEbi(reinterpret_cast<KeyAESPtr*>(storage), algorithm, std::move(keyData), extractable, usages);
    Require(result == reinterpret_cast<KeyAESPtr*>(storage));
    KeyAES* key = std::exchange(result->ptr, nullptr);
    result->~KeyAESPtr();
    return key;
}

void Release(KeyAES* key) {
    NKWebKit::CryptoAbi::Deref(key ? &key->base : nullptr);
}

void TestValidLengths() {
    for (unsigned length : {16u, 24u, 32u}) {
        std::string hex;
        for (unsigned i = 0; i < length; ++i) hex += "a5";
        ByteVector keyData = Bytes(hex);
        const unsigned char* buffer = keyData.data();
        KeyAES* key = Import(keyData, 9, false, 0x30);
        Require(key != nullptr);
        Require(keyData.size == 0 && keyData.data() == nullptr);
        Require(key->key.data() == buffer && Hex(key->key) == hex);
        Require(key->base.refCount == 1);
        Require(key->base.algorithmIdentifier == 9);
        Require(key->base.type == 2);
        Require(!key->base.extractable);
        Require(key->base.usages == 0x30);
        Require(reinterpret_cast<int(APS5_VABI*)(const KeyAES*)>(key->base.vtable[3])(key) == 0);
        Release(key);
    }
}

void TestInvalidLengths() {
    for (unsigned length : {0u, 1u, 15u, 17u, 20u, 31u, 33u, 64u}) {
        std::string hex;
        for (unsigned i = 0; i < length; ++i) hex += "00";
        ByteVector keyData = Bytes(hex);
        Require(Import(keyData) == nullptr);
        Require(keyData.size == length);
    }
}

void TestEncryptWithImportedKey() {
    ByteVector keyData = Bytes("2b7e151628aed2a6abf7158809cf4f3c");
    KeyAES* key = Import(keyData);
    Require(key != nullptr);
    AesCbcCfbParams parameters{};
    parameters.ivVector = Bytes("000102030405060708090a0b0c0d0e0f");
    ByteVector plainText = Bytes("6bc1bee22e409f96e93d7e117393172a");
    alignas(ByteVectorOrException) unsigned char storage[sizeof(ByteVectorOrException)];
    auto* result = _ZN7WebCore22CryptoAlgorithmAES_CBC15platformEncryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(reinterpret_cast<ByteVectorOrException*>(storage), parameters, *key, plainText, true);
    Require(result->hasValue);
    Require(Hex(result->value).substr(0, 32) == "7649abac8119b246cee98e9b12e9197d");
    result->~ByteVectorOrException();
    Release(key);
}

}  // namespace

int main() {
    TestValidLengths();
    TestInvalidLengths();
    TestEncryptWithImportedKey();
    return 0;
}
