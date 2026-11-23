# libSceNKWebKit

## add Functional

- [Nicosia::BackingStoreTextureMapperImpl tile update hand-off](../../../core/libs/prx/libSceNKWebKit/src/NicosiaBackingStoreTextureMapperImpl.cpp) (libSceNKWebKit) - not compared with a console module. `flushUpdate` and `takeUpdate` follow the pinned [NicosiaBackingStoreTextureMapperImpl.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/platform/graphics/nicosia/texmap/NicosiaBackingStoreTextureMapperImpl.cpp): a flush appends the layer's tile creations, updates and removals to the pending update with WTF `Vector` growth, references each update's `Nicosia::Buffer`, then empties the layer update, and a take moves the pending update out. The layouts are unverified. Closes when: compared with a console module.
