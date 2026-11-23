// Derived from Source/WebCore/platform/graphics/nicosia/texmap/NicosiaBackingStoreTextureMapperImpl.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

#include "prx/libc/include/General.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/CoordinatedBackingStoreExports.hpp"
#include "prx/libSceNKWebKit/include/NicosiaBackingStoreExports.hpp"
#include "prx/libSceNKWebKit/include/WtfExports.hpp"
#include "prx/libSceNKWebKit/src/WtfIntHashTable.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace {

using NKWebKit::CoordinatedAbi::Buffer;
using NKWebKit::NicosiaAbi::BackingStoreTextureMapperImpl;
using NKWebKit::NicosiaAbi::BackingStoreTileUpdate;
using NKWebKit::NicosiaAbi::TileUpdateData;
using NKWebKit::NicosiaAbi::TileVector;

constexpr unsigned VectorMinCapacity = 16;

BackingStoreTextureMapperImpl& Checked(BackingStoreTextureMapperImpl* self, const char* caller) {
    if (!self) throw std::invalid_argument(std::string("Nicosia::BackingStoreTextureMapperImpl::") + caller + ": null this");
    return *self;
}

void RefBuffer(Buffer* buffer) {
    if (buffer) std::atomic_ref<unsigned>(buffer->refCount).fetch_add(1);
}

void DerefBuffer(Buffer* buffer) {
    if (buffer && std::atomic_ref<unsigned>(buffer->refCount).fetch_sub(1) == 1) {
        _ZN7Nicosia6BufferD1Ev(buffer);
        _ZN3WTF8fastFreeEPv(buffer);
    }
}

template<typename T>
void AppendVector(TileVector<T>& to, const TileVector<T>& from) {
    if (!from.size) return;
    const std::uint64_t newSize = std::uint64_t(to.size) + from.size;
    if (newSize > to.capacity) {
        const std::uint64_t grown = std::uint64_t(to.capacity) + to.capacity / 4 + 1;
        const std::uint64_t capacity = std::max<std::uint64_t>(newSize, std::max<std::uint64_t>(VectorMinCapacity, grown));
        if (capacity > 0xffffffffULL / sizeof(T)) throw std::length_error("WTF::Vector<BackingStoreTextureMapperImpl::TileUpdate>: capacity overflow");
        auto* buffer = static_cast<T*>(_ZN3WTF10fastMallocEm(capacity * sizeof(T)));
        if (to.size) std::memcpy(buffer, to.buffer, std::size_t(to.size) * sizeof(T));
        if (to.buffer) _ZN3WTF8fastFreeEPv(to.buffer);
        to.buffer = buffer;
        to.capacity = static_cast<unsigned>(capacity);
    }
    std::memcpy(to.buffer + to.size, from.buffer, std::size_t(from.size) * sizeof(T));
    if constexpr (std::is_same_v<T, TileUpdateData>)
        for (unsigned i = 0; i < from.size; ++i) RefBuffer(to.buffer[to.size + i].updateInfo.buffer);
    to.size = static_cast<unsigned>(newSize);
}

template<typename T>
void ClearVector(TileVector<T>& vector) {
    T* buffer = std::exchange(vector.buffer, nullptr);
    const unsigned size = std::exchange(vector.size, 0U);
    vector.capacity = 0;
    if constexpr (std::is_same_v<T, TileUpdateData>)
        for (unsigned i = 0; i < size; ++i) DerefBuffer(buffer[i].updateInfo.buffer);
    if (buffer) _ZN3WTF8fastFreeEPv(buffer);
}

template<typename T>
TileVector<T> TakeVector(TileVector<T>& vector) {
    return {std::exchange(vector.buffer, nullptr), std::exchange(vector.capacity, 0U), std::exchange(vector.size, 0U)};
}

}  // namespace

extern "C" {

void APS5_VABI _ZN7Nicosia29BackingStoreTextureMapperImpl11flushUpdateEv(BackingStoreTextureMapperImpl* self) {
    auto& impl = Checked(self, "flushUpdate");
    impl.hasPendingTileCreation = false;
    {
        NKWebKit::WtfAbi::HashTableAbi::LockHolder locker(impl.lock);
        AppendVector(impl.pending.tilesToCreate, impl.layerUpdate.tilesToCreate);
        AppendVector(impl.pending.tilesToUpdate, impl.layerUpdate.tilesToUpdate);
        AppendVector(impl.pending.tilesToRemove, impl.layerUpdate.tilesToRemove);
    }
    ClearVector(impl.layerUpdate.tilesToCreate);
    ClearVector(impl.layerUpdate.tilesToUpdate);
    ClearVector(impl.layerUpdate.tilesToRemove);
}

BackingStoreTileUpdate APS5_VABI _ZN7Nicosia29BackingStoreTextureMapperImpl10takeUpdateEv(BackingStoreTextureMapperImpl* self) {
    auto& impl = Checked(self, "takeUpdate");
    NKWebKit::WtfAbi::HashTableAbi::LockHolder locker(impl.lock);
    return BackingStoreTileUpdate(TakeVector(impl.pending.tilesToCreate), TakeVector(impl.pending.tilesToUpdate), TakeVector(impl.pending.tilesToRemove));
}

}
