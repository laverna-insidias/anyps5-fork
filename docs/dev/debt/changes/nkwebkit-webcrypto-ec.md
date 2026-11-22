# libSceNKWebKit

## add Functional

- [WebCore::CryptoKeyEC importRaw, exportRaw, generatePair and CryptoAlgorithmECDH::platformDeriveBits](../../../core/libs/prx/libSceNKWebKit/src/CryptoKeyEc.cpp) (libSceNKWebKit) - not compared with a console module. They follow the pinned [CryptoKeyEC.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/crypto/keys/CryptoKeyEC.cpp) and its OpenSSL backend for P-256, P-384 and P-521 on libSceLibreSsl. The key holds a libSceLibreSsl `EC_KEY` where the console holds an `EVP_PKEY`, so only these exports can use it; one built elsewhere throws. Its vtable has no type info and `algorithm()` throws. Layouts are unverified. Closes when: compared with a console module.

## remove | `_ZN7WebCore11CryptoKeyEC12generatePairENS_25CryptoAlgorithmIdentifierERKN3WTF6StringEbi` | `UvU6Uh2YH-s` |

## remove | `_ZN7WebCore11CryptoKeyEC9importRawENS_25CryptoAlgorithmIdentifierERKN3WTF6StringEONS2_6VectorIhLm0ENS2_15CrashOnOverflowELm16ENS2_10FastMallocEEEbi` | `9b-5iZGs0Fc` |
