// Derived from Source/WebCore/platform/graphics/nicosia/texmap/NicosiaImageBackingTextureMapperImpl.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

// Derived from Source/WebCore/platform/graphics/nicosia/texmap/NicosiaImageBackingStore.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2022 Metrological Group B.V.
 * Copyright (C) 2022 Igalia S.L.
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

#include "prx/libSceNKWebKit/src/CoordinatedBackingStoreAbi.hpp"

#include <cstddef>
#include <cstdint>

namespace NKWebKit::NicosiaAbi {

struct BackingStoreContainer {
    unsigned refCount;
    CoordinatedAbi::CoordinatedBackingStore* backingStore;
};

struct ImageBackingStore {
    unsigned refCount;
    CoordinatedAbi::Buffer* buffer;
    BackingStoreContainer* container;
};

struct ImageBackingUpdate {
    bool isVisible;
    std::uintptr_t nativeImageID;
    ImageBackingStore* imageBackingStore;

    ImageBackingUpdate(bool visible = false, std::uintptr_t imageID = 0, ImageBackingStore* store = nullptr) : isVisible(visible), nativeImageID(imageID), imageBackingStore(store) {}
    ImageBackingUpdate(const ImageBackingUpdate&) = delete;
    ImageBackingUpdate& operator=(const ImageBackingUpdate&) = delete;
    ~ImageBackingUpdate() {}
};

struct ImageBackingTextureMapperImpl {
    const void* const* vtable;
    std::uintptr_t imageID;
    ImageBackingUpdate layerUpdate;
    ImageBackingStore* compositionStore;
    std::uint8_t lock;
    ImageBackingUpdate update;
};

static_assert(offsetof(BackingStoreContainer, backingStore) == 8);
static_assert(offsetof(ImageBackingStore, buffer) == 8);
static_assert(offsetof(ImageBackingStore, container) == 16);
static_assert(offsetof(ImageBackingUpdate, nativeImageID) == 8);
static_assert(offsetof(ImageBackingUpdate, imageBackingStore) == 16);
static_assert(sizeof(ImageBackingUpdate) == 24);
static_assert(offsetof(ImageBackingTextureMapperImpl, imageID) == 8);
static_assert(offsetof(ImageBackingTextureMapperImpl, layerUpdate) == 16);
static_assert(offsetof(ImageBackingTextureMapperImpl, compositionStore) == 40);
static_assert(offsetof(ImageBackingTextureMapperImpl, lock) == 48);
static_assert(offsetof(ImageBackingTextureMapperImpl, update) == 56);
static_assert(sizeof(ImageBackingTextureMapperImpl) == 80);

}  // namespace NKWebKit::NicosiaAbi
