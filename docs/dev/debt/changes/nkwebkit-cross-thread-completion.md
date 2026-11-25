# libSceNKWebKit

## add Functional

- [WTF::CrossThreadTaskHandler::setCompletionCallback](../../../core/libs/prx/libSceNKWebKit/src/CrossThreadTaskHandler.cpp) (libSceNKWebKit) - not compared with a console module. Following the pinned [CrossThreadTaskHandler.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WTF/wtf/CrossThreadTaskHandler.cpp), it moves the function into the handler's completion slot at offset 112 under the handler's lock and destroys the one it replaces, so an empty function clears the slot. Like the pinned source, replacing the callback while the worker thread is already running the old one is not guarded. A null `this`, a destroyed handler or a null function reference throws `std::invalid_argument`. Closes when: compared with a console module.
