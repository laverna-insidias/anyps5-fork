# libSceNKWebKit

## add Functional

- [Nicosia::ContentLayerTextureMapperImpl flushUpdate and swapBuffersIfNeeded](../../../core/libs/prx/libSceNKWebKit/src/NicosiaContentLayerTextureMapperImpl.cpp) (libSceNKWebKit) - not compared with a console module. Both follow the pinned [NicosiaContentLayerTextureMapperImpl.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/platform/graphics/nicosia/texmap/NicosiaContentLayerTextureMapperImpl.cpp): `flushUpdate` takes and clears the pending-update flag under the client lock, and `swapBuffersIfNeeded` calls the client's `swapBuffersIfNeeded` through vtable slot 2 under that lock. The 40-byte layout and the slot are unverified. A null `this` throws `std::invalid_argument`. Closes when: compared with a console module.
