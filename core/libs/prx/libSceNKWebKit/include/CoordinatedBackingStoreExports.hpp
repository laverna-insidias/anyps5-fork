#pragma once

#include "SceTypes.hpp"
#include "prx/libSceNKWebKit/src/CoordinatedBackingStoreAbi.hpp"

#include <cstdint>

extern "C" {

void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreC1Ev(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreC2Ev(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreD0Ev(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreD1Ev(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreD2Ev(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self);
NKWebKit::CoordinatedAbi::CoordinatedBackingStoreRef APS5_VABI _ZN7WebCore23CoordinatedBackingStore6createEv();
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore10createTileEjf(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, std::uint32_t id, float scale);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore10removeTileEj(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, std::uint32_t id);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore14removeAllTilesEv(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore10updateTileEjRKNS_7IntRectES3_ON3WTF6RefPtrIN7Nicosia6BufferENS4_13DumbPtrTraitsIS7_EEEERKNS_8IntPointE(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, std::uint32_t id, const WebCore::IntRect* sourceRect, const WebCore::IntRect* tileRect, NKWebKit::CoordinatedAbi::BufferRefPtr* buffer, const WebCore::IntPoint* offset);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore7setSizeERKNS_9FloatSizeE(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, const WebCore::FloatSize* size);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore20commitTileOperationsERNS_13TextureMapperE(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, NKWebKit::CoordinatedAbi::TextureMapper* textureMapper);
WebCore::FloatRect APS5_VABI _ZNK7WebCore23CoordinatedBackingStore4rectEv(const NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self);
WebCore::TransformationMatrix APS5_VABI _ZN7WebCore23CoordinatedBackingStore24adjustedTransformForRectERKNS_9FloatRectE(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, const WebCore::FloatRect* targetRect);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore20paintToTextureMapperERNS_13TextureMapperERKNS_9FloatRectERKNS_20TransformationMatrixEf(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, NKWebKit::CoordinatedAbi::TextureMapper* textureMapper, const WebCore::FloatRect* targetRect, const WebCore::TransformationMatrix* transform, float opacity);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore25paintTilesToTextureMapperERN3WTF6VectorIPNS_17TextureMapperTileELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEERNS_13TextureMapperERKNS_20TransformationMatrixEfRKNS_9FloatRectE(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, NKWebKit::CoordinatedAbi::TilePointerVector* tiles, NKWebKit::CoordinatedAbi::TextureMapper* textureMapper, const WebCore::TransformationMatrix* transform, float opacity, const WebCore::FloatRect* rect);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore10drawBorderERNS_13TextureMapperERKNS_5ColorEfRKNS_9FloatRectERKNS_20TransformationMatrixE(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, NKWebKit::CoordinatedAbi::TextureMapper* textureMapper, const NKWebKit::CoordinatedAbi::Color* borderColor, float borderWidth, const WebCore::FloatRect* targetRect, const WebCore::TransformationMatrix* transform);
void APS5_VABI _ZN7WebCore23CoordinatedBackingStore18drawRepaintCounterERNS_13TextureMapperEiRKNS_5ColorERKNS_9FloatRectERKNS_20TransformationMatrixE(NKWebKit::CoordinatedAbi::CoordinatedBackingStore* self, NKWebKit::CoordinatedAbi::TextureMapper* textureMapper, int repaintCount, const NKWebKit::CoordinatedAbi::Color* borderColor, const WebCore::FloatRect* targetRect, const WebCore::TransformationMatrix* transform);

NKWebKit::CoordinatedAbi::BufferRef APS5_VABI _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(const WebCore::IntSize* size, unsigned flags);
void APS5_VABI _ZN7Nicosia6BufferD1Ev(NKWebKit::CoordinatedAbi::Buffer* self);
void APS5_VABI _ZN7Nicosia6BufferD2Ev(NKWebKit::CoordinatedAbi::Buffer* self);
void APS5_VABI _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(NKWebKit::CoordinatedAbi::Buffer* self);

}
