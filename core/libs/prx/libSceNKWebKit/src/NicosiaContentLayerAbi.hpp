// Derived from Source/WebCore/platform/graphics/nicosia/texmap/NicosiaContentLayerTextureMapperImpl.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

#pragma once

#include <cstddef>
#include <cstdint>

namespace NKWebKit::NicosiaContentLayerAbi {

struct ContentLayerClient {
    const void* const* vtable;
};

enum ContentLayerClientSlot : std::size_t {
    ContentLayerClientSwapBuffersIfNeeded = 2,
};

struct ContentLayerTextureMapperImpl {
    const void* const* vtable;
    void* proxy;
    std::uint8_t lock;
    ContentLayerClient* client;
    bool pendingUpdate;
};

static_assert(offsetof(ContentLayerTextureMapperImpl, proxy) == 8);
static_assert(offsetof(ContentLayerTextureMapperImpl, lock) == 16);
static_assert(offsetof(ContentLayerTextureMapperImpl, client) == 24);
static_assert(offsetof(ContentLayerTextureMapperImpl, pendingUpdate) == 32);
static_assert(sizeof(ContentLayerTextureMapperImpl) == 40);

}  // namespace NKWebKit::NicosiaContentLayerAbi
