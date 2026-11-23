# libSceNKWebKit

## add Functional

- [Nicosia::ImageBackingTextureMapperImpl update hand-off](../../../core/libs/prx/libSceNKWebKit/src/NicosiaImageBackingTextureMapperImpl.cpp) (libSceNKWebKit) - not compared with a console module. `flushUpdate` and `takeUpdate` follow the pinned [NicosiaImageBackingTextureMapperImpl.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/platform/graphics/nicosia/texmap/NicosiaImageBackingTextureMapperImpl.cpp): a flush for the same image without a new store only copies visibility, otherwise the layer update moves to the pending one, and a take moves the pending store out. A replaced `ImageBackingStore` is released down to its `Nicosia::Buffer` and `CoordinatedBackingStore`. The layouts are unverified. Closes when: compared with a console module.
