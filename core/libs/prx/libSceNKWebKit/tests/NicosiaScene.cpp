#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"
#include "prx/libSceNKWebKit/src/WtfIntHashTable.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>

namespace {

namespace HashTableAbi = NKWebKit::WtfAbi::HashTableAbi;
using NKWebKit::NicosiaAbi::PlatformLayer;
using NKWebKit::NicosiaAbi::Scene;
using NKWebKit::NicosiaAbi::SceneIntegration;
using NKWebKit::NicosiaAbi::SceneIntegrationClient;
using NKWebKit::NicosiaAbi::SceneState;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "NicosiaScene check failed at line %d\n", line);
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

int destroyedLayers = 0;
int updateRequests = 0;

void APS5_VABI LayerDestroy(PlatformLayer*) {}

void APS5_VABI LayerDestroyAndCount(PlatformLayer*) {
    ++destroyedLayers;
}

const void* const LayerVtable[] = {
    reinterpret_cast<const void*>(&LayerDestroy),
    reinterpret_cast<const void*>(&LayerDestroyAndCount),
};

void APS5_VABI ClientRequestUpdate(SceneIntegrationClient*) {
    ++updateRequests;
}

const void* const ClientVtable[] = {nullptr, nullptr, reinterpret_cast<const void*>(&ClientRequestUpdate)};

struct TestLayerPolicy {
    using Bucket = PlatformLayer*;
    using Key = PlatformLayer*;
    static constexpr bool EmptyValueIsZero = true;
    static Key KeyOf(Bucket& bucket) { return bucket; }
    static unsigned Hash(Key key) { return HashTableAbi::PtrHash(key); }
    static bool IsEmpty(Bucket& bucket) { return !bucket; }
    static bool IsDeleted(Bucket& bucket) { return bucket == reinterpret_cast<PlatformLayer*>(~std::uintptr_t(0)); }
    static void SetDeleted(Bucket& bucket) { bucket = reinterpret_cast<PlatformLayer*>(~std::uintptr_t(0)); }
    static void ConstructEmpty(Bucket& bucket) { bucket = nullptr; }
    static void DestroyValue(Bucket&) {}
    static void Move(Bucket& from, Bucket& to) { to = from; }
};

void AddLayer(SceneState& state, PlatformLayer& layer) {
    ++layer.refCount;
    Require(HashTableAbi::Add<TestLayerPolicy>(state.layers.table, &layer, [&](PlatformLayer*& bucket) { bucket = &layer; }));
}

bool HasLayer(const SceneState& state, PlatformLayer& layer) {
    return HashTableAbi::Find<TestLayerPolicy>(state.layers.table, &layer) != nullptr;
}

void TestState() {
    SceneState state;
    _ZN7Nicosia5Scene5StateC1Ev(&state);
    Require(state.id == 0 && !state.platformLayerUpdated && !state.layers.table && !state.rootLayer);

    PlatformLayer layers[20];
    for (auto& layer : layers) {
        layer.vtable = LayerVtable;
        layer.refCount = 1;
        AddLayer(state, layer);
    }
    state.id = 7;
    state.platformLayerUpdated = true;
    state.rootLayer = &layers[0];
    ++layers[0].refCount;
    Require(HashTableAbi::KeyCount(state.layers.table) == 20);

    SceneState copy;
    _ZN7Nicosia5Scene5StateC2ERKS1_(&copy, &state);
    Require(copy.id == 7 && copy.platformLayerUpdated && copy.rootLayer == &layers[0]);
    Require(copy.layers.table != state.layers.table);
    Require(HashTableAbi::KeyCount(copy.layers.table) == 20);
    Require(HashTableAbi::TableSize(copy.layers.table) == 64);
    for (auto& layer : layers) Require(HasLayer(copy, layer));
    Require(layers[0].refCount == 5);
    Require(layers[1].refCount == 3);

    SceneState assigned;
    _ZN7Nicosia5Scene5StateC1Ev(&assigned);
    PlatformLayer other{LayerVtable, 1};
    AddLayer(assigned, other);
    assigned.rootLayer = &other;
    ++other.refCount;
    Require(_ZN7Nicosia5Scene5StateaSERKS1_(&assigned, &state) == &assigned);
    Require(other.refCount == 1);
    Require(assigned.id == 7 && assigned.rootLayer == &layers[0] && !HasLayer(assigned, other));
    for (auto& layer : layers) Require(HasLayer(assigned, layer));
    Require(layers[0].refCount == 7);
    Require(_ZN7Nicosia5Scene5StateaSERKS1_(&assigned, &assigned) == &assigned);
    Require(layers[0].refCount == 7 && HashTableAbi::KeyCount(assigned.layers.table) == 20);

    _ZN7Nicosia5Scene5StateD1Ev(&assigned);
    _ZN7Nicosia5Scene5StateD2Ev(&copy);
    Require(!copy.layers.table && !copy.rootLayer);
    Require(layers[0].refCount == 3 && layers[1].refCount == 2);
    Require(destroyedLayers == 0);

    for (auto& layer : layers) layer.refCount = 1;
    layers[0].refCount = 2;
    _ZN7Nicosia5Scene5StateD1Ev(&state);
    Require(destroyedLayers == 20);
    Require(other.refCount == 1);

    RequireInvalid([] { _ZN7Nicosia5Scene5StateC1Ev(nullptr); });
    RequireInvalid([&] { _ZN7Nicosia5Scene5StateC1ERKS1_(&copy, nullptr); });
    RequireInvalid([&] { _ZN7Nicosia5Scene5StateaSERKS1_(nullptr, &copy); });
    RequireInvalid([] { _ZN7Nicosia5Scene5StateD1Ev(nullptr); });
}

void TestScene() {
    destroyedLayers = 0;
    Scene* scene = _ZN7Nicosia5Scene6createEv().ptr;
    Require(scene->refCount == 1 && scene->lock == 0);
    Require(scene->state.id == 0 && !scene->state.layers.table && !scene->state.rootLayer);
    PlatformLayer layer{LayerVtable, 1};
    AddLayer(scene->state, layer);
    --layer.refCount;
    _ZN7Nicosia5SceneD2Ev(scene);
    Require(destroyedLayers == 1);
    _ZN3WTF8fastFreeEPv(scene);

    Scene local;
    _ZN7Nicosia5SceneC2Ev(&local);
    Require(local.refCount == 1);
    _ZN7Nicosia5SceneD1Ev(&local);
    RequireInvalid([] { _ZN7Nicosia5SceneC1Ev(nullptr); });
    RequireInvalid([] { _ZN7Nicosia5SceneD1Ev(nullptr); });
}

void TestSceneIntegration() {
    destroyedLayers = 0;
    updateRequests = 0;
    Scene* scene = _ZN7Nicosia5Scene6createEv().ptr;
    PlatformLayer layer{LayerVtable, 0};
    AddLayer(scene->state, layer);
    SceneIntegrationClient client{ClientVtable};
    SceneIntegrationClient replacement{ClientVtable};

    SceneIntegration* integration = _ZN7Nicosia16SceneIntegration6createERNS_5SceneERNS0_6ClientE(scene, &client).ptr;
    Require(integration->refCount == 1 && integration->lock == 0);
    Require(integration->scene == scene && integration->client == &client);
    Require(scene->refCount == 2);

    _ZN7Nicosia16SceneIntegration13requestUpdateEv(integration);
    Require(updateRequests == 1 && integration->lock == 0);
    _ZN7Nicosia16SceneIntegration9setClientERNS0_6ClientE(integration, &replacement);
    Require(integration->client == &replacement && integration->lock == 0);
    _ZN7Nicosia16SceneIntegration13requestUpdateEv(integration);
    Require(updateRequests == 2);

    --scene->refCount;
    _ZN7Nicosia16SceneIntegration10invalidateEv(integration);
    Require(!integration->scene && !integration->client && integration->lock == 0);
    Require(destroyedLayers == 1);
    _ZN7Nicosia16SceneIntegration13requestUpdateEv(integration);
    Require(updateRequests == 2);
    _ZN7Nicosia16SceneIntegrationD1Ev(integration);
    _ZN3WTF8fastFreeEPv(integration);

    Scene* other = _ZN7Nicosia5Scene6createEv().ptr;
    SceneIntegration local;
    _ZN7Nicosia16SceneIntegrationC2ERNS_5SceneERNS0_6ClientE(&local, other, &client);
    Require(other->refCount == 2);
    _ZN7Nicosia16SceneIntegrationD2Ev(&local);
    Require(other->refCount == 1);
    _ZN7Nicosia16SceneIntegrationC1ERNS_5SceneERNS0_6ClientE(&local, other, &client);
    --other->refCount;
    _ZN7Nicosia16SceneIntegrationD1Ev(&local);

    RequireInvalid([&] { _ZN7Nicosia16SceneIntegration6createERNS_5SceneERNS0_6ClientE(nullptr, &client); });
    RequireInvalid([&] { _ZN7Nicosia16SceneIntegration6createERNS_5SceneERNS0_6ClientE(other, nullptr); });
    RequireInvalid([&] { _ZN7Nicosia16SceneIntegration9setClientERNS0_6ClientE(&local, nullptr); });
    RequireInvalid([] { _ZN7Nicosia16SceneIntegration10invalidateEv(nullptr); });
    RequireInvalid([] { _ZN7Nicosia16SceneIntegration13requestUpdateEv(nullptr); });
}

}  // namespace

int main() {
    TestState();
    TestScene();
    TestSceneIntegration();
    return 0;
}
