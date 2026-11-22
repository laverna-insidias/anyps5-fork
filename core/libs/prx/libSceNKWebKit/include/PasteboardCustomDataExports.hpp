#pragma once

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/src/WtfStringAbi.hpp"
#include "prx/libSceNKWebKit/src/WtfVector.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <new>
#include <utility>

namespace NKWebKit::PasteboardAbi {

struct SharedBuffer {
    unsigned refCount;
};

inline void Ref(SharedBuffer* buffer) {
    std::atomic_ref<unsigned>(buffer->refCount).fetch_add(1);
}

void Deref(SharedBuffer* buffer);

struct PlatformData {
    union {
        NKWebKit::WtfAbi::String string;
        SharedBuffer* buffer;
    };
    std::uint8_t index;
    std::uint8_t padding[7];

    PlatformData() : string(), index(0), padding{} {}

    PlatformData(const PlatformData& other) : index(0), padding{} {
        if (other.index == 1) {
            buffer = other.buffer;
            Ref(buffer);
            index = 1;
        } else if (other.index == 0) {
            new (&string) NKWebKit::WtfAbi::String(other.string);
        } else {
            new (&string) NKWebKit::WtfAbi::String();
        }
    }

    PlatformData(PlatformData&& other) noexcept : index(other.index), padding{} {
        if (index == 1) {
            buffer = std::exchange(other.buffer, nullptr);
        } else if (index == 0) {
            new (&string) NKWebKit::WtfAbi::String(std::move(other.string));
        }
    }

    PlatformData& operator=(PlatformData&& other) noexcept {
        if (this != &other) {
            Destroy();
            new (this) PlatformData(std::move(other));
        }
        return *this;
    }

    PlatformData& operator=(const PlatformData& other) {
        PlatformData copy(other);
        return *this = std::move(copy);
    }

    ~PlatformData() { Destroy(); }

    void Destroy() {
        if (index == 1) {
            Deref(std::exchange(buffer, nullptr));
        } else if (index == 0) {
            string.~String();
        }
    }
};

struct Entry {
    NKWebKit::WtfAbi::String type;
    NKWebKit::WtfAbi::String customData;
    PlatformData platformData;
};

struct PasteboardCustomData {
    NKWebKit::WtfAbi::String origin;
    NKWebKit::WtfAbi::Vector<Entry> data;
};

static_assert(sizeof(PlatformData) == 16);
static_assert(offsetof(PlatformData, index) == 8);
static_assert(sizeof(Entry) == 32);
static_assert(offsetof(Entry, customData) == 8);
static_assert(offsetof(Entry, platformData) == 16);
static_assert(sizeof(PasteboardCustomData) == 24);
static_assert(offsetof(PasteboardCustomData, data) == 8);

}  // namespace NKWebKit::PasteboardAbi

extern "C" {

void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC1Ev(NKWebKit::PasteboardAbi::Entry* self);
void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC2Ev(NKWebKit::PasteboardAbi::Entry* self);
void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC1ERKS1_(NKWebKit::PasteboardAbi::Entry* self, const NKWebKit::PasteboardAbi::Entry& other);
void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC2ERKS1_(NKWebKit::PasteboardAbi::Entry* self, const NKWebKit::PasteboardAbi::Entry& other);
void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC1EOS1_(NKWebKit::PasteboardAbi::Entry* self, NKWebKit::PasteboardAbi::Entry&& other);
void APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryC2EOS1_(NKWebKit::PasteboardAbi::Entry* self, NKWebKit::PasteboardAbi::Entry&& other);
NKWebKit::PasteboardAbi::Entry* APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryaSERKS1_(NKWebKit::PasteboardAbi::Entry* self, const NKWebKit::PasteboardAbi::Entry& other);
NKWebKit::PasteboardAbi::Entry* APS5_VABI _ZN7WebCore20PasteboardCustomData5EntryaSEOS1_(NKWebKit::PasteboardAbi::Entry* self, NKWebKit::PasteboardAbi::Entry&& other);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataC1Ev(NKWebKit::PasteboardAbi::PasteboardCustomData* self);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataC2Ev(NKWebKit::PasteboardAbi::PasteboardCustomData* self);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataC1EON3WTF6StringEONS1_6VectorINS0_5EntryELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEE(NKWebKit::PasteboardAbi::PasteboardCustomData* self, NKWebKit::WtfAbi::String&& origin, NKWebKit::WtfAbi::Vector<NKWebKit::PasteboardAbi::Entry>&& data);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataC2EON3WTF6StringEONS1_6VectorINS0_5EntryELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEE(NKWebKit::PasteboardAbi::PasteboardCustomData* self, NKWebKit::WtfAbi::String&& origin, NKWebKit::WtfAbi::Vector<NKWebKit::PasteboardAbi::Entry>&& data);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataC1ERKS0_(NKWebKit::PasteboardAbi::PasteboardCustomData* self, const NKWebKit::PasteboardAbi::PasteboardCustomData& other);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataC2ERKS0_(NKWebKit::PasteboardAbi::PasteboardCustomData* self, const NKWebKit::PasteboardAbi::PasteboardCustomData& other);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataC1EOS0_(NKWebKit::PasteboardAbi::PasteboardCustomData* self, NKWebKit::PasteboardAbi::PasteboardCustomData&& other);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataC2EOS0_(NKWebKit::PasteboardAbi::PasteboardCustomData* self, NKWebKit::PasteboardAbi::PasteboardCustomData&& other);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataD1Ev(NKWebKit::PasteboardAbi::PasteboardCustomData* self);
void APS5_VABI _ZN7WebCore20PasteboardCustomDataD2Ev(NKWebKit::PasteboardAbi::PasteboardCustomData* self);
NKWebKit::PasteboardAbi::PasteboardCustomData* APS5_VABI _ZN7WebCore20PasteboardCustomDataaSERKS0_(NKWebKit::PasteboardAbi::PasteboardCustomData* self, const NKWebKit::PasteboardAbi::PasteboardCustomData& other);

}
