# libSceNKWebKit

## add Functional

- [WebCore::CryptoAlgorithmAES_CBC::platformEncrypt and platformDecrypt](../../../core/libs/prx/libSceNKWebKit/src/CryptoAlgorithmAesCbc.cpp) (libSceNKWebKit) - not compared with a console module. They follow the pinned [CryptoAlgorithmAES_CBCOpenSSL.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/crypto/openssl/CryptoAlgorithmAES_CBCOpenSSL.cpp#L38-L140): PKCS#7-padded AES-CBC whatever the padding argument, and `OperationError` (30) for a key that is not 128, 192 or 256 bits or for bad padding. The cipher runs on libSceLibreSsl `AES_cbc_encrypt`, whose `EVP_aes_*_cbc` are CMAC-only. The key, parameter and `ExceptionOr` layouts follow the pinned headers and are unverified. An empty iv is all zeros; one shorter than 16 bytes throws. An iv still held in a JavaScript buffer throws through `NotImplemented_nid_no_patch`. Closes when: compared with a console module.

## remove | `_ZN7WebCore22CryptoAlgorithmAES_CBC15platformDecryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE` | `lC6NyPIlPns` |

## remove | `_ZN7WebCore22CryptoAlgorithmAES_CBC15platformEncryptERKNS_30CryptoAlgorithmAesCbcCfbParamsERKNS_12CryptoKeyAESERKN3WTF6VectorIhLm0ENS7_15CrashOnOverflowELm16ENS7_10FastMallocEEENS0_7PaddingE` | `D4Jb9-xqqBk` |
