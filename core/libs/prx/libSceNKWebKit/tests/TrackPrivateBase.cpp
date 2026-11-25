#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>

using NKWebKit::WtfAbi::AtomString;
using NKWebKit::WtfAbi::String;
using NKWebKit::WtfAbi::StringImpl;

static void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "TrackPrivateBase check failed at line %d\n", line);
        std::abort();
    }
}
#define Require(value) Check((value), __LINE__)

namespace {

using AtomGetter = AtomString (APS5_VABI *)(const void*);

StringImpl* EmptyImpl() {
    String empty = _ZN3WTF10StringImpl6createEPKhj(nullptr, 0);
    return empty.impl;
}

void TestEmptyAtom(AtomGetter getter) {
    StringImpl* empty = EmptyImpl();
    Require(empty != nullptr);
    Require(empty->length == 0);
    Require((empty->refCount & 1) == 1);
    Require((empty->hashAndFlags & 16) != 0);
    Require((empty->hashAndFlags & 4) != 0);
    const unsigned before = empty->refCount;

    std::array<std::uint8_t, 64> object;
    object.fill(0x3c);
    const auto objectBefore = object;

    alignas(AtomString) unsigned char raw[sizeof(AtomString)];
    std::memset(raw, 0xa5, sizeof(raw));
    auto* first = ::new (raw) AtomString(getter(object.data()));
    Require(first->string.impl == empty);
    Require(empty->refCount == before + 2);
    Require(object == objectBefore);

    alignas(AtomString) unsigned char rawNull[sizeof(AtomString)];
    std::memset(rawNull, 0x5a, sizeof(rawNull));
    auto* second = ::new (rawNull) AtomString(getter(nullptr));
    Require(second->string.impl == empty);
    Require(empty->refCount == before + 4);

    first->~AtomString();
    second->~AtomString();
    Require(empty->refCount == before);
}

void TestStartTimeVariance() {
    const WTF::MediaTime& zero = _ZN3WTF9MediaTime8zeroTimeEv();
    std::array<std::uint8_t, 64> object;
    object.fill(0x3c);
    const auto objectBefore = object;

    alignas(WTF::MediaTime) unsigned char raw[sizeof(WTF::MediaTime)];
    std::memset(raw, 0xa5, sizeof(raw));
    auto* time = ::new (raw) WTF::MediaTime(_ZNK7WebCore16TrackPrivateBase17startTimeVarianceEv(object.data()));
    Require(time != &zero);
    std::int64_t value;
    std::uint32_t scale;
    std::memcpy(&value, raw, sizeof(value));
    std::memcpy(&scale, raw + 8, sizeof(scale));
    Require(value == 0);
    Require(scale == 1);
    Require(raw[12] == WTF::MediaTime::Valid);
    Require(time->m_timeValue == zero.m_timeValue);
    Require(time->m_timeScale == zero.m_timeScale);
    Require(time->m_timeFlags == zero.m_timeFlags);
    Require(object == objectBefore);

    WTF::MediaTime fromNull = _ZNK7WebCore16TrackPrivateBase17startTimeVarianceEv(nullptr);
    Require(fromNull.m_timeValue == 0 && fromNull.m_timeScale == 1 && fromNull.m_timeFlags == WTF::MediaTime::Valid);
}

void TestConstructorNullSelf() {
    bool threw = false;
    try {
        _ZN7WebCore16TrackPrivateBaseC2Ev(nullptr);
    } catch (const std::invalid_argument& e) {
        threw = true;
        Require(std::strcmp(e.what(), "WebCore::TrackPrivateBase::TrackPrivateBase: null object") == 0);
    }
    Require(threw);
}

void TestConstructor() {
    alignas(8) std::array<std::uint8_t, 32> object;
    object.fill(0xa5);
    _ZN7WebCore16TrackPrivateBaseC2Ev(object.data());

    const void* const* vptr;
    std::memcpy(&vptr, object.data(), sizeof(vptr));
    Require(vptr != nullptr);
    Require(vptr[-2] == nullptr);
    Require(vptr[-1] == nullptr);
    for (std::size_t i = 0; i < 8; ++i)
        Require(vptr[i] != nullptr);
    Require(vptr[0] != vptr[1] && vptr[0] != vptr[2] && vptr[1] != vptr[2]);
    for (std::size_t i = 3; i < 6; ++i)
        TestEmptyAtom(reinterpret_cast<AtomGetter>(const_cast<void*>(vptr[i])));
    Require(reinterpret_cast<int (APS5_VABI *)()>(const_cast<void*>(vptr[6]))() == 0);
    const WTF::MediaTime variance = reinterpret_cast<WTF::MediaTime (APS5_VABI *)(const void*)>(const_cast<void*>(vptr[7]))(object.data());
    Require(variance.m_timeValue == 0 && variance.m_timeScale == 1 && variance.m_timeFlags == WTF::MediaTime::Valid);

    std::uint32_t refCount;
    std::memcpy(&refCount, object.data() + 8, sizeof(refCount));
    Require(refCount == 1);
    for (std::size_t i = 12; i < object.size(); ++i)
        Require(object[i] == 0xa5);

    StringImpl* empty = EmptyImpl();
    const unsigned before = empty->refCount;
    {
        AtomString label = reinterpret_cast<AtomGetter>(const_cast<void*>(vptr[4]))(object.data());
        Require(label.string.impl == empty);
        Require(empty->refCount == before + 2);
    }
    Require(empty->refCount == before);

    _ZN7WebCore16TrackPrivateBaseD2Ev();
    std::memcpy(&refCount, object.data() + 8, sizeof(refCount));
    Require(refCount == 1);

    object.fill(0x00);
    _ZN7WebCore16TrackPrivateBaseC2Ev(object.data());
    const void* const* again;
    std::memcpy(&again, object.data(), sizeof(again));
    Require(again == vptr);
    std::memcpy(&refCount, object.data() + 8, sizeof(refCount));
    Require(refCount == 1);
    for (std::size_t i = 12; i < object.size(); ++i)
        Require(object[i] == 0);
}

struct FakeClient {
    const void* vptr;
    void* removed = nullptr;
    int calls = 0;
};

FakeClient* g_client = nullptr;

void* APS5_VABI ReturnClient(const void*) {
    return g_client;
}

void APS5_VABI WillRemove(void* client, void* track) {
    auto* fake = static_cast<FakeClient*>(client);
    fake->removed = track;
    ++fake->calls;
}

void APS5_VABI Unexpected(void*) {
    Require(false);
}

void TestWillBeRemoved() {
    const void* clientVtable[6] = {reinterpret_cast<const void*>(&Unexpected), reinterpret_cast<const void*>(&Unexpected), nullptr, nullptr, nullptr, reinterpret_cast<const void*>(&WillRemove)};
    FakeClient client{clientVtable};

    alignas(8) std::array<std::uint8_t, 16> object;
    _ZN7WebCore16TrackPrivateBaseC2Ev(object.data());
    const void* const* baseVtable;
    std::memcpy(&baseVtable, object.data(), sizeof(baseVtable));
    std::array<const void*, 8> derived;
    for (std::size_t i = 0; i < derived.size(); ++i) derived[i] = baseVtable[i];
    derived[2] = reinterpret_cast<const void*>(&ReturnClient);
    const void* derivedPointer = derived.data();
    std::memcpy(object.data(), &derivedPointer, sizeof(derivedPointer));

    g_client = &client;
    _ZN7WebCore16TrackPrivateBase13willBeRemovedEv(object.data());
    Require(client.calls == 1 && client.removed == object.data());

    g_client = nullptr;
    _ZN7WebCore16TrackPrivateBase13willBeRemovedEv(object.data());
    Require(client.calls == 1);
}

void TestDestructors() {
    alignas(8) std::array<std::uint8_t, 16> object;
    object.fill(0x5a);
    _ZN7WebCore16TrackPrivateBaseC2Ev(object.data());
    const std::array<std::uint8_t, 16> constructed = object;
    _ZN7WebCore16TrackPrivateBaseD1Ev(object.data());
    Require(object == constructed);

    void* heap = _ZN3WTF10fastMallocEm(16);
    _ZN7WebCore16TrackPrivateBaseC2Ev(heap);
    const void* const* vptr;
    std::memcpy(&vptr, heap, sizeof(vptr));
    reinterpret_cast<void (APS5_VABI*)(void*)>(const_cast<void*>(vptr[1]))(heap);

    heap = _ZN3WTF10fastMallocEm(16);
    _ZN7WebCore16TrackPrivateBaseC2Ev(heap);
    reinterpret_cast<void (APS5_VABI*)(void*)>(const_cast<void*>(vptr[0]))(heap);
    _ZN7WebCore16TrackPrivateBaseD0Ev(heap);
}

template<typename TFunction>
bool ThrowsInvalidArgument(TFunction&& function) {
    try {
        function();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

void TestNullSelf() {
    Require(ThrowsInvalidArgument([] { _ZN7WebCore16TrackPrivateBaseD0Ev(nullptr); }));
    Require(ThrowsInvalidArgument([] { _ZN7WebCore16TrackPrivateBaseD1Ev(nullptr); }));
    Require(ThrowsInvalidArgument([] { _ZN7WebCore16TrackPrivateBase13willBeRemovedEv(nullptr); }));
}

}

int main() {
    TestConstructorNullSelf();
    TestConstructor();
    TestEmptyAtom(_ZNK7WebCore16TrackPrivateBase2idEv);
    TestEmptyAtom(_ZNK7WebCore16TrackPrivateBase5labelEv);
    TestEmptyAtom(_ZNK7WebCore16TrackPrivateBase8languageEv);
    TestStartTimeVariance();
    TestWillBeRemoved();
    TestDestructors();
    TestNullSelf();
    return 0;
}
