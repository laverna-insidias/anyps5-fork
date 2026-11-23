#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace {

using NKWebKit::CoordinatedAbi::Buffer;
using NKWebKit::NicosiaAbi::BackingStoreTextureMapperImpl;
using NKWebKit::NicosiaAbi::TileCreationData;
using NKWebKit::NicosiaAbi::TileRemovalData;
using NKWebKit::NicosiaAbi::TileUpdateData;
using NKWebKit::NicosiaAbi::TileVector;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "NicosiaBackingStore check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(value) Check((value), __LINE__)

template<typename T>
void Append(TileVector<T>& vector, const T& value) {
    if (vector.size == vector.capacity) {
        const unsigned capacity = vector.capacity ? vector.capacity * 2 : 4;
        auto* buffer = static_cast<T*>(_ZN3WTF10fastMallocEm(capacity * sizeof(T)));
        for (unsigned i = 0; i < vector.size; ++i) buffer[i] = vector.buffer[i];
        if (vector.buffer) _ZN3WTF8fastFreeEPv(vector.buffer);
        vector.buffer = buffer;
        vector.capacity = capacity;
    }
    vector.buffer[vector.size++] = value;
}

template<typename T>
void Free(TileVector<T>& vector) {
    if (vector.buffer) _ZN3WTF8fastFreeEPv(vector.buffer);
    vector = {};
}

Buffer* MakeBuffer() {
    WebCore::IntSize size{1, 1};
    return _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&size, 0).ptr;
}

void Init(BackingStoreTextureMapperImpl& impl) {
    impl.vtable = nullptr;
    impl.tiledBackingStoreClientVtable = nullptr;
    impl.mainBackingStore = nullptr;
    impl.layerUpdate.tilesToCreate = {};
    impl.layerUpdate.tilesToUpdate = {};
    impl.layerUpdate.tilesToRemove = {};
    impl.isFlushing = false;
    impl.isPurging = false;
    impl.hasPendingTileCreation = true;
    impl.backingStore = nullptr;
    impl.lock = 0;
    impl.pending.tilesToCreate = {};
    impl.pending.tilesToUpdate = {};
    impl.pending.tilesToRemove = {};
}

void TestFlushAndTake() {
    BackingStoreTextureMapperImpl impl;
    Init(impl);
    Buffer* buffer = MakeBuffer();

    _ZN7Nicosia29BackingStoreTextureMapperImpl11flushUpdateEv(&impl);
    Require(!impl.hasPendingTileCreation && impl.lock == 0);
    Require(!impl.pending.tilesToCreate.buffer && !impl.pending.tilesToUpdate.buffer && !impl.pending.tilesToRemove.buffer);

    for (unsigned i = 1; i <= 20; ++i) Append(impl.layerUpdate.tilesToCreate, TileCreationData{i, 2.0f});
    Append(impl.layerUpdate.tilesToUpdate, TileUpdateData{3, {{1, 2}, {3, 4}}, {{{5, 6}, {7, 8}}, buffer}});
    Append(impl.layerUpdate.tilesToRemove, TileRemovalData{9});
    impl.hasPendingTileCreation = true;
    _ZN7Nicosia29BackingStoreTextureMapperImpl11flushUpdateEv(&impl);
    Require(!impl.hasPendingTileCreation);
    Require(!impl.layerUpdate.tilesToCreate.buffer && !impl.layerUpdate.tilesToCreate.size && !impl.layerUpdate.tilesToCreate.capacity);
    Require(!impl.layerUpdate.tilesToUpdate.buffer && !impl.layerUpdate.tilesToRemove.buffer);
    Require(impl.pending.tilesToCreate.size == 20 && impl.pending.tilesToCreate.capacity == 20);
    Require(impl.pending.tilesToUpdate.size == 1 && impl.pending.tilesToUpdate.capacity == 16);
    Require(impl.pending.tilesToRemove.size == 1 && impl.pending.tilesToRemove.capacity == 16);
    Require(buffer->refCount == 1);

    Append(impl.layerUpdate.tilesToCreate, TileCreationData{21, 1.0f});
    Append(impl.layerUpdate.tilesToUpdate, TileUpdateData{4, {}, {{}, buffer}});
    ++buffer->refCount;
    _ZN7Nicosia29BackingStoreTextureMapperImpl11flushUpdateEv(&impl);
    Require(impl.pending.tilesToCreate.size == 21 && impl.pending.tilesToCreate.capacity == 26);
    Require(impl.pending.tilesToCreate.buffer[0].tileID == 1 && impl.pending.tilesToCreate.buffer[20].tileID == 21);
    Require(impl.pending.tilesToUpdate.size == 2 && buffer->refCount == 2);

    auto update = _ZN7Nicosia29BackingStoreTextureMapperImpl10takeUpdateEv(&impl);
    Require(impl.lock == 0);
    Require(!impl.pending.tilesToCreate.buffer && !impl.pending.tilesToUpdate.size && !impl.pending.tilesToRemove.capacity);
    Require(update.tilesToCreate.size == 21 && update.tilesToCreate.buffer[5].scale == 2.0f);
    Require(update.tilesToUpdate.size == 2);
    const auto& first = update.tilesToUpdate.buffer[0];
    Require(first.tileID == 3 && first.tileRect.m_location.m_x == 1 && first.tileRect.m_size.m_height == 4);
    Require(first.updateInfo.updateRect.m_size.m_width == 7 && first.updateInfo.buffer == buffer);
    Require(update.tilesToRemove.size == 1 && update.tilesToRemove.buffer[0].tileID == 9);

    auto empty = _ZN7Nicosia29BackingStoreTextureMapperImpl10takeUpdateEv(&impl);
    Require(!empty.tilesToCreate.buffer && !empty.tilesToUpdate.buffer && !empty.tilesToRemove.buffer);

    Free(update.tilesToCreate);
    Free(update.tilesToUpdate);
    Free(update.tilesToRemove);
    buffer->refCount = 1;
    _ZN7Nicosia6BufferD1Ev(buffer);
    _ZN3WTF8fastFreeEPv(buffer);
}

void TestLayerUpdateRelease() {
    BackingStoreTextureMapperImpl impl;
    Init(impl);
    Buffer* buffer = MakeBuffer();
    Append(impl.layerUpdate.tilesToUpdate, TileUpdateData{1, {}, {{}, buffer}});
    _ZN7Nicosia29BackingStoreTextureMapperImpl11flushUpdateEv(&impl);
    Require(buffer->refCount == 1);
    Require(impl.pending.tilesToUpdate.buffer[0].updateInfo.buffer == buffer);
    Free(impl.pending.tilesToUpdate);
    _ZN7Nicosia6BufferD1Ev(buffer);
    _ZN3WTF8fastFreeEPv(buffer);
}

void TestNullThis() {
    bool threw = false;
    try {
        _ZN7Nicosia29BackingStoreTextureMapperImpl11flushUpdateEv(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
    threw = false;
    try {
        _ZN7Nicosia29BackingStoreTextureMapperImpl10takeUpdateEv(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
}

}  // namespace

int main() {
    TestFlushAndTake();
    TestLayerUpdateRelease();
    TestNullThis();
    return 0;
}
