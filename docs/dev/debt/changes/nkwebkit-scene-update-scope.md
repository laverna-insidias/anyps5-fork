# libSceNKWebKit

## add Functional

- [Nicosia::SceneIntegration::createUpdateScope](../../../core/libs/prx/libSceNKWebKit/src/NicosiaScene.cpp) (libSceNKWebKit) - not compared with a console module. Following the pinned [NicosiaSceneIntegration.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/platform/graphics/nicosia/NicosiaSceneIntegration.cpp), it takes the client lock, then returns a 16-byte scope holding a new reference to the integration and the lock. `UpdateScope`'s destructor is not exported, so nothing here flushes the scene's layers, asks the client for an update, or releases the lock and the reference; until a console comparison shows who does, the lock stays held. The lock spins instead of parking. A null result or `this` throws `std::invalid_argument`. Closes when: compared with a console module.
