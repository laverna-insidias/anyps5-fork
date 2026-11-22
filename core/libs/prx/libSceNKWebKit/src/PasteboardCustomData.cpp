// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/platform/PasteboardCustomData.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2019-2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "prx/libSceNKWebKit/include/LegacyExports.hpp"
#include "prx/libSceNKWebKit/include/PasteboardCustomDataExports.hpp"

#include <atomic>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace NKWebKit::PasteboardAbi {

void Deref(SharedBuffer* buffer) {
    if (buffer && std::atomic_ref<unsigned>(buffer->refCount).fetch_sub(1) == 1) {
        reinterpret_cast<void(APS5_VABI*)(SharedBuffer*)>(&_ZN7WebCore12SharedBufferD1Ev)(buffer);
        _ZN3WTF8fastFreeEPv(buffer);
    }
}

}  // namespace NKWebKit::PasteboardAbi

namespace {

using NKWebKit::PasteboardAbi::Entry;
using NKWebKit::PasteboardAbi::PasteboardCustomData;
using NKWebKit::WtfAbi::String;
using NKWebKit::WtfAbi::Vector;

template <typename T>
T* Checked(T* self, const char* function) {
    if (!self) {
        throw std::invalid_argument(std::string(function) + ": null object");
    }
    return self;
}

void ConstructEntry(Entry* self) {
    new (Checked(self, "PasteboardCustomData::Entry")) Entry();
}

void CopyEntry(Entry* self, const Entry& other) {
    new (Checked(self, "PasteboardCustomData::Entry")) Entry(other);
}

void MoveEntry(Entry* self, Entry&& other) {
    new (Checked(self, "PasteboardCustomData::Entry")) Entry(std::move(other));
}

void Construct(PasteboardCustomData* self) {
    new (Checked(self, "PasteboardCustomData")) PasteboardCustomData();
}

void ConstructFrom(PasteboardCustomData* self, String&& origin, Vector<Entry>&& data) {
    new (Checked(self, "PasteboardCustomData")) PasteboardCustomData{std::move(origin), std::move(data)};
}

void Copy(PasteboardCustomData* self, const PasteboardCustomData& other) {
    new (Checked(self, "PasteboardCustomData")) PasteboardCustomData(other);
}

void Move(PasteboardCustomData* self, PasteboardCustomData&& other) {
    new (Checked(self, "PasteboardCustomData")) PasteboardCustomData(std::move(other));
}

void Destroy(PasteboardCustomData* self) {
    Checked(self, "~PasteboardCustomData")->~PasteboardCustomData();
}

}  // namespace

extern "C" {

void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC1Ev(Entry* self) {
    ConstructEntry(self);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC2Ev(Entry* self) {
    ConstructEntry(self);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC1ERKS1_(Entry* self, const Entry& other) {
    CopyEntry(self, other);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC2ERKS1_(Entry* self, const Entry& other) {
    CopyEntry(self, other);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC1EOS1_(Entry* self, Entry&& other) {
    MoveEntry(self, std::move(other));
}

void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC2EOS1_(Entry* self, Entry&& other) {
    MoveEntry(self, std::move(other));
}

Entry* APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryaSERKS1_(Entry* self, const Entry& other) {
    auto& entry = *Checked(self, "PasteboardCustomData::Entry::operator=");
    entry.type = other.type;
    entry.customData = other.customData;
    entry.platformData = other.platformData;
    return self;
}

Entry* APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryaSEOS1_(Entry* self, Entry&& other) {
    auto& entry = *Checked(self, "PasteboardCustomData::Entry::operator=");
    entry.type = std::move(other.type);
    entry.customData = std::move(other.customData);
    entry.platformData = std::move(other.platformData);
    return self;
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataC1Ev(PasteboardCustomData* self) {
    Construct(self);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataC2Ev(PasteboardCustomData* self) {
    Construct(self);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataC1EON3WTF6StringEONS1_6VectorINS0_5EntryELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEE(PasteboardCustomData* self, String&& origin, Vector<Entry>&& data) {
    ConstructFrom(self, std::move(origin), std::move(data));
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataC2EON3WTF6StringEONS1_6VectorINS0_5EntryELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEE(PasteboardCustomData* self, String&& origin, Vector<Entry>&& data) {
    ConstructFrom(self, std::move(origin), std::move(data));
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataC1ERKS0_(PasteboardCustomData* self, const PasteboardCustomData& other) {
    Copy(self, other);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataC2ERKS0_(PasteboardCustomData* self, const PasteboardCustomData& other) {
    Copy(self, other);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataC1EOS0_(PasteboardCustomData* self, PasteboardCustomData&& other) {
    Move(self, std::move(other));
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataC2EOS0_(PasteboardCustomData* self, PasteboardCustomData&& other) {
    Move(self, std::move(other));
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataD1Ev(PasteboardCustomData* self) {
    Destroy(self);
}

void APS5_VABI _ZN7WebCore20PasteboardCustomDataD2Ev(PasteboardCustomData* self) {
    Destroy(self);
}

PasteboardCustomData* APS5_VABI _ZN7WebCore20PasteboardCustomDataaSERKS0_(PasteboardCustomData* self, const PasteboardCustomData& other) {
    auto& data = *Checked(self, "PasteboardCustomData::operator=");
    data.origin = other.origin;
    data.data = other.data;
    return self;
}

}
