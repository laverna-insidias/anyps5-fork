// Derived from Source/WebCore/platform/graphics/nicosia/texmap/NicosiaImageBackingTextureMapperImpl.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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
#include "prx/libSceNKWebKit/include/NicosiaImageBackingExports.hpp"
#include "prx/libSceNKWebKit/include/WtfExports.hpp"
#include "prx/libSceNKWebKit/src/WtfIntHashTable.hpp"

#include <atomic>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

using NKWebKit::CoordinatedAbi::Buffer;
using NKWebKit::CoordinatedAbi::CoordinatedBackingStore;
using NKWebKit::NicosiaAbi::BackingStoreContainer;
using NKWebKit::NicosiaAbi::ImageBackingStore;
using NKWebKit::NicosiaAbi::ImageBackingTextureMapperImpl;
using NKWebKit::NicosiaAbi::ImageBackingUpdate;

ImageBackingTextureMapperImpl& Checked(ImageBackingTextureMapperImpl* self, const char* caller) {
    if (!self) throw std::invalid_argument(std::string("Nicosia::ImageBackingTextureMapperImpl::") + caller + ": null this");
    return *self;
}

template<typename T>
bool DerefThreadSafe(T* object) {
    return object && std::atomic_ref<unsigned>(object->refCount).fetch_sub(1) == 1;
}

void DerefBackingStore(CoordinatedBackingStore* backingStore) {
    if (!backingStore) return;
    if (backingStore->refCount != 1) {
        --backingStore->refCount;
        return;
    }
    using DeletingDestructor = void(APS5_VABI*)(CoordinatedBackingStore*);
    reinterpret_cast<DeletingDestructor>(backingStore->vtable[1])(backingStore);
}

void DerefImageBackingStore(ImageBackingStore* store) {
    if (!DerefThreadSafe(store)) return;
    BackingStoreContainer* container = std::exchange(store->container, nullptr);
    if (DerefThreadSafe(container)) {
        DerefBackingStore(std::exchange(container->backingStore, nullptr));
        _ZN3WTF8fastFreeEPv(container);
    }
    Buffer* buffer = std::exchange(store->buffer, nullptr);
    if (DerefThreadSafe(buffer)) {
        _ZN7Nicosia6BufferD1Ev(buffer);
        _ZN3WTF8fastFreeEPv(buffer);
    }
    _ZN3WTF8fastFreeEPv(store);
}

void MoveAssign(ImageBackingUpdate& to, ImageBackingUpdate& from) {
    to.isVisible = from.isVisible;
    to.nativeImageID = from.nativeImageID;
    DerefImageBackingStore(std::exchange(to.imageBackingStore, std::exchange(from.imageBackingStore, nullptr)));
}

}  // namespace

extern "C" {

void APS5_VABI _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(ImageBackingTextureMapperImpl* self) {
    auto& impl = Checked(self, "flushUpdate");
    NKWebKit::WtfAbi::HashTableAbi::LockHolder locker(impl.lock);
    if (impl.layerUpdate.nativeImageID == impl.update.nativeImageID && !impl.layerUpdate.imageBackingStore) {
        impl.update.isVisible = impl.layerUpdate.isVisible;
        return;
    }
    MoveAssign(impl.update, impl.layerUpdate);
}

ImageBackingUpdate APS5_VABI _ZN7Nicosia29ImageBackingTextureMapperImpl10takeUpdateEv(ImageBackingTextureMapperImpl* self) {
    auto& impl = Checked(self, "takeUpdate");
    NKWebKit::WtfAbi::HashTableAbi::LockHolder locker(impl.lock);
    return ImageBackingUpdate(impl.update.isVisible, impl.update.nativeImageID, std::exchange(impl.update.imageBackingStore, nullptr));
}

}
