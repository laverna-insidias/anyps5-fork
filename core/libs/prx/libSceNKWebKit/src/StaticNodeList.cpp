// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/dom/StaticNodeList.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2007, 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "prx/libSceNKWebKit/include/StaticNodeListExports.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>

namespace {

struct StaticNodeList {
    const void* vtable;
    void* wrapper;
    unsigned refCount;
    void** nodes;
    std::uint32_t capacity;
    std::uint32_t size;
};

static_assert(offsetof(StaticNodeList, nodes) == 24);
static_assert(offsetof(StaticNodeList, size) == 36);
static_assert(sizeof(StaticNodeList) == 40);

}  // namespace

extern "C" {

void* APS5_VABI _ZNK7WebCore14StaticNodeList4itemEj(const void* self, unsigned index) {
    if (!self) throw std::invalid_argument("null WebCore::StaticNodeList");
    const auto* list = static_cast<const StaticNodeList*>(self);
    return index < list->size ? list->nodes[index] : nullptr;
}

}
