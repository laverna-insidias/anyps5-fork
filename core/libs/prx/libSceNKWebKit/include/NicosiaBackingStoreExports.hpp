#pragma once

#include "SceTypes.hpp"
#include "prx/libSceNKWebKit/src/NicosiaBackingStoreAbi.hpp"

extern "C" {

void APS5_VABI _ZN7Nicosia29BackingStoreTextureMapperImpl11flushUpdateEv(NKWebKit::NicosiaAbi::BackingStoreTextureMapperImpl* self);
NKWebKit::NicosiaAbi::BackingStoreTileUpdate APS5_VABI _ZN7Nicosia29BackingStoreTextureMapperImpl10takeUpdateEv(NKWebKit::NicosiaAbi::BackingStoreTextureMapperImpl* self);

}
