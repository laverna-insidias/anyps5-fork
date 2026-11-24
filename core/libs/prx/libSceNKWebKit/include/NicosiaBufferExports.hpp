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

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/src/WebCoreTypes.hpp"

#include <cstddef>
#include <cstdint>

namespace NKWebKit::NicosiaBufferAbi {

enum class PaintingState : int {
    InProgress,
    Complete,
};

struct Buffer {
    unsigned refCount;
    unsigned char* data;
    WebCore::IntSize size;
    unsigned flags;
    std::uint8_t paintingLock;
    std::uint8_t paintingCondition;
    PaintingState paintingState;
};

struct BufferRef {
    Buffer* ptr;

    explicit BufferRef(Buffer* adopted) : ptr(adopted) {}
    BufferRef(const BufferRef&) = delete;
    BufferRef& operator=(const BufferRef&) = delete;
    ~BufferRef();
};

static_assert(offsetof(Buffer, data) == 8);
static_assert(offsetof(Buffer, size) == 16);
static_assert(offsetof(Buffer, flags) == 24);
static_assert(offsetof(Buffer, paintingLock) == 28);
static_assert(offsetof(Buffer, paintingCondition) == 29);
static_assert(offsetof(Buffer, paintingState) == 32);
static_assert(sizeof(Buffer) == 40);
static_assert(sizeof(BufferRef) == 8);

}  // namespace NKWebKit::NicosiaBufferAbi

extern "C" {

NKWebKit::NicosiaBufferAbi::BufferRef APS5_VABI _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(const WebCore::IntSize* size, unsigned flags);
void APS5_VABI _ZN7Nicosia6BufferD1Ev(NKWebKit::NicosiaBufferAbi::Buffer* self);
void APS5_VABI _ZN7Nicosia6BufferD2Ev(NKWebKit::NicosiaBufferAbi::Buffer* self);
void APS5_VABI _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(NKWebKit::NicosiaBufferAbi::Buffer* self);

}
