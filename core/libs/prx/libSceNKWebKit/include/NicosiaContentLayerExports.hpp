#pragma once

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/src/NicosiaContentLayerAbi.hpp"

extern "C" {

bool APS5_VABI _ZN7Nicosia29ContentLayerTextureMapperImpl11flushUpdateEv(NKWebKit::NicosiaContentLayerAbi::ContentLayerTextureMapperImpl* self);
void APS5_VABI _ZN7Nicosia29ContentLayerTextureMapperImpl19swapBuffersIfNeededEv(NKWebKit::NicosiaContentLayerAbi::ContentLayerTextureMapperImpl* self);

}
