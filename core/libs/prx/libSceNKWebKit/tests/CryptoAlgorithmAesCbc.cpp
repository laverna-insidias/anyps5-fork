#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>

namespace {

using NKWebKit::CryptoAbi::AesCbcCfbParams;
using NKWebKit::CryptoAbi::ByteVector;
using NKWebKit::CryptoAbi::ByteVectorOrException;
using NKWebKit::CryptoAbi::KeyAES;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "CryptoAlgorithmAES_CBC check failed at line %d\n", line);
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

struct Result {
    alignas(ByteVectorOrException) unsigned char storage[sizeof(ByteVectorOrException)];
    ByteVectorOrException& get() { return *reinterpret_cast<ByteVectorOrException*>(storage); }
    ~Result() { get().~ByteVectorOrException(); }
};

AesCbcCfbParams Params(const std::string& iv) {
    AesCbcCfbParams parameters{};
    parameters.ivVector = Bytes(iv);
    return parameters;
}

KeyAES Key(const std::string& key) {
    KeyAES value{};
    value.key = Bytes(key);
    return value;
}

std::string Encrypt(const std::string& key, const std::string& iv, const std::string& plainText) {
    Result result;
    auto parameters = Params(iv);
    auto keyValue = Key(key);
    auto input = Bytes(plainText);
    Require(_ZN7WebCore22CryptoAlgorithmAES_CBC15platformEncryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(&result.get(), parameters, keyValue, input, true) == &result.get());
    if (!result.get().hasValue) {
        Require(result.get().exception.code == 30);
        Require(!result.get().exception.message.impl);
        return "error";
    }
    return Hex(result.get().value);
}

std::string Decrypt(const std::string& key, const std::string& iv, const std::string& cipherText) {
    Result result;
    auto parameters = Params(iv);
    auto keyValue = Key(key);
    auto input = Bytes(cipherText);
    _ZN7WebCore22CryptoAlgorithmAES_CBC15platformDecryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(&result.get(), parameters, keyValue, input, false);
    if (!result.get().hasValue) {
        Require(result.get().exception.code == 30);
        return "error";
    }
    return Hex(result.get().value);
}

const std::string kKey128 = "2b7e151628aed2a6abf7158809cf4f3c";
const std::string kKey192 = "8e73b0f7da0e6452c810f32b809079e562f8ead2522c6b7b";
const std::string kKey256 = "603deb1015ca71be2b73aef0857d77811f352c073b6108d72d9810a30914dff4";
const std::string kIv = "000102030405060708090a0b0c0d0e0f";
const std::string kPlain = "6bc1bee22e409f96e93d7e117393172aae2d8a571e03ac9c9eb76fac45af8e51";
const std::string kCipher = "7649abac8119b246cee98e9b12e9197d5086cb9b507219ee95db113a917678b255e21d7100b988ffec32feeafaf23538";

void TestEncrypt() {
    Require(Encrypt(kKey128, kIv, kPlain) == kCipher);
    Require(Encrypt(kKey192, kIv, "616263") == "897770a5b9f82ee7b9007fec128538f7");
    Require(Encrypt(kKey256, "", "") == "4c45dfb3b3b484ec35b0512dc8c1c4d6");
    Require(Encrypt("00112233445566778899aabbccddee", kIv, kPlain) == "error");
    Require(Encrypt("", kIv, kPlain) == "error");
}

void TestDecrypt() {
    Require(Decrypt(kKey128, kIv, kCipher) == kPlain);
    Require(Decrypt(kKey192, kIv, "897770a5b9f82ee7b9007fec128538f7") == "616263");
    Require(Decrypt(kKey256, "", "4c45dfb3b3b484ec35b0512dc8c1c4d6") == "");
    Require(Decrypt(kKey128, kIv, kCipher.substr(0, 64)) == "error");
    Require(Decrypt(kKey128, kIv, kCipher.substr(0, 94)) == "error");
    Require(Decrypt(kKey128, kIv, "") == "error");
    Require(Decrypt(kKey256, kIv, kCipher) == "error");
}

void TestRoundTrip() {
    std::string plain;
    for (unsigned length = 0; length <= 48; ++length) {
        const std::string cipher = Encrypt(kKey256, kIv, plain);
        Require(cipher.size() == ((length / 16) + 1) * 32);
        Require(Decrypt(kKey256, kIv, cipher) == plain);
        static const char digits[] = "0123456789abcdef";
        plain += digits[length % 16];
        plain += digits[(length * 7) % 16];
    }
}

void TestShortIv() {
    Result result;
    auto parameters = Params("0001020304050607");
    auto keyValue = Key(kKey128);
    auto input = Bytes(kPlain);
    bool threw = false;
    try {
        _ZN7WebCore22CryptoAlgorithmAES_CBC15platformEncryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(&result.get(), parameters, keyValue, input, true);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
    new (&result.get()) ByteVectorOrException(0);
}

void TestIvFromJavaScriptBuffer() {
    Result result;
    AesCbcCfbParams parameters{};
    int buffer = 0;
    parameters.iv.object = &buffer;
    auto keyValue = Key(kKey128);
    auto input = Bytes(kPlain);
    bool threw = false;
    try {
        _ZN7WebCore22CryptoAlgorithmAES_CBC15platformEncryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(&result.get(), parameters, keyValue, input, true);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    Require(threw);
    new (&result.get()) ByteVectorOrException(0);
}

}  // namespace

int main() {
    TestEncrypt();
    TestDecrypt();
    TestRoundTrip();
    TestShortIv();
    TestIvFromJavaScriptBuffer();
    return 0;
}
