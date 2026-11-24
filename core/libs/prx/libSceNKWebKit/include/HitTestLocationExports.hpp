#pragma once

#include "SceTypes.hpp"
#include "prx/libSceNKWebKit/src/HitTestLocationAbi.hpp"

extern "C" {

void APS5_VABI _ZN7WebCore15HitTestLocationC1ERKNS_10FloatPointE(NKWebKit::WebCoreAbi::LegacyHitTestLocation* self, const WebCore::FloatPoint* point);
void APS5_VABI _ZN7WebCore15HitTestLocationC2ERKNS_10FloatPointE(NKWebKit::WebCoreAbi::LegacyHitTestLocation* self, const WebCore::FloatPoint* point);
void APS5_VABI _ZN7WebCore15HitTestLocationC1Ev(NKWebKit::WebCoreAbi::LegacyHitTestLocation* self);
void APS5_VABI _ZN7WebCore15HitTestLocationC2Ev(NKWebKit::WebCoreAbi::LegacyHitTestLocation* self);
void APS5_VABI _ZN7WebCore15HitTestLocationC1ERKS0_(NKWebKit::WebCoreAbi::LegacyHitTestLocation* self, const NKWebKit::WebCoreAbi::LegacyHitTestLocation* other);
void APS5_VABI _ZN7WebCore15HitTestLocationC2ERKS0_(NKWebKit::WebCoreAbi::LegacyHitTestLocation* self, const NKWebKit::WebCoreAbi::LegacyHitTestLocation* other);

}
