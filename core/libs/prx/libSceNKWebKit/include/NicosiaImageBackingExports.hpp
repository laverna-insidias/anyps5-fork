#pragma once

#include "SceTypes.hpp"
#include "prx/libSceNKWebKit/src/NicosiaImageBackingAbi.hpp"

extern "C" {

void APS5_VABI _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(NKWebKit::NicosiaAbi::ImageBackingTextureMapperImpl* self);
NKWebKit::NicosiaAbi::ImageBackingUpdate APS5_VABI _ZN7Nicosia29ImageBackingTextureMapperImpl10takeUpdateEv(NKWebKit::NicosiaAbi::ImageBackingTextureMapperImpl* self);

}
