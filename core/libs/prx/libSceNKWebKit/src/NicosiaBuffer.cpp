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

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/NicosiaBufferExports.hpp"
#include "prx/libSceNKWebKit/include/WtfExports.hpp"

#include <atomic>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

namespace {

using NKWebKit::NicosiaBufferAbi::Buffer;
using NKWebKit::NicosiaBufferAbi::PaintingState;

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

Buffer& Checked(Buffer* self, const char* caller) {
    if (!self) throw std::invalid_argument(std::string("Nicosia::Buffer::") + caller + ": null this");
    return *self;
}

std::size_t ByteCount(const WebCore::IntSize& size) {
    if (size.m_width < 0 || size.m_height < 0) throw std::overflow_error("Nicosia::Buffer::create: negative size");
    const auto area = static_cast<unsigned long long>(size.m_width) * static_cast<unsigned long long>(size.m_height);
    if (area > std::numeric_limits<unsigned>::max() / 4) throw std::overflow_error("Nicosia::Buffer::create: area overflow");
    return static_cast<std::size_t>(area * 4);
}

}  // namespace

namespace NKWebKit::NicosiaBufferAbi {

BufferRef::~BufferRef() {
    if (!ptr || std::atomic_ref<unsigned>(ptr->refCount).fetch_sub(1, std::memory_order_acq_rel) != 1) return;
    _ZN7Nicosia6BufferD1Ev(ptr);
    _ZN3WTF8fastFreeEPv(ptr);
}

}  // namespace NKWebKit::NicosiaBufferAbi

extern "C" {

NKWebKit::NicosiaBufferAbi::BufferRef APS5_VABI _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(const WebCore::IntSize* size, unsigned flags) {
    if (!size) throw std::invalid_argument("Nicosia::Buffer::create: null size");
    const std::size_t bytes = ByteCount(*size);
    WTF::TryMallocReturnValue pixels{};
    _ZN3WTF19tryFastZeroedMallocEm(&pixels, bytes);
    auto* buffer = static_cast<Buffer*>(_ZN3WTF10fastMallocEm(sizeof(Buffer)));
    *buffer = Buffer{1, static_cast<unsigned char*>(pixels.data), *size, flags, 0, 0, PaintingState::Complete};
    return NKWebKit::NicosiaBufferAbi::BufferRef(buffer);
}

void APS5_VABI _ZN7Nicosia6BufferD2Ev(Buffer* self) {
    auto& buffer = Checked(self, "~Buffer");
    _ZN3WTF8fastFreeEPv(std::exchange(buffer.data, nullptr));
}

void APS5_VABI _ZN7Nicosia6BufferD1Ev(Buffer* self) {
    _ZN7Nicosia6BufferD2Ev(self);
}

void APS5_VABI _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(Buffer* self) {
    auto& buffer = Checked(self, "waitUntilPaintingComplete");
    for (;;) {
        {
            Locker locker(buffer.paintingLock);
            if (buffer.paintingState == PaintingState::Complete) return;
        }
        std::this_thread::yield();
    }
}

}
