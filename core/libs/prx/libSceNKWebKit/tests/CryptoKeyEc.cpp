#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

using NKWebKit::CryptoAbi::ByteVector;
using NKWebKit::CryptoAbi::ByteVectorOrException;
using NKWebKit::CryptoAbi::Key;
using NKWebKit::CryptoAbi::KeyEC;
using NKWebKit::CryptoAbi::KeyECPtr;
using NKWebKit::CryptoAbi::KeyPairOrException;
using NKWebKit::CryptoAbi::OptionalByteVector;
using NKWebKit::WtfAbi::String;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "CryptoKeyEc check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(value) Check((value), __LINE__)

template <typename T>
struct Result {
    alignas(T) unsigned char storage[sizeof(T)];
    bool constructed = false;
    T* place() { return reinterpret_cast<T*>(storage); }
    T& get() { return *reinterpret_cast<T*>(storage); }
    ~Result() {
        if (constructed) get().~T();
    }
};

String Make(const char* text) {
    return _ZN3WTF10StringImpl6createEPKhj(reinterpret_cast<const unsigned char*>(text), static_cast<unsigned>(std::strlen(text)));
}

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

KeyEC* Import(const char* curve, const std::string& hex, int identifier = 6, bool extractable = true, int usages = 0) {
    Result<KeyECPtr> result;
    String name = Make(curve);
    ByteVector data = Bytes(hex);
    _ZN7WebCore11CryptoKeyEC9importRawENS_25CryptoAlgorithmIdentifierERKN3WTF6StringEONS2_6VectorIhLm0ENS2_15CrashOnOverflowELm16ENS2_10FastMallocEEEbi(result.place(), identifier, name, std::move(data), extractable, usages);
    result.constructed = true;
    return std::exchange(result.get().ptr, nullptr);
}

std::string Export(const KeyEC* key, int* error = nullptr) {
    Result<ByteVectorOrException> result;
    _ZNK7WebCore11CryptoKeyEC9exportRawEv(result.place(), key);
    result.constructed = true;
    if (!result.get().hasValue) {
        if (error) *error = result.get().exception.code;
        return "error";
    }
    return Hex(result.get().value);
}

std::string Derive(const KeyEC& base, const KeyEC& peer) {
    Result<OptionalByteVector> result;
    _ZN7WebCore19CryptoAlgorithmECDH18platformDeriveBitsERKNS_11CryptoKeyECES3_(result.place(), base, peer);
    result.constructed = true;
    return result.get().hasValue ? Hex(result.get().value) : "none";
}

int KeyClass(const KeyEC* key) {
    return reinterpret_cast<int(APS5_VABI*)(const KeyEC*)>(key->base.vtable[3])(key);
}

void Release(KeyEC* key) {
    NKWebKit::CryptoAbi::Deref(key ? &key->base : nullptr);
}

const std::string kP256Uncompressed =
    "04dad0b65394221cf9b051e1feca5787d098dfe637fc90b9ef945d0c3772581180"
    "5271a0461cdb8252d61f1c456fa3e59ab1f45b33accf5f58389e0577b8990bb3";
const std::string kP256Compressed = "03dad0b65394221cf9b051e1feca5787d098dfe637fc90b9ef945d0c3772581180";

void TestImportExport() {
    KeyEC* key = Import("P-256", kP256Uncompressed, 6, false, 0x20);
    Require(key != nullptr);
    Require(key->base.refCount == 1);
    Require(key->base.algorithmIdentifier == 6);
    Require(key->base.type == 0);
    Require(!key->base.extractable);
    Require(key->base.usages == 0x20);
    Require(key->curve == 0);
    Require(KeyClass(key) == 1);
    Require(Export(key) == kP256Uncompressed);
    Release(key);

    key = Import("P-256", kP256Compressed);
    Require(key != nullptr);
    Require(Export(key) == kP256Uncompressed);
    Release(key);
}

void TestImportRejects() {
    Require(Import("P-256", "") == nullptr);
    Require(Import("P-256", kP256Uncompressed.substr(0, 128)) == nullptr);
    std::string offCurve = kP256Uncompressed;
    offCurve.back() = offCurve.back() == '3' ? '4' : '3';
    Require(Import("P-256", offCurve) == nullptr);
    Require(Import("P-384", kP256Uncompressed) == nullptr);
    Require(Import("p-256", kP256Uncompressed) == nullptr);
    Require(Import("P-256 ", kP256Uncompressed) == nullptr);
    Require(Import("K-256", kP256Uncompressed) == nullptr);
    Require(Import("", kP256Uncompressed) == nullptr);
}

KeyPairOrException* Generate(Result<KeyPairOrException>& result, const char* curve, bool extractable = false, int usages = 0x30) {
    String name = Make(curve);
    KeyPairOrException* returned = _ZN7WebCore11CryptoKeyEC12generatePairENS_25CryptoAlgorithmIdentifierERKN3WTF6StringEbi(result.place(), 6, name, extractable, usages);
    result.constructed = true;
    return returned;
}

void TestGenerateAndDerive(const char* curve, int index, unsigned fieldSize) {
    Result<KeyPairOrException> first;
    Result<KeyPairOrException> second;
    Require(Generate(first, curve) == &first.get());
    Generate(second, curve);
    Require(first.get().hasValue && second.get().hasValue);
    auto* firstPublic = reinterpret_cast<KeyEC*>(first.get().value.publicKey);
    auto* firstPrivate = reinterpret_cast<KeyEC*>(first.get().value.privateKey);
    auto* secondPublic = reinterpret_cast<KeyEC*>(second.get().value.publicKey);
    auto* secondPrivate = reinterpret_cast<KeyEC*>(second.get().value.privateKey);

    Require(firstPublic->base.type == 0 && firstPrivate->base.type == 1);
    Require(firstPublic->base.extractable && !firstPrivate->base.extractable);
    Require(firstPublic->base.usages == 0x30 && firstPrivate->base.usages == 0x30);
    Require(firstPublic->curve == index && firstPrivate->curve == index);

    const std::string exported = Export(firstPublic);
    Require(exported.size() == (1 + 2 * fieldSize) * 2);
    Require(exported.substr(0, 2) == "04");
    int error = 0;
    Require(Export(firstPrivate, &error) == "error" && error == 12);

    KeyEC* reimported = Import(curve, exported);
    Require(reimported != nullptr && Export(reimported) == exported);

    const std::string shared = Derive(*firstPrivate, *secondPublic);
    Require(shared.size() == fieldSize * 2);
    Require(shared == Derive(*secondPrivate, *firstPublic));
    Require(shared == Derive(*secondPrivate, *reimported));
    Require(shared != Derive(*firstPrivate, *firstPublic));
    Require(Derive(*firstPublic, *secondPublic) == "none");
    Release(reimported);
}

void TestDeriveAcrossCurves() {
    Result<KeyPairOrException> p256;
    Result<KeyPairOrException> p384;
    Generate(p256, "P-256");
    Generate(p384, "P-384");
    auto* privateKey = reinterpret_cast<KeyEC*>(p256.get().value.privateKey);
    auto* publicKey = reinterpret_cast<KeyEC*>(p384.get().value.publicKey);
    Require(Derive(*privateKey, *publicKey) == "none");
}

void TestGenerateUnsupportedCurve() {
    Result<KeyPairOrException> result;
    Generate(result, "P-224");
    Require(!result.get().hasValue);
    Require(result.get().exception.code == 6);
    Require(!result.get().exception.message.impl);
}

void TestForeignKey() {
    KeyEC foreign{};
    const void* const vtable[4] = {};
    foreign.base.vtable = &vtable[2];
    bool threw = false;
    try {
        Export(&foreign);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    Require(threw);
}

}  // namespace

int main() {
    TestImportExport();
    TestImportRejects();
    TestGenerateAndDerive("P-256", 0, 32);
    TestGenerateAndDerive("P-384", 1, 48);
    TestGenerateAndDerive("P-521", 2, 66);
    TestDeriveAcrossCurves();
    TestGenerateUnsupportedCurve();
    TestForeignKey();
    return 0;
}
