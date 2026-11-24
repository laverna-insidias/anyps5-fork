# libSceNKWebKit

## add Functional

- [WebCore::FrameLoaderStateMachine getters](../../../core/libs/prx/libSceNKWebKit/src/FrameLoaderStateMachine.cpp) (libSceNKWebKit) - not compared with a console module. `committingFirstRealLoad`, `creatingInitialEmptyDocument`, `isDisplayingInitialEmptyDocument` and `firstLayoutDone` follow the pinned [FrameLoaderStateMachine.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/loader/FrameLoaderStateMachine.cpp) and read the `State` enum at offset 0; the enum values are unverified. A null `this` throws `std::invalid_argument`. Closes when: compared with a console module.
