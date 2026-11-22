#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

namespace {

using NKWebKit::CoordinatedAbi::Buffer;
using NKWebKit::CoordinatedAbi::BufferRefPtr;
using NKWebKit::CoordinatedAbi::CoordinatedBackingStore;
using NKWebKit::CoordinatedAbi::TextureMapperTile;
using NKWebKit::CoordinatedAbi::Tile;
using NKWebKit::CoordinatedAbi::TileBucket;
using NKWebKit::CoordinatedAbi::TilePointerVector;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "CoordinatedBackingStore check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(value) Check((value), __LINE__)

template<typename Function>
void RequireInvalid(Function function) {
    bool threw = false;
    try {
        function();
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
}

template<typename Function>
void RequireNotImplemented(Function function) {
    bool threw = false;
    try {
        function();
    } catch (const std::invalid_argument&) {
    } catch (const std::runtime_error& error) {
        threw = std::string(error.what()).find("not implemented") != std::string::npos;
    }
    Require(threw);
}

unsigned IntHash(std::uint32_t key) {
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}

const unsigned* Metadata(const void* table) {
    return static_cast<const unsigned*>(table);
}

unsigned TableSize(const void* table) {
    return table ? Metadata(table)[-1] : 0U;
}

unsigned KeyCount(const void* table) {
    return table ? Metadata(table)[-3] : 0U;
}

Tile* FindTile(CoordinatedBackingStore& store, unsigned id) {
    for (unsigned i = 0; i < TableSize(store.tiles.table); ++i)
        if (store.tiles.table[i].key == id) return &store.tiles.table[i].value;
    return nullptr;
}

bool Pending(CoordinatedBackingStore& store, unsigned id) {
    for (unsigned i = 0; i < TableSize(store.tilesToRemove.table); ++i)
        if (store.tilesToRemove.table[i] == id) return true;
    return false;
}

CoordinatedBackingStore* Create() {
    auto ref = _ZN7WebCore23CoordinatedBackingStore6createEv();
    return ref.ptr;
}

void Destroy(CoordinatedBackingStore* store) {
    reinterpret_cast<void(APS5_VABI*)(CoordinatedBackingStore*)>(const_cast<void*>(store->vtable[1]))(store);
}

Buffer* MakeBuffer(int width, int height) {
    WebCore::IntSize size{width, height};
    auto ref = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&size, 1);
    return ref.ptr;
}

void DestroyBuffer(Buffer* buffer) {
    _ZN7Nicosia6BufferD1Ev(buffer);
    _ZN3WTF8fastFreeEPv(buffer);
}

void Update(CoordinatedBackingStore* store, unsigned id, Buffer* buffer) {
    WebCore::IntRect source{{0, 0}, {4, 4}};
    WebCore::IntRect target{{8, 8}, {4, 4}};
    WebCore::IntPoint offset{1, 2};
    BufferRefPtr ref{buffer};
    _ZN7WebCore23CoordinatedBackingStore10updateTileEjRKNS_7IntRectES3_ON3WTF6RefPtrIN7Nicosia6BufferENS4_13DumbPtrTraitsIS7_EEEERKNS_8IntPointE(store, id, &source, &target, &ref, &offset);
    Require(ref.ptr == nullptr);
}

void Commit(CoordinatedBackingStore* store) {
    _ZN7WebCore23CoordinatedBackingStore20commitTileOperationsERNS_13TextureMapperE(store, nullptr);
}

void TestCreate() {
    CoordinatedBackingStore* store = Create();
    Require(store->refCount == 1);
    Require(store->client == nullptr);
    Require(store->tiles.table == nullptr && store->tilesToRemove.table == nullptr);
    Require(store->scale == 1.0f);
    Require(store->vtable[0] == reinterpret_cast<const void*>(&_ZN7WebCore23CoordinatedBackingStoreD1Ev));
    Require(store->vtable[1] == reinterpret_cast<const void*>(&_ZN7WebCore23CoordinatedBackingStoreD0Ev));
    WebCore::FloatRect rect = _ZNK7WebCore23CoordinatedBackingStore4rectEv(store);
    Require(rect.x() == 0 && rect.y() == 0 && rect.width() == 0 && rect.height() == 0);
    Destroy(store);

    CoordinatedBackingStore inPlace;
    std::memset(&inPlace, 0xab, sizeof(inPlace));
    _ZN7WebCore23CoordinatedBackingStoreC2Ev(&inPlace);
    Require(inPlace.refCount == 1 && inPlace.tiles.table == nullptr && inPlace.scale == 1.0f);
    _ZN7WebCore23CoordinatedBackingStoreD2Ev(&inPlace);

    RequireInvalid([] { _ZN7WebCore23CoordinatedBackingStoreC1Ev(nullptr); });
    RequireInvalid([] { _ZN7WebCore23CoordinatedBackingStoreD1Ev(nullptr); });
    RequireInvalid([] { _ZNK7WebCore23CoordinatedBackingStore4rectEv(nullptr); });
}

void TestCreateTile() {
    CoordinatedBackingStore* store = Create();
    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 7, 2.0f);
    Require(TableSize(store->tiles.table) == 8);
    Require(Metadata(store->tiles.table)[-2] == 7);
    Require(KeyCount(store->tiles.table) == 1);
    TileBucket& bucket = store->tiles.table[IntHash(7) & 7];
    Require(bucket.key == 7 && bucket.value.scale == 2.0f && bucket.value.base.texture == nullptr && bucket.value.updates == nullptr);
    Require(store->scale == 2.0f);
    for (unsigned i = 0; i < 8; ++i) {
        if (store->tiles.table[i].key) continue;
        Require(store->tiles.table[i].value.scale == 1.0f);
        Require(store->tiles.table[i].value.base.vtable == bucket.value.base.vtable);
    }
    Require(bucket.value.base.vtable[0] == reinterpret_cast<const void*>(&_ZN7WebCore17TextureMapperTile5paintERNS_13TextureMapperERKNS_20TransformationMatrixEfj));

    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 7, 3.0f);
    Require(KeyCount(store->tiles.table) == 1);
    Require(FindTile(*store, 7)->scale == 2.0f);
    Require(store->scale == 3.0f);

    for (unsigned id = 1; id <= 40; ++id) _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, id, float(id));
    Require(KeyCount(store->tiles.table) == 40);
    Require(TableSize(store->tiles.table) == 64);
    for (unsigned id = 1; id <= 40; ++id) Require(FindTile(*store, id) && FindTile(*store, id)->scale == (id == 7 ? 2.0f : float(id)));
    Require(store->scale == 40.0f);

    RequireInvalid([&] { _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 0, 1.0f); });
    RequireInvalid([&] { _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, ~0U, 1.0f); });
    RequireInvalid([&] { _ZN7WebCore23CoordinatedBackingStore10removeTileEj(store, 0); });
    RequireInvalid([] { _ZN7WebCore23CoordinatedBackingStore10createTileEjf(nullptr, 1, 1.0f); });
    Require(store->scale == 40.0f);
    Destroy(store);
}

void TestRemove() {
    CoordinatedBackingStore* store = Create();
    for (unsigned id = 1; id <= 40; ++id) _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, id, 1.0f);
    for (unsigned id = 4; id <= 40; ++id) _ZN7WebCore23CoordinatedBackingStore10removeTileEj(store, id);
    _ZN7WebCore23CoordinatedBackingStore10removeTileEj(store, 99);
    Require(KeyCount(store->tilesToRemove.table) == 38);
    Require(Pending(*store, 99));
    Require(KeyCount(store->tiles.table) == 40);
    Commit(store);
    Require(store->tilesToRemove.table == nullptr);
    Require(KeyCount(store->tiles.table) == 3);
    Require(TableSize(store->tiles.table) == 16);
    for (unsigned id = 1; id <= 3; ++id) Require(FindTile(*store, id));
    Require(!FindTile(*store, 4));

    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 50, 1.0f);
    _ZN7WebCore23CoordinatedBackingStore14removeAllTilesEv(store);
    Require(KeyCount(store->tilesToRemove.table) == 4);
    Require(Pending(*store, 1) && Pending(*store, 50));
    Commit(store);
    Require(KeyCount(store->tiles.table) == 0);
    Require(TableSize(store->tiles.table) == 8);
    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 5, 1.0f);
    Require(KeyCount(store->tiles.table) == 1 && FindTile(*store, 5));
    Require(Metadata(store->tiles.table)[-4] <= 2);
    Destroy(store);
}

void TestSize() {
    CoordinatedBackingStore* store = Create();
    WebCore::FloatSize size{100.0f, 50.0f};
    _ZN7WebCore23CoordinatedBackingStore7setSizeERKNS_9FloatSizeE(store, &size);
    Require(_ZNK7WebCore23CoordinatedBackingStore4rectEv(store).width() == 0);
    Commit(store);
    WebCore::FloatRect rect = _ZNK7WebCore23CoordinatedBackingStore4rectEv(store);
    Require(rect.x() == 0 && rect.y() == 0 && rect.width() == 100.0f && rect.height() == 50.0f);
    Require(store->pendingSize.m_width == 0 && store->pendingSize.m_height == 0);
    Commit(store);
    Require(_ZNK7WebCore23CoordinatedBackingStore4rectEv(store).width() == 100.0f);
    WebCore::FloatSize narrow{0.0f, 20.0f};
    _ZN7WebCore23CoordinatedBackingStore7setSizeERKNS_9FloatSizeE(store, &narrow);
    Commit(store);
    Require(_ZNK7WebCore23CoordinatedBackingStore4rectEv(store).width() == 0 && _ZNK7WebCore23CoordinatedBackingStore4rectEv(store).height() == 20.0f);
    _ZN7WebCore23CoordinatedBackingStore7setSizeERKNS_9FloatSizeE(store, &size);
    Commit(store);

    WebCore::FloatRect target{{10.0f, 20.0f}, {200.0f, 100.0f}};
    WebCore::TransformationMatrix adjusted = _ZN7WebCore23CoordinatedBackingStore24adjustedTransformForRectERKNS_9FloatRectE(store, &target);
    WebCore::FloatRect from{{0.0f, 0.0f}, {100.0f, 50.0f}};
    WebCore::TransformationMatrix expected = _ZN7WebCore20TransformationMatrix10rectToRectERKNS_9FloatRectES3_(from, target);
    Require(std::memcmp(&adjusted, &expected, sizeof(adjusted)) == 0);
    Require(adjusted.m_matrix[0][0] == 2.0 && adjusted.m_matrix[1][1] == 2.0);
    RequireInvalid([&] { _ZN7WebCore23CoordinatedBackingStore7setSizeERKNS_9FloatSizeE(store, nullptr); });
    RequireInvalid([&] { _ZN7WebCore23CoordinatedBackingStore24adjustedTransformForRectERKNS_9FloatRectE(store, nullptr); });
    Destroy(store);
}

void TestUpdates() {
    CoordinatedBackingStore* store = Create();
    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 1, 1.0f);
    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 2, 1.0f);
    for (unsigned i = 0; i < 16; ++i) Update(store, 1, nullptr);
    Tile* tile = FindTile(*store, 1);
    Require(tile->updateCount == 16 && tile->updateCapacity == 16);
    Update(store, 1, nullptr);
    Require(tile->updateCount == 17 && tile->updateCapacity == 21);
    Require(tile->updates[16].sourceRect.m_size.m_width == 4 && tile->updates[16].tileRect.m_location.m_x == 8);
    Require(tile->updates[16].bufferOffset.m_x == 1 && tile->updates[16].bufferOffset.m_y == 2);
    Commit(store);
    Require(tile->updateCount == 0 && tile->updates == nullptr);

    Buffer* buffer = MakeBuffer(4, 4);
    buffer->refCount = 2;
    Update(store, 2, buffer);
    Require(buffer->refCount == 2);
    WebCore::FloatSize size{10.0f, 10.0f};
    _ZN7WebCore23CoordinatedBackingStore7setSizeERKNS_9FloatSizeE(store, &size);
    RequireNotImplemented([&] { Commit(store); });
    Require(store->size.m_width == 0 && store->pendingSize.m_width == 10.0f);
    Require(FindTile(*store, 2)->updateCount == 1);

    BufferRefPtr kept{buffer};
    WebCore::IntRect rect{};
    WebCore::IntPoint offset{};
    RequireInvalid([&] {
        _ZN7WebCore23CoordinatedBackingStore10updateTileEjRKNS_7IntRectES3_ON3WTF6RefPtrIN7Nicosia6BufferENS4_13DumbPtrTraitsIS7_EEEERKNS_8IntPointE(store, 3, &rect, &rect, &kept, &offset);
    });
    Require(kept.ptr == buffer);

    _ZN7WebCore23CoordinatedBackingStore10removeTileEj(store, 2);
    Commit(store);
    Require(buffer->refCount == 1);
    Require(store->size.m_width == 10.0f);
    Require(!FindTile(*store, 2));

    Update(store, 1, buffer);
    Require(buffer->refCount == 1);
    Destroy(store);

    Buffer* owned = MakeBuffer(2, 2);
    CoordinatedBackingStore* second = Create();
    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(second, 9, 1.0f);
    owned->refCount = 2;
    Update(second, 9, owned);
    Destroy(second);
    Require(owned->refCount == 1);
    DestroyBuffer(owned);
}

void TestPaint() {
    CoordinatedBackingStore* store = Create();
    WebCore::FloatRect target{{0.0f, 0.0f}, {10.0f, 10.0f}};
    WebCore::TransformationMatrix transform{};
    _ZN7WebCore23CoordinatedBackingStore20paintToTextureMapperERNS_13TextureMapperERKNS_9FloatRectERKNS_20TransformationMatrixEf(store, nullptr, &target, &transform, 1.0f);
    _ZN7WebCore23CoordinatedBackingStore10drawBorderERNS_13TextureMapperERKNS_5ColorEfRKNS_9FloatRectERKNS_20TransformationMatrixE(store, nullptr, nullptr, 1.0f, &target, &transform);
    _ZN7WebCore23CoordinatedBackingStore18drawRepaintCounterERNS_13TextureMapperEiRKNS_5ColorERKNS_9FloatRectERKNS_20TransformationMatrixE(store, nullptr, 3, nullptr, &target, &transform);

    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 1, 1.0f);
    _ZN7WebCore23CoordinatedBackingStore20paintToTextureMapperERNS_13TextureMapperERKNS_9FloatRectERKNS_20TransformationMatrixEf(store, nullptr, &target, &transform, 1.0f);
    RequireNotImplemented([&] {
        _ZN7WebCore23CoordinatedBackingStore10drawBorderERNS_13TextureMapperERKNS_5ColorEfRKNS_9FloatRectERKNS_20TransformationMatrixE(store, nullptr, nullptr, 1.0f, &target, &transform);
    });
    RequireNotImplemented([&] {
        _ZN7WebCore23CoordinatedBackingStore18drawRepaintCounterERNS_13TextureMapperEiRKNS_5ColorERKNS_9FloatRectERKNS_20TransformationMatrixE(store, nullptr, 3, nullptr, &target, &transform);
    });

    Tile* tile = FindTile(*store, 1);
    int texture = 0;
    tile->base.texture = &texture;
    RequireNotImplemented([&] {
        _ZN7WebCore23CoordinatedBackingStore20paintToTextureMapperERNS_13TextureMapperERKNS_9FloatRectERKNS_20TransformationMatrixEf(store, nullptr, &target, &transform, 1.0f);
    });
    RequireNotImplemented([&] { _ZN7WebCore23CoordinatedBackingStoreD1Ev(store); });
    Require(store->tiles.table != nullptr);
    tile->base.texture = nullptr;
    Destroy(store);
}

struct PaintCall {
    TextureMapperTile* tile;
    const WebCore::TransformationMatrix* transform;
    float opacity;
    unsigned edges;
};

PaintCall calls[4];
unsigned callCount = 0;

void APS5_VABI RecordPaint(TextureMapperTile* tile, void*, const WebCore::TransformationMatrix* transform, float opacity, unsigned edges) {
    calls[callCount++] = PaintCall{tile, transform, opacity, edges};
}

void TestPaintTiles() {
    CoordinatedBackingStore* store = Create();
    const void* const vtable[] = {reinterpret_cast<const void*>(&RecordPaint)};
    TextureMapperTile first{vtable, nullptr, {{0.0f, 0.0f}, {5.0f, 5.0f}}};
    TextureMapperTile second{vtable, nullptr, {{5.0f, 5.0f}, {5.0f, 5.0f}}};
    TextureMapperTile* list[] = {&first, &second};
    TilePointerVector tiles{list, 2, 2};
    WebCore::FloatRect total{{0.0f, 0.0f}, {10.0f, 10.0f}};
    WebCore::TransformationMatrix transform{};
    _ZN7WebCore23CoordinatedBackingStore25paintTilesToTextureMapperERN3WTF6VectorIPNS_17TextureMapperTileELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEERNS_13TextureMapperERKNS_20TransformationMatrixEfRKNS_9FloatRectE(store, &tiles, nullptr, &transform, 0.5f, &total);
    Require(callCount == 2);
    Require(calls[0].tile == &first && calls[1].tile == &second);
    Require(calls[0].transform == &transform && calls[0].opacity == 0.5f);
    Require(calls[0].edges == _ZN7WebCore25TextureMapperBackingStore25calculateExposedTileEdgesERKNS_9FloatRectES3_(total, first.rect));
    Require(calls[1].edges == _ZN7WebCore25TextureMapperBackingStore25calculateExposedTileEdgesERKNS_9FloatRectES3_(total, second.rect));
    Require(calls[0].edges != calls[1].edges);

    _ZN7WebCore23CoordinatedBackingStore10createTileEjf(store, 1, 1.0f);
    TextureMapperTile* real[] = {&FindTile(*store, 1)->base};
    TilePointerVector realTiles{real, 1, 1};
    RequireNotImplemented([&] {
        _ZN7WebCore23CoordinatedBackingStore25paintTilesToTextureMapperERN3WTF6VectorIPNS_17TextureMapperTileELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEERNS_13TextureMapperERKNS_20TransformationMatrixEfRKNS_9FloatRectE(store, &realTiles, nullptr, &transform, 1.0f, &total);
    });
    Destroy(store);
}

void TestBuffer() {
    Buffer* buffer = MakeBuffer(3, -2);
    Require(buffer->refCount == 1);
    Require(buffer->size.m_width == 3 && buffer->size.m_height == -2);
    Require(buffer->flags == 1);
    Require(buffer->paintingState == NKWebKit::CoordinatedAbi::PaintingComplete);
    Require(buffer->data != nullptr);
    for (int i = 0; i < 24; ++i) Require(buffer->data[i] == 0);
    _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(buffer);
    buffer->paintingState = NKWebKit::CoordinatedAbi::PaintingInProgress;
    RequireNotImplemented([&] { _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(buffer); });
    buffer->paintingState = NKWebKit::CoordinatedAbi::PaintingComplete;
    _ZN7Nicosia6BufferD2Ev(buffer);
    Require(buffer->data == nullptr);
    _ZN7Nicosia6BufferD1Ev(buffer);
    _ZN3WTF8fastFreeEPv(buffer);

    bool overflow = false;
    try {
        MakeBuffer(70000, 70000);
    } catch (const std::overflow_error&) {
        overflow = true;
    }
    Require(overflow);
    RequireInvalid([] { _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(nullptr, 0); });
    RequireInvalid([] { _ZN7Nicosia6BufferD1Ev(nullptr); });
    RequireInvalid([] { _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(nullptr); });
}

}

int main() {
    TestCreate();
    TestCreateTile();
    TestRemove();
    TestSize();
    TestUpdates();
    TestPaint();
    TestPaintTiles();
    TestBuffer();
    return 0;
}
