# libSceNKWebKit

## add Functional

- [WebCore::HitTestLocation default and copy constructors](../../../core/libs/prx/libSceNKWebKit/src/HitTestLocation.cpp) (libSceNKWebKit) - not compared with a console module. Following the pinned [HitTestLocation.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/rendering/HitTestLocation.cpp), the default constructor zeroes the point, bounding box, transformed point and quad and sets point-based and rectilinear, and the copy constructor copies every field, using the existing 68-byte layout with an integer bounding box. A null `this` or source throws `std::invalid_argument`. Closes when: compared with a console module.
