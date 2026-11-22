// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/crypto/openssl/CryptoAlgorithmAES_CBCOpenSSL.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
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
#include "prx/libSceNKWebKit/include/CryptoAlgorithmAesCbcExports.hpp"

#include <cstddef>
#include <cstring>
#include <optional>
#include <stdexcept>

namespace {

using NKWebKit::CryptoAbi::AesCbcCfbParams;
using NKWebKit::CryptoAbi::ByteVector;
using NKWebKit::CryptoAbi::ByteVectorOrException;
using NKWebKit::CryptoAbi::KeyAES;

constexpr unsigned kBlockSize = 16;
constexpr int kOperationError = 30;

bool IsAesKeySize(unsigned size) {
    return size * 8 == 128 || size * 8 == 192 || size * 8 == 256;
}

const ByteVector& IvVector(const AesCbcCfbParams& parameters) {
    if (parameters.ivVector.size || !parameters.iv.object) {
        return parameters.ivVector;
    }
    NotImplemented_nid_no_patch("CryptoAlgorithmAesCbcCfbParams::ivVector: copying the iv from a JavaScript buffer");
}

void LoadIv(const ByteVector& iv, unsigned char (&block)[kBlockSize]) {
    std::memset(block, 0, sizeof(block));
    if (!iv.size) {
        return;
    }
    if (iv.size < kBlockSize) {
        throw std::invalid_argument("CryptoAlgorithmAES_CBC: the iv is shorter than one block");
    }
    std::memcpy(block, iv.data(), kBlockSize);
}

ByteVector Allocate(unsigned size) {
    ByteVector bytes(size);
    bytes.size = size;
    return bytes;
}

std::optional<ByteVector> CryptEncrypt(const ByteVector& key, const ByteVector& ivVector, const ByteVector& plainText) {
    if (!IsAesKeySize(key.size)) {
        return std::nullopt;
    }
    AesKey schedule;
    if (AES_set_encrypt_key_nid_postfix(key.data(), static_cast<int>(key.size * 8), &schedule)) {
        return std::nullopt;
    }
    unsigned char iv[kBlockSize];
    LoadIv(ivVector, iv);
    unsigned plainSize = plainText.size;
    unsigned cipherTextLen = (plainSize + 1 + kBlockSize - 1) / kBlockSize * kBlockSize;
    ByteVector padded = Allocate(cipherTextLen);
    if (plainSize) {
        std::memcpy(padded.data(), plainText.data(), plainSize);
    }
    std::memset(padded.data() + plainSize, static_cast<int>(cipherTextLen - plainSize), cipherTextLen - plainSize);
    ByteVector cipherText = Allocate(cipherTextLen);
    AES_cbc_encrypt_nid_postfix(padded.data(), cipherText.data(), cipherTextLen, &schedule, iv, 1);
    return cipherText;
}

std::optional<ByteVector> CryptDecrypt(const ByteVector& key, const ByteVector& ivVector, const ByteVector& cipherText) {
    if (!IsAesKeySize(key.size)) {
        return std::nullopt;
    }
    unsigned cipherSize = cipherText.size;
    if (!cipherSize || cipherSize % kBlockSize) {
        return std::nullopt;
    }
    AesKey schedule;
    if (AES_set_decrypt_key_nid_postfix(key.data(), static_cast<int>(key.size * 8), &schedule)) {
        return std::nullopt;
    }
    unsigned char iv[kBlockSize];
    LoadIv(ivVector, iv);
    ByteVector plainText = Allocate(cipherSize);
    AES_cbc_encrypt_nid_postfix(cipherText.data(), plainText.data(), cipherSize, &schedule, iv, 0);
    unsigned pad = plainText[cipherSize - 1];
    if (!pad || pad > kBlockSize) {
        return std::nullopt;
    }
    for (unsigned i = cipherSize - pad; i < cipherSize; ++i) {
        if (plainText[i] != pad) {
            return std::nullopt;
        }
    }
    plainText.size = cipherSize - pad;
    return plainText;
}

ByteVectorOrException* Finish(ByteVectorOrException* result, std::optional<ByteVector>&& output) {
    if (!output) {
        return new (result) ByteVectorOrException(kOperationError);
    }
    return new (result) ByteVectorOrException(std::move(*output));
}

}  // namespace

extern "C" {

ByteVectorOrException* APS5_VABI _ZN7WebCore22CryptoAlgorithmAES_CBC15platformEncryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(ByteVectorOrException* result, const AesCbcCfbParams& parameters, const KeyAES& key, const ByteVector& plainText, bool) {
    return Finish(result, CryptEncrypt(key.key, IvVector(parameters), plainText));
}

ByteVectorOrException* APS5_VABI _ZN7WebCore22CryptoAlgorithmAES_CBC15platformDecryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE(ByteVectorOrException* result, const AesCbcCfbParams& parameters, const KeyAES& key, const ByteVector& cipherText, bool) {
    return Finish(result, CryptDecrypt(key.key, IvVector(parameters), cipherText));
}

}
