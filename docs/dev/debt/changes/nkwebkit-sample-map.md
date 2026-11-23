# libSceNKWebKit

## add Functional

- [WebCore::SampleMap, PresentationOrderSampleMap and DecodeOrderSampleMap](../../../core/libs/prx/libSceNKWebKit/src/SampleMap.cpp) (libSceNKWebKit) - not compared with a console module. The 20 exports follow the pinned [SampleMap.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/Modules/mediasource/SampleMap.cpp). Both `std::map`s are modelled on the libc++ red-black tree layout with this project's own insert and erase, nodes come from `fastMalloc`, and samples are held through `RefPtr` and released through their deleting destructor. The tree, node, `reverse_iterator` and `MediaSample` vtable slots are unverified. A null `this` or sample throws `std::invalid_argument`. Closes when: compared with a console module.
