// Derived from Source/WebCore/platform/graphics/texmap/coordinated/CoordinatedBackingStore.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

// Derived from Source/WebCore/platform/graphics/texmap/TextureMapperTile.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 Copyright (C) 2013 Nokia Corporation and/or its subsidiary(-ies)

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

// Derived from Source/WebCore/platform/graphics/nicosia/NicosiaBuffer.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

#pragma once

#include "prx/libSceNKWebKit/src/WebCoreTypes.hpp"

#include <cstddef>
#include <cstdint>

namespace NKWebKit::CoordinatedAbi {

struct TextureMapper;
struct Color;

struct Buffer {
    unsigned refCount;
    unsigned char* data;
    WebCore::IntSize size;
    unsigned flags;
    std::uint8_t lock;
    std::uint8_t condition;
    int paintingState;
};

inline constexpr int PaintingInProgress = 0;
inline constexpr int PaintingComplete = 1;

struct BufferRef {
    Buffer* ptr;

    explicit BufferRef(Buffer* value) : ptr(value) {}
    BufferRef(const BufferRef&) = delete;
    BufferRef& operator=(const BufferRef&) = delete;
    ~BufferRef() {}
};

struct BufferRefPtr {
    Buffer* ptr;
};

struct TextureMapperTile {
    const void* const* vtable;
    void* texture;
    WebCore::FloatRect rect;
};

struct TileUpdate {
    BufferRefPtr buffer;
    WebCore::IntRect sourceRect;
    WebCore::IntRect tileRect;
    WebCore::IntPoint bufferOffset;
};

struct Tile {
    TextureMapperTile base;
    TileUpdate* updates;
    unsigned updateCapacity;
    unsigned updateCount;
    float scale;
};

struct TileBucket {
    unsigned key;
    Tile value;
};

struct TileMap {
    TileBucket* table;
};

struct TileIdSet {
    unsigned* table;
};

struct TilePointerVector {
    TextureMapperTile** buffer;
    unsigned capacity;
    unsigned size;
};

struct CoordinatedBackingStore {
    const void* const* vtable;
    void* client;
    unsigned refCount;
    TileMap tiles;
    TileIdSet tilesToRemove;
    WebCore::FloatSize pendingSize;
    WebCore::FloatSize size;
    float scale;
};

struct CoordinatedBackingStoreRef {
    CoordinatedBackingStore* ptr;

    explicit CoordinatedBackingStoreRef(CoordinatedBackingStore* value) : ptr(value) {}
    CoordinatedBackingStoreRef(const CoordinatedBackingStoreRef&) = delete;
    CoordinatedBackingStoreRef& operator=(const CoordinatedBackingStoreRef&) = delete;
    ~CoordinatedBackingStoreRef() {}
};

static_assert(sizeof(Buffer) == 40);
static_assert(offsetof(Buffer, data) == 8);
static_assert(offsetof(Buffer, size) == 16);
static_assert(offsetof(Buffer, flags) == 24);
static_assert(offsetof(Buffer, lock) == 28);
static_assert(offsetof(Buffer, condition) == 29);
static_assert(offsetof(Buffer, paintingState) == 32);
static_assert(sizeof(BufferRef) == 8);
static_assert(sizeof(BufferRefPtr) == 8);
static_assert(sizeof(TextureMapperTile) == 32);
static_assert(offsetof(TextureMapperTile, rect) == 16);
static_assert(sizeof(TileUpdate) == 48);
static_assert(offsetof(TileUpdate, tileRect) == 24);
static_assert(offsetof(TileUpdate, bufferOffset) == 40);
static_assert(sizeof(Tile) == 56);
static_assert(offsetof(Tile, updates) == 32);
static_assert(offsetof(Tile, scale) == 48);
static_assert(sizeof(TileBucket) == 64);
static_assert(offsetof(TileBucket, value) == 8);
static_assert(sizeof(TilePointerVector) == 16);
static_assert(sizeof(CoordinatedBackingStore) == 64);
static_assert(offsetof(CoordinatedBackingStore, refCount) == 16);
static_assert(offsetof(CoordinatedBackingStore, tiles) == 24);
static_assert(offsetof(CoordinatedBackingStore, tilesToRemove) == 32);
static_assert(offsetof(CoordinatedBackingStore, pendingSize) == 40);
static_assert(offsetof(CoordinatedBackingStore, size) == 48);
static_assert(offsetof(CoordinatedBackingStore, scale) == 56);
static_assert(sizeof(CoordinatedBackingStoreRef) == 8);

}  // namespace NKWebKit::CoordinatedAbi
