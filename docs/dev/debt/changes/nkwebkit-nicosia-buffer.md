# libSceNKWebKit

## add Functional

- [Nicosia::Buffer create, destructors and waitUntilPaintingComplete](../../../core/libs/prx/libSceNKWebKit/src/NicosiaBuffer.cpp) (libSceNKWebKit) - not compared with a console module. Following the pinned [NicosiaBuffer.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/platform/graphics/nicosia/NicosiaBuffer.cpp), `create` allocates a zeroed width x height x 4 pixel block and a 40-byte buffer with one reference, and the destructors free the pixels. A negative size or an area past the unsigned limit throws `std::overflow_error` where upstream crashes. `waitUntilPaintingComplete` spins on the painting lock and yields instead of parking on the condition, since nothing here parks. The layout is unverified. A null `this` throws `std::invalid_argument`. Closes when: compared with a console module.
