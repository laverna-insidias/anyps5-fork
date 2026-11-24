# libSceNKWebKit

## add Functional

- [WebCore::OriginLock destructors](../../../core/libs/prx/libSceNKWebKit/src/OriginLock.cpp) (libSceNKWebKit) - not compared with a console module. Following the pinned [OriginLock.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/Modules/webdatabase/OriginLock.cpp), the defaulted destructor releases the lock file name `String` at offset 8; it does not unlock or close a lock file, as upstream does not. The layout is unverified. A null `this` throws `std::invalid_argument`. Closes when: compared with a console module.
