# libSceNKWebKit

## add Functional

- [WebCore::CoordinatedBackingStore and Nicosia::Buffer](../../../core/libs/prx/libSceNKWebKit/src/CoordinatedBackingStore.cpp) (libSceNKWebKit) - not compared with a console module. Tile creation, removal, updates, pending size and the tile map follow the pinned [CoordinatedBackingStore.cpp](https://github.com/neonmodder123/PS5-WebKit-11.x/blob/80f77e94c63ee7202eec3d4b5b1a021dd89011ba/Source/WebCore/platform/graphics/texmap/coordinated/CoordinatedBackingStore.cpp), with `HashMap<uint32_t, CoordinatedBackingStoreTile>` and `HashSet<uint32_t>` modelled on WTF's table layout. The layouts are unverified. Uploading a buffered tile update, painting or releasing textured tiles, `drawBorder` and `drawRepaintCounter` with tiles, and waiting on a buffer still being painted throw, because `TextureMapper` and `BitmapTexture` are not implemented. Reserved tile IDs and updates to a missing tile throw instead of reaching the hash table assertions. Closes when: compared with a console module.

## remove | `_ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj` | `MfJ3R1TdXCY` |
## remove | `_ZN7Nicosia6BufferD1Ev` | `VKOM+R2KVYk` |
## remove | `_ZN7Nicosia6BufferD2Ev` | `8mn-7ut9drU` |
