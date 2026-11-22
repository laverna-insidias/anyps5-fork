# libSceNKWebKit

## add Functional

- [WebCore::CryptoKeyAES::importRaw](../../../core/libs/prx/libSceNKWebKit/src/CryptoKeyAes.cpp) (libSceNKWebKit) - not compared with a console module. It follows the pinned [CryptoKeyAES.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/crypto/keys/CryptoKeyAES.cpp#L78-L83): a 128, 192 or 256-bit key becomes a secret key that takes the key bytes, and any other length gives no key. The object uses the pinned `CryptoKeyAES` layout with a vtable of this library and no type info; its `algorithm()` throws. Closes when: compared with a console module.

## remove | `_ZN7WebCore12CryptoKeyAES9importRawENS_25CryptoAlgorithmIdentifierEON3WTF6VectorIhLm0ENS2_15CrashOnOverflowELm16ENS2_10FastMallocEEEbi` | `K6ZnlaOFL+0` |
