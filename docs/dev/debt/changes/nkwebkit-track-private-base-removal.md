# libSceNKWebKit

## add Functional

- [WebCore::TrackPrivateBase::willBeRemoved and its destructors](../../../core/libs/prx/libSceNKWebKit/src/TrackPrivateBase.cpp) (libSceNKWebKit) - not compared with a console module. Following the pinned [TrackPrivateBase.h](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/platform/graphics/TrackPrivateBase.h), `willBeRemoved` asks the track for its client through vtable slot 2 and calls the client's `willRemove`, slot 5 of the pinned `TrackPrivateBaseClient`, passing the track as a second argument in case the module's client takes it. The complete destructor leaves the 16-byte object alone and the deleting destructor frees it with `WTF::fastFree`. A null `this` throws `std::invalid_argument`. Closes when: compared with a console module.
