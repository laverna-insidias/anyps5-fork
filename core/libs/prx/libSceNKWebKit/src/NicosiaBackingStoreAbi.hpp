// Derived from Source/WebCore/platform/graphics/nicosia/texmap/NicosiaBackingStoreTextureMapperImpl.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2018 Metrological Group B.V.
 * Copyright (C) 2018 Igalia S.L.
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

// Derived from Source/WebCore/platform/graphics/texmap/coordinated/SurfaceUpdateInfo.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

#pragma once

#include "prx/libSceNKWebKit/src/CoordinatedBackingStoreAbi.hpp"

#include <cstddef>
#include <cstdint>

namespace NKWebKit::NicosiaAbi {

template<typename T>
struct TileVector {
    T* buffer;
    unsigned capacity;
    unsigned size;
};

struct TileCreationData {
    std::uint32_t tileID;
    float scale;
};

struct SurfaceUpdateInfo {
    WebCore::IntRect updateRect;
    CoordinatedAbi::Buffer* buffer;
};

struct TileUpdateData {
    std::uint32_t tileID;
    WebCore::IntRect tileRect;
    SurfaceUpdateInfo updateInfo;
};

struct TileRemovalData {
    std::uint32_t tileID;
};

struct BackingStoreTileUpdate {
    TileVector<TileCreationData> tilesToCreate;
    TileVector<TileUpdateData> tilesToUpdate;
    TileVector<TileRemovalData> tilesToRemove;

    BackingStoreTileUpdate(TileVector<TileCreationData> create = {}, TileVector<TileUpdateData> update = {}, TileVector<TileRemovalData> remove = {}) : tilesToCreate(create), tilesToUpdate(update), tilesToRemove(remove) {}
    BackingStoreTileUpdate(const BackingStoreTileUpdate&) = delete;
    BackingStoreTileUpdate& operator=(const BackingStoreTileUpdate&) = delete;
    ~BackingStoreTileUpdate() {}
};

struct BackingStoreTextureMapperImpl {
    const void* const* vtable;
    const void* const* tiledBackingStoreClientVtable;
    void* mainBackingStore;
    BackingStoreTileUpdate layerUpdate;
    bool isFlushing;
    bool isPurging;
    bool hasPendingTileCreation;
    CoordinatedAbi::CoordinatedBackingStore* backingStore;
    std::uint8_t lock;
    BackingStoreTileUpdate pending;
};

static_assert(sizeof(TileCreationData) == 8);
static_assert(offsetof(TileUpdateData, tileRect) == 4);
static_assert(offsetof(TileUpdateData, updateInfo) == 24);
static_assert(offsetof(SurfaceUpdateInfo, buffer) == 16);
static_assert(sizeof(TileUpdateData) == 48);
static_assert(sizeof(TileRemovalData) == 4);
static_assert(sizeof(TileVector<TileUpdateData>) == 16);
static_assert(sizeof(BackingStoreTileUpdate) == 48);
static_assert(offsetof(BackingStoreTextureMapperImpl, mainBackingStore) == 16);
static_assert(offsetof(BackingStoreTextureMapperImpl, layerUpdate) == 24);
static_assert(offsetof(BackingStoreTextureMapperImpl, isFlushing) == 72);
static_assert(offsetof(BackingStoreTextureMapperImpl, hasPendingTileCreation) == 74);
static_assert(offsetof(BackingStoreTextureMapperImpl, backingStore) == 80);
static_assert(offsetof(BackingStoreTextureMapperImpl, lock) == 88);
static_assert(offsetof(BackingStoreTextureMapperImpl, pending) == 96);
static_assert(sizeof(BackingStoreTextureMapperImpl) == 144);

}  // namespace NKWebKit::NicosiaAbi
