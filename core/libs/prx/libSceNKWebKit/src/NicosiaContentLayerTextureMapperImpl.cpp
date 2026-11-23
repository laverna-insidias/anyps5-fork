// Derived from Source/WebCore/platform/graphics/nicosia/texmap/NicosiaContentLayerTextureMapperImpl.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/NicosiaContentLayerExports.hpp"

#include <atomic>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace {

using NKWebKit::NicosiaContentLayerAbi::ContentLayerTextureMapperImpl;

class Locker {
public:
    explicit Locker(std::uint8_t& lock) : m_lock(lock) {
        for (;;) {
            std::uint8_t expected = 0;
            if (m_lock.compare_exchange_weak(expected, 1, std::memory_order_acquire)) return;
            std::this_thread::yield();
        }
    }
    ~Locker() { m_lock.store(0, std::memory_order_release); }
    Locker(const Locker&) = delete;
    Locker& operator=(const Locker&) = delete;

private:
    std::atomic_ref<std::uint8_t> m_lock;
};

ContentLayerTextureMapperImpl& Checked(ContentLayerTextureMapperImpl* self, const char* caller) {
    if (!self) throw std::invalid_argument(std::string("Nicosia::ContentLayerTextureMapperImpl::") + caller + ": null this");
    return *self;
}

}  // namespace

extern "C" {

bool APS5_VABI _ZN7Nicosia29ContentLayerTextureMapperImpl11flushUpdateEv(ContentLayerTextureMapperImpl* self) {
    auto& impl = Checked(self, "flushUpdate");
    Locker locker(impl.lock);
    return std::exchange(impl.pendingUpdate, false);
}

void APS5_VABI _ZN7Nicosia29ContentLayerTextureMapperImpl19swapBuffersIfNeededEv(ContentLayerTextureMapperImpl* self) {
    auto& impl = Checked(self, "swapBuffersIfNeeded");
    Locker locker(impl.lock);
    if (!impl.client) return;
    using SwapBuffers = void(APS5_VABI*)(NKWebKit::NicosiaContentLayerAbi::ContentLayerClient*);
    reinterpret_cast<SwapBuffers>(impl.client->vtable[NKWebKit::NicosiaContentLayerAbi::ContentLayerClientSwapBuffersIfNeeded])(impl.client);
}

}
