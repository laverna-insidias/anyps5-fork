#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace {

using NKWebKit::CoordinatedAbi::Buffer;
using NKWebKit::CoordinatedAbi::CoordinatedBackingStore;
using NKWebKit::NicosiaAbi::BackingStoreContainer;
using NKWebKit::NicosiaAbi::ImageBackingStore;
using NKWebKit::NicosiaAbi::ImageBackingTextureMapperImpl;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "NicosiaImageBacking check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(value) Check((value), __LINE__)

template<typename T>
T* Allocate() {
    return static_cast<T*>(_ZN3WTF10fastMallocEm(sizeof(T)));
}

CoordinatedBackingStore* CreateBackingStore() {
    auto ref = _ZN7WebCore23CoordinatedBackingStore6createEv();
    return ref.ptr;
}

ImageBackingStore* CreateStore(CoordinatedBackingStore* backingStore, Buffer** bufferOut) {
    WebCore::IntSize size{2, 2};
    Buffer* buffer = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&size, 0).ptr;
    auto* container = Allocate<BackingStoreContainer>();
    container->refCount = 1;
    container->backingStore = backingStore;
    ++backingStore->refCount;
    auto* store = Allocate<ImageBackingStore>();
    store->refCount = 1;
    store->buffer = buffer;
    store->container = container;
    if (bufferOut) *bufferOut = buffer;
    return store;
}

void InitImpl(ImageBackingTextureMapperImpl& impl) {
    impl.vtable = nullptr;
    impl.imageID = 0;
    impl.layerUpdate.isVisible = false;
    impl.layerUpdate.nativeImageID = 0;
    impl.layerUpdate.imageBackingStore = nullptr;
    impl.compositionStore = nullptr;
    impl.lock = 0;
    impl.update.isVisible = false;
    impl.update.nativeImageID = 0;
    impl.update.imageBackingStore = nullptr;
}

void TestSameImageKeepsStore() {
    CoordinatedBackingStore* backingStore = CreateBackingStore();
    ImageBackingTextureMapperImpl impl;
    InitImpl(impl);
    impl.layerUpdate.nativeImageID = 5;
    impl.layerUpdate.isVisible = true;
    impl.layerUpdate.imageBackingStore = CreateStore(backingStore, nullptr);
    ImageBackingStore* store = impl.layerUpdate.imageBackingStore;

    _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(&impl);
    Require(impl.lock == 0);
    Require(impl.update.imageBackingStore == store && !impl.layerUpdate.imageBackingStore);
    Require(impl.update.nativeImageID == 5 && impl.update.isVisible);
    Require(impl.layerUpdate.nativeImageID == 5);

    impl.layerUpdate.isVisible = false;
    _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(&impl);
    Require(impl.update.imageBackingStore == store && !impl.update.isVisible);

    auto taken = _ZN7Nicosia29ImageBackingTextureMapperImpl10takeUpdateEv(&impl);
    Require(impl.lock == 0);
    Require(taken.imageBackingStore == store && taken.nativeImageID == 5 && !taken.isVisible);
    Require(!impl.update.imageBackingStore && impl.update.nativeImageID == 5);

    auto empty = _ZN7Nicosia29ImageBackingTextureMapperImpl10takeUpdateEv(&impl);
    Require(!empty.imageBackingStore);

    impl.update.imageBackingStore = taken.imageBackingStore;
    Require(backingStore->refCount == 2);
    impl.layerUpdate.nativeImageID = 6;
    _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(&impl);
    Require(!impl.update.imageBackingStore && impl.update.nativeImageID == 6);
    Require(backingStore->refCount == 1);
    _ZN7WebCore23CoordinatedBackingStoreD0Ev(backingStore);
}

void TestReplacementReleasesSharedStore() {
    CoordinatedBackingStore* backingStore = CreateBackingStore();
    Buffer* buffer = nullptr;
    ImageBackingTextureMapperImpl impl;
    InitImpl(impl);
    impl.update.nativeImageID = 1;
    impl.update.imageBackingStore = CreateStore(backingStore, &buffer);
    ImageBackingStore* shared = impl.update.imageBackingStore;
    ++shared->refCount;
    ++buffer->refCount;

    impl.layerUpdate.nativeImageID = 2;
    impl.layerUpdate.imageBackingStore = CreateStore(backingStore, nullptr);
    ImageBackingStore* replacement = impl.layerUpdate.imageBackingStore;
    Require(backingStore->refCount == 3);

    _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(&impl);
    Require(impl.update.imageBackingStore == replacement && impl.update.nativeImageID == 2);
    Require(shared->refCount == 1 && backingStore->refCount == 3);

    impl.layerUpdate.nativeImageID = 3;
    impl.layerUpdate.imageBackingStore = shared;
    _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(&impl);
    Require(impl.update.imageBackingStore == shared);
    Require(backingStore->refCount == 2);

    impl.layerUpdate.nativeImageID = 4;
    _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(&impl);
    Require(!impl.update.imageBackingStore);
    Require(buffer->refCount == 1 && backingStore->refCount == 1);
    _ZN7Nicosia6BufferD1Ev(buffer);
    _ZN3WTF8fastFreeEPv(buffer);
    _ZN7WebCore23CoordinatedBackingStoreD0Ev(backingStore);
}

void TestNullThis() {
    bool threw = false;
    try {
        _ZN7Nicosia29ImageBackingTextureMapperImpl11flushUpdateEv(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
    threw = false;
    try {
        _ZN7Nicosia29ImageBackingTextureMapperImpl10takeUpdateEv(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
}

}  // namespace

int main() {
    TestSameImageKeepsStore();
    TestReplacementReleasesSharedStore();
    TestNullThis();
    return 0;
}
