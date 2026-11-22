// Derived from Source/WebCore/platform/graphics/texmap/coordinated/CoordinatedBackingStore.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

// Derived from Source/WebCore/platform/graphics/nicosia/NicosiaBuffer.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2017 Metrological Group B.V.
 * Copyright (C) 2017 Igalia S.L.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "prx/libc/include/General.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/CoordinatedBackingStoreExports.hpp"
#include "prx/libSceNKWebKit/include/InventoryExports.hpp"
#include "prx/libSceNKWebKit/include/WebCoreExports.hpp"
#include "prx/libSceNKWebKit/include/WtfExports.hpp"
#include "prx/libSceNKWebKit/src/WtfIntHashTable.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstdlib>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

namespace HashTableAbi = NKWebKit::WtfAbi::HashTableAbi;
using NKWebKit::CoordinatedAbi::Buffer;
using NKWebKit::CoordinatedAbi::BufferRef;
using NKWebKit::CoordinatedAbi::BufferRefPtr;
using NKWebKit::CoordinatedAbi::CoordinatedBackingStore;
using NKWebKit::CoordinatedAbi::CoordinatedBackingStoreRef;
using NKWebKit::CoordinatedAbi::PaintingComplete;
using NKWebKit::CoordinatedAbi::TextureMapper;
using NKWebKit::CoordinatedAbi::TextureMapperTile;
using NKWebKit::CoordinatedAbi::Tile;
using NKWebKit::CoordinatedAbi::TileBucket;
using NKWebKit::CoordinatedAbi::TileMap;
using NKWebKit::CoordinatedAbi::TilePointerVector;
using NKWebKit::CoordinatedAbi::TileUpdate;

constexpr unsigned EmptyKey = 0;
constexpr unsigned DeletedKey = ~0U;
constexpr unsigned VectorMinCapacity = 16;

[[noreturn]] void Invalid(const char* caller, const char* what) {
    throw std::invalid_argument(std::string("WebCore::CoordinatedBackingStore::") + caller + ": " + what);
}

CoordinatedBackingStore& Checked(CoordinatedBackingStore* self, const char* caller) {
    if (!self) Invalid(caller, "null this");
    return *self;
}

const CoordinatedBackingStore& Checked(const CoordinatedBackingStore* self, const char* caller) {
    if (!self) Invalid(caller, "null this");
    return *self;
}

template<typename T>
const T& CheckedArgument(const T* value, const char* caller, const char* name) {
    if (!value) Invalid(caller, (std::string("null ") + name).c_str());
    return *value;
}

void CheckedKey(std::uint32_t id, const char* caller) {
    if (id == EmptyKey || id == DeletedKey) Invalid(caller, "tile ID is a reserved hash table value");
}

void DerefBuffer(Buffer* buffer) {
    if (buffer && std::atomic_ref<unsigned>(buffer->refCount).fetch_sub(1) == 1) {
        _ZN7Nicosia6BufferD1Ev(buffer);
        _ZN3WTF8fastFreeEPv(buffer);
    }
}

void APS5_VABI TileDestroy(Tile* tile);
void APS5_VABI TileDestroyAndFree(Tile* tile);

const void* const TileVtable[] = {
    nullptr,
    nullptr,
    reinterpret_cast<const void*>(&_ZN7WebCore17TextureMapperTile5paintERNS_13TextureMapperERKNS_20TransformationMatrixEfj),
    reinterpret_cast<const void*>(&TileDestroy),
    reinterpret_cast<const void*>(&TileDestroyAndFree),
};

void ConstructTile(Tile* tile, float scale) {
    tile->base.vtable = &TileVtable[2];
    tile->base.texture = nullptr;
    tile->base.rect = WebCore::FloatRect{};
    tile->updates = nullptr;
    tile->updateCapacity = 0;
    tile->updateCount = 0;
    tile->scale = scale;
}

void ClearUpdates(Tile& tile) {
    TileUpdate* updates = std::exchange(tile.updates, nullptr);
    const unsigned count = std::exchange(tile.updateCount, 0U);
    tile.updateCapacity = 0;
    for (unsigned i = 0; i < count; ++i) DerefBuffer(std::exchange(updates[i].buffer.ptr, nullptr));
    if (updates) _ZN3WTF8fastFreeEPv(updates);
}

void APS5_VABI TileDestroy(Tile* tile) {
    ClearUpdates(*tile);
}

void APS5_VABI TileDestroyAndFree(Tile* tile) {
    TileDestroy(tile);
    _ZN3WTF8fastFreeEPv(tile);
}

void AppendUpdate(Tile& tile, TileUpdate&& update) {
    if (tile.updateCount == tile.updateCapacity) {
        const std::uint64_t grown = std::uint64_t(tile.updateCapacity) + tile.updateCapacity / 4 + 1;
        const std::uint64_t capacity = std::max<std::uint64_t>(std::max<std::uint64_t>(tile.updateCount + 1ULL, VectorMinCapacity), grown);
        if (capacity > 0xffffffffULL / sizeof(TileUpdate)) throw std::length_error("WTF::Vector<CoordinatedBackingStoreTile::Update>: capacity overflow");
        auto* updates = static_cast<TileUpdate*>(_ZN3WTF10fastMallocEm(capacity * sizeof(TileUpdate)));
        for (unsigned i = 0; i < tile.updateCount; ++i) updates[i] = tile.updates[i];
        if (tile.updates) _ZN3WTF8fastFreeEPv(tile.updates);
        tile.updates = updates;
        tile.updateCapacity = static_cast<unsigned>(capacity);
    }
    tile.updates[tile.updateCount++] = update;
}

struct TilePolicy {
    using Bucket = TileBucket;
    using Key = unsigned;
    static constexpr bool EmptyValueIsZero = false;
    static unsigned KeyOf(Bucket& bucket) { return bucket.key; }
    static unsigned Hash(unsigned key) { return HashTableAbi::IntHash(key); }
    static bool IsEmpty(Bucket& bucket) { return bucket.key == EmptyKey; }
    static bool IsDeleted(Bucket& bucket) { return bucket.key == DeletedKey; }
    static void SetDeleted(Bucket& bucket) { bucket.key = DeletedKey; }
    static void ConstructEmpty(Bucket& bucket) {
        bucket.key = EmptyKey;
        ConstructTile(&bucket.value, 1.0f);
    }
    static void DestroyValue(Bucket& bucket) { TileDestroy(&bucket.value); }
    static void Move(Bucket& from, Bucket& to) {
        DestroyValue(to);
        to.key = from.key;
        to.value = from.value;
        from.value.base.texture = nullptr;
        from.value.updates = nullptr;
        from.value.updateCapacity = 0;
        from.value.updateCount = 0;
    }
};

struct IdPolicy {
    using Bucket = unsigned;
    using Key = unsigned;
    static constexpr bool EmptyValueIsZero = true;
    static unsigned KeyOf(Bucket& bucket) { return bucket; }
    static unsigned Hash(unsigned key) { return HashTableAbi::IntHash(key); }
    static bool IsEmpty(Bucket& bucket) { return bucket == EmptyKey; }
    static bool IsDeleted(Bucket& bucket) { return bucket == DeletedKey; }
    static void SetDeleted(Bucket& bucket) { bucket = DeletedKey; }
    static void ConstructEmpty(Bucket& bucket) { bucket = EmptyKey; }
    static void DestroyValue(Bucket&) {}
    static void Move(Bucket& from, Bucket& to) { to = from; }
};

bool HasTexture(TileMap& tiles) {
    bool found = false;
    HashTableAbi::ForEachLive<TilePolicy>(tiles.table, [&](TileBucket& bucket) { found = found || bucket.value.base.texture; });
    return found;
}

const void* const Vtable[] = {
    nullptr,
    nullptr,
    reinterpret_cast<const void*>(&_ZN7WebCore23CoordinatedBackingStoreD1Ev),
    reinterpret_cast<const void*>(&_ZN7WebCore23CoordinatedBackingStoreD0Ev),
    reinterpret_cast<const void*>(&_ZN7WebCore23CoordinatedBackingStore20paintToTextureMapperERNS_13TextureMapperERKNS_9FloatRectERKNS_20TransformationMatrixEf),
    reinterpret_cast<const void*>(&_ZN7WebCore23CoordinatedBackingStore10drawBorderERNS_13TextureMapperERKNS_5ColorEfRKNS_9FloatRectERKNS_20TransformationMatrixE),
    reinterpret_cast<const void*>(&_ZN7WebCore23CoordinatedBackingStore18drawRepaintCounterERNS_13TextureMapperEiRKNS_5ColorERKNS_9FloatRectERKNS_20TransformationMatrixE),
};

void Construct(CoordinatedBackingStore* self) {
    auto& object = Checked(self, "CoordinatedBackingStore");
    object.vtable = &Vtable[2];
    object.client = nullptr;
    object.refCount = 1;
    object.tiles.table = nullptr;
    object.tilesToRemove.table = nullptr;
    object.pendingSize = WebCore::FloatSize{};
    object.size = WebCore::FloatSize{};
    object.scale = 1.0f;
}

WebCore::FloatRect Rect(const CoordinatedBackingStore& object) {
    return WebCore::FloatRect{WebCore::FloatPoint{}, object.size};
}

bool IsZero(const WebCore::FloatSize& size) {
    return !size.m_width && !size.m_height;
}

using TilePaint = void(APS5_VABI*)(TextureMapperTile*, TextureMapper*, const WebCore::TransformationMatrix*, float, unsigned);

}  // namespace

extern "C" {

void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreC1Ev(CoordinatedBackingStore* self) {
    Construct(self);
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreC2Ev(CoordinatedBackingStore* self) {
    Construct(self);
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreD1Ev(CoordinatedBackingStore* self) {
    auto& object = Checked(self, "~CoordinatedBackingStore");
    if (HasTexture(object.tiles)) NotImplemented_nid_no_patch("WebCore::CoordinatedBackingStoreTile: release of a BitmapTexture");
    object.vtable = &Vtable[2];
    HashTableAbi::DeallocateTable<IdPolicy>(std::exchange(object.tilesToRemove.table, nullptr));
    HashTableAbi::DeallocateTable<TilePolicy>(std::exchange(object.tiles.table, nullptr));
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreD2Ev(CoordinatedBackingStore* self) {
    _ZN7WebCore23CoordinatedBackingStoreD1Ev(self);
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStoreD0Ev(CoordinatedBackingStore* self) {
    _ZN7WebCore23CoordinatedBackingStoreD1Ev(self);
    _ZN3WTF8fastFreeEPv(self);
}

CoordinatedBackingStoreRef APS5_VABI _ZN7WebCore23CoordinatedBackingStore6createEv() {
    auto* object = static_cast<CoordinatedBackingStore*>(_ZN3WTF10fastMallocEm(sizeof(CoordinatedBackingStore)));
    Construct(object);
    return CoordinatedBackingStoreRef(object);
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore10createTileEjf(CoordinatedBackingStore* self, std::uint32_t id, float scale) {
    auto& object = Checked(self, "createTile");
    CheckedKey(id, "createTile");
    HashTableAbi::Add<TilePolicy>(object.tiles.table, id, [id, scale](TileBucket& bucket) {
        bucket.key = id;
        bucket.value.scale = scale;
    });
    object.scale = scale;
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore10removeTileEj(CoordinatedBackingStore* self, std::uint32_t id) {
    auto& object = Checked(self, "removeTile");
    CheckedKey(id, "removeTile");
    HashTableAbi::Add<IdPolicy>(object.tilesToRemove.table, id, [id](unsigned& bucket) { bucket = id; });
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore14removeAllTilesEv(CoordinatedBackingStore* self) {
    auto& object = Checked(self, "removeAllTiles");
    HashTableAbi::ForEachLive<TilePolicy>(object.tiles.table, [&](TileBucket& bucket) { HashTableAbi::Add<IdPolicy>(object.tilesToRemove.table, bucket.key, [&bucket](unsigned& id) { id = bucket.key; }); });
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore10updateTileEjRKNS_7IntRectES3_ON3WTF6RefPtrIN7Nicosia6BufferENS4_13DumbPtrTraitsIS7_EEEERKNS_8IntPointE(
    CoordinatedBackingStore* self, std::uint32_t id, const WebCore::IntRect* sourceRect, const WebCore::IntRect* tileRect, BufferRefPtr* buffer, const WebCore::IntPoint* offset) {
    auto& object = Checked(self, "updateTile");
    CheckedKey(id, "updateTile");
    const auto& source = CheckedArgument(sourceRect, "updateTile", "source rect");
    const auto& target = CheckedArgument(tileRect, "updateTile", "tile rect");
    auto& bufferRef = const_cast<BufferRefPtr&>(CheckedArgument(buffer, "updateTile", "buffer"));
    const auto& bufferOffset = CheckedArgument(offset, "updateTile", "offset");
    TileBucket* entry = HashTableAbi::Find<TilePolicy>(object.tiles.table, id);
    if (!entry) Invalid("updateTile", "no tile with this ID");
    AppendUpdate(entry->value, TileUpdate{BufferRefPtr{bufferRef.ptr}, source, target, bufferOffset});
    bufferRef.ptr = nullptr;
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore7setSizeERKNS_9FloatSizeE(CoordinatedBackingStore* self, const WebCore::FloatSize* size) {
    auto& object = Checked(self, "setSize");
    object.pendingSize = CheckedArgument(size, "setSize", "size");
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore20commitTileOperationsERNS_13TextureMapperE(CoordinatedBackingStore* self, TextureMapper*) {
    auto& object = Checked(self, "commitTileOperations");
    bool needsTextureMapper = false;
    HashTableAbi::ForEachLive<TilePolicy>(object.tiles.table, [&](TileBucket& bucket) {
        const bool removed = HashTableAbi::Find<IdPolicy>(object.tilesToRemove.table, bucket.key);
        if (removed && bucket.value.base.texture) needsTextureMapper = true;
        if (removed) return;
        for (unsigned i = 0; i < bucket.value.updateCount; ++i)
            if (bucket.value.updates[i].buffer.ptr) needsTextureMapper = true;
    });
    if (needsTextureMapper) NotImplemented_nid_no_patch("WebCore::CoordinatedBackingStoreTile::swapBuffers: TextureMapper upload");

    if (!IsZero(object.pendingSize)) {
        object.size = object.pendingSize;
        object.pendingSize = WebCore::FloatSize{};
    }
    HashTableAbi::ForEachLive<IdPolicy>(object.tilesToRemove.table, [&](unsigned& id) {
        HashTableAbi::Remove<TilePolicy>(object.tiles.table, HashTableAbi::Find<TilePolicy>(object.tiles.table, id));
    });
    HashTableAbi::DeallocateTable<IdPolicy>(std::exchange(object.tilesToRemove.table, nullptr));
    HashTableAbi::ForEachLive<TilePolicy>(object.tiles.table, [](TileBucket& bucket) { ClearUpdates(bucket.value); });
}

WebCore::FloatRect APS5_VABI _ZNK7WebCore23CoordinatedBackingStore4rectEv(const CoordinatedBackingStore* self) {
    return Rect(Checked(self, "rect"));
}

WebCore::TransformationMatrix APS5_VABI _ZN7WebCore23CoordinatedBackingStore24adjustedTransformForRectERKNS_9FloatRectE(CoordinatedBackingStore* self, const WebCore::FloatRect* targetRect) {
    const auto& object = Checked(self, "adjustedTransformForRect");
    const auto& target = CheckedArgument(targetRect, "adjustedTransformForRect", "target rect");
    return _ZN7WebCore20TransformationMatrix10rectToRectERKNS_9FloatRectES3_(Rect(object), target);
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore20paintToTextureMapperERNS_13TextureMapperERKNS_9FloatRectERKNS_20TransformationMatrixEf(
    CoordinatedBackingStore* self, TextureMapper*, const WebCore::FloatRect*, const WebCore::TransformationMatrix*, float) {
    auto& object = Checked(self, "paintToTextureMapper");
    if (HasTexture(object.tiles)) NotImplemented_nid_no_patch("WebCore::CoordinatedBackingStore::paintToTextureMapper: textured tiles");
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore25paintTilesToTextureMapperERN3WTF6VectorIPNS_17TextureMapperTileELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEERNS_13TextureMapperERKNS_20TransformationMatrixEfRKNS_9FloatRectE(
    CoordinatedBackingStore* self, TilePointerVector* tiles, TextureMapper* textureMapper, const WebCore::TransformationMatrix* transform, float opacity, const WebCore::FloatRect* rect) {
    Checked(self, "paintTilesToTextureMapper");
    const auto& list = CheckedArgument(tiles, "paintTilesToTextureMapper", "tiles");
    const auto& totalRect = CheckedArgument(rect, "paintTilesToTextureMapper", "rect");
    for (unsigned i = 0; i < list.size; ++i) {
        TextureMapperTile* tile = list.buffer[i];
        if (!tile) Invalid("paintTilesToTextureMapper", "null tile");
        const unsigned edges = _ZN7WebCore25TextureMapperBackingStore25calculateExposedTileEdgesERKNS_9FloatRectES3_(totalRect, tile->rect);
        reinterpret_cast<TilePaint>(const_cast<void*>(tile->vtable[0]))(tile, textureMapper, transform, opacity, edges);
    }
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore10drawBorderERNS_13TextureMapperERKNS_5ColorEfRKNS_9FloatRectERKNS_20TransformationMatrixE(
    CoordinatedBackingStore* self, TextureMapper*, const NKWebKit::CoordinatedAbi::Color*, float, const WebCore::FloatRect*, const WebCore::TransformationMatrix*) {
    auto& object = Checked(self, "drawBorder");
    if (HashTableAbi::KeyCount(object.tiles.table)) NotImplemented_nid_no_patch("WebCore::CoordinatedBackingStore::drawBorder: TextureMapper::drawBorder");
}

void APS5_VABI _ZN7WebCore23CoordinatedBackingStore18drawRepaintCounterERNS_13TextureMapperEiRKNS_5ColorERKNS_9FloatRectERKNS_20TransformationMatrixE(
    CoordinatedBackingStore* self, TextureMapper*, int, const NKWebKit::CoordinatedAbi::Color*, const WebCore::FloatRect*, const WebCore::TransformationMatrix*) {
    auto& object = Checked(self, "drawRepaintCounter");
    if (HashTableAbi::KeyCount(object.tiles.table)) NotImplemented_nid_no_patch("WebCore::CoordinatedBackingStore::drawRepaintCounter: TextureMapper::drawNumber");
}

BufferRef APS5_VABI _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(const WebCore::IntSize* size, unsigned flags) {
    if (!size) throw std::invalid_argument("Nicosia::Buffer::create: null size");
    const std::uint64_t area = std::uint64_t(std::llabs(size->m_width)) * std::uint64_t(std::llabs(size->m_height));
    if (area > 0xffffffffULL || area * 4 > 0xffffffffULL) throw std::overflow_error("Nicosia::Buffer::create: area overflows");
    WTF::TryMallocReturnValue data{};
    _ZN3WTF19tryFastZeroedMallocEm(&data, static_cast<std::size_t>(area * 4));
    auto* buffer = static_cast<Buffer*>(_ZN3WTF10fastMallocEm(sizeof(Buffer)));
    buffer->refCount = 1;
    buffer->data = static_cast<unsigned char*>(data.data);
    buffer->size = *size;
    buffer->flags = flags;
    buffer->lock = 0;
    buffer->condition = 0;
    buffer->paintingState = PaintingComplete;
    return BufferRef(buffer);
}

void APS5_VABI _ZN7Nicosia6BufferD1Ev(Buffer* self) {
    if (!self) throw std::invalid_argument("Nicosia::Buffer::~Buffer: null this");
    if (self->data) _ZN3WTF8fastFreeEPv(std::exchange(self->data, nullptr));
}

void APS5_VABI _ZN7Nicosia6BufferD2Ev(Buffer* self) {
    _ZN7Nicosia6BufferD1Ev(self);
}

void APS5_VABI _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(Buffer* self) {
    if (!self) throw std::invalid_argument("Nicosia::Buffer::waitUntilPaintingComplete: null this");
    if (self->paintingState != PaintingComplete) NotImplemented_nid_no_patch("Nicosia::Buffer::waitUntilPaintingComplete: painting in progress");
}

}
