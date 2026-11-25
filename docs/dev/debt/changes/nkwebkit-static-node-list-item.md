# libSceNKWebKit

## add Functional

- [WebCore::StaticNodeList::item](../../../core/libs/prx/libSceNKWebKit/src/StaticNodeList.cpp) (libSceNKWebKit) - not compared with a console module. Following the pinned [StaticNodeList.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/dom/StaticNodeList.cpp), it returns the node at the index from the list's node vector, read at offset 24 with its size at offset 36 as the existing `length` getter reads it, and null past the end. A null `this` throws `std::invalid_argument`. Closes when: compared with a console module.
