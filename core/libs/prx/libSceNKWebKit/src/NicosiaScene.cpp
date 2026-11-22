// Derived from Source/WebCore/platform/graphics/nicosia/NicosiaScene.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

// Derived from Source/WebCore/platform/graphics/nicosia/NicosiaSceneIntegration.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2019 Metrological Group B.V.
 * Copyright (C) 2019 Igalia S.L.
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
#include "prx/libSceNKWebKit/include/NicosiaSceneExports.hpp"
#include "prx/libSceNKWebKit/include/WtfExports.hpp"
#include "prx/libSceNKWebKit/src/WtfIntHashTable.hpp"

#include <atomic>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

namespace HashTableAbi = NKWebKit::WtfAbi::HashTableAbi;
using NKWebKit::NicosiaAbi::PlatformLayer;
using NKWebKit::NicosiaAbi::Scene;
using NKWebKit::NicosiaAbi::SceneIntegration;
using NKWebKit::NicosiaAbi::SceneIntegrationClient;
using NKWebKit::NicosiaAbi::SceneIntegrationRef;
using NKWebKit::NicosiaAbi::SceneRef;
using NKWebKit::NicosiaAbi::SceneState;

[[noreturn]] void Invalid(const char* caller, const char* what) {
    throw std::invalid_argument(std::string("Nicosia::") + caller + ": " + what);
}

template<typename T>
T& Checked(T* value, const char* caller, const char* name) {
    if (!value) Invalid(caller, (std::string("null ") + name).c_str());
    return *value;
}

PlatformLayer* const DeletedLayer = reinterpret_cast<PlatformLayer*>(~std::uintptr_t(0));

void RefLayer(PlatformLayer* layer) {
    if (layer) std::atomic_ref<unsigned>(layer->refCount).fetch_add(1);
}

void DerefLayer(PlatformLayer* layer) {
    if (!layer || std::atomic_ref<unsigned>(layer->refCount).fetch_sub(1) != 1) return;
    using DeletingDestructor = void(APS5_VABI*)(PlatformLayer*);
    reinterpret_cast<DeletingDestructor>(layer->vtable[NKWebKit::NicosiaAbi::PlatformLayerDeletingDestructorSlot])(layer);
}

struct LayerPolicy {
    using Bucket = PlatformLayer*;
    using Key = PlatformLayer*;
    static constexpr bool EmptyValueIsZero = true;
    static Key KeyOf(Bucket& bucket) { return bucket; }
    static unsigned Hash(Key key) { return HashTableAbi::PtrHash(key); }
    static bool IsEmpty(Bucket& bucket) { return !bucket; }
    static bool IsDeleted(Bucket& bucket) { return bucket == DeletedLayer; }
    static void SetDeleted(Bucket& bucket) { bucket = DeletedLayer; }
    static void ConstructEmpty(Bucket& bucket) { bucket = nullptr; }
    static void DestroyValue(Bucket& bucket) { DerefLayer(std::exchange(bucket, nullptr)); }
    static void Move(Bucket& from, Bucket& to) { to = std::exchange(from, nullptr); }
};

PlatformLayer** CopyLayers(PlatformLayer** other) {
    return HashTableAbi::CopyTable<LayerPolicy>(other, [](PlatformLayer*& from, PlatformLayer*& to) {
        RefLayer(from);
        to = from;
    });
}

void AssignLayer(PlatformLayer*& target, PlatformLayer* value) {
    RefLayer(value);
    DerefLayer(std::exchange(target, value));
}

void ConstructState(SceneState& state) {
    state.id = 0;
    state.platformLayerUpdated = false;
    state.layers.table = nullptr;
    state.rootLayer = nullptr;
}

void CopyState(SceneState& state, const SceneState& other) {
    state.id = other.id;
    state.platformLayerUpdated = other.platformLayerUpdated;
    state.layers.table = CopyLayers(other.layers.table);
    state.rootLayer = other.rootLayer;
    RefLayer(state.rootLayer);
}

void DestroyState(SceneState& state) {
    DerefLayer(std::exchange(state.rootLayer, nullptr));
    HashTableAbi::DeallocateTable<LayerPolicy>(std::exchange(state.layers.table, nullptr));
}

void DerefScene(Scene* scene) {
    if (!scene || std::atomic_ref<unsigned>(scene->refCount).fetch_sub(1) != 1) return;
    _ZN7Nicosia5SceneD1Ev(scene);
    _ZN3WTF8fastFreeEPv(scene);
}

void ConstructScene(Scene* self) {
    auto& scene = Checked(self, "Scene::Scene", "this");
    scene.refCount = 1;
    scene.lock = 0;
    ConstructState(scene.state);
}

void ConstructIntegration(SceneIntegration* self, Scene* scene, SceneIntegrationClient* client) {
    auto& integration = Checked(self, "SceneIntegration::SceneIntegration", "this");
    auto& sceneObject = Checked(scene, "SceneIntegration::SceneIntegration", "scene");
    auto& clientObject = Checked(client, "SceneIntegration::SceneIntegration", "client");
    integration.refCount = 1;
    integration.lock = 0;
    std::atomic_ref<unsigned>(sceneObject.refCount).fetch_add(1);
    integration.scene = &sceneObject;
    integration.client = &clientObject;
}

void DestroyIntegration(SceneIntegration* self) {
    auto& integration = Checked(self, "SceneIntegration::~SceneIntegration", "this");
    DerefScene(std::exchange(integration.scene, nullptr));
}

}  // namespace

extern "C" {

SceneRef APS5_VABI _ZN7Nicosia5Scene6createEv() {
    auto* scene = static_cast<Scene*>(_ZN3WTF10fastMallocEm(sizeof(Scene)));
    ConstructScene(scene);
    return SceneRef(scene);
}

void APS5_VABI _ZN7Nicosia5SceneC1Ev(Scene* self) {
    ConstructScene(self);
}

void APS5_VABI _ZN7Nicosia5SceneC2Ev(Scene* self) {
    ConstructScene(self);
}

void APS5_VABI _ZN7Nicosia5SceneD1Ev(Scene* self) {
    DestroyState(Checked(self, "Scene::~Scene", "this").state);
}

void APS5_VABI _ZN7Nicosia5SceneD2Ev(Scene* self) {
    _ZN7Nicosia5SceneD1Ev(self);
}

void APS5_VABI _ZN7Nicosia5Scene5StateC1Ev(SceneState* self) {
    ConstructState(Checked(self, "Scene::State::State", "this"));
}

void APS5_VABI _ZN7Nicosia5Scene5StateC2Ev(SceneState* self) {
    _ZN7Nicosia5Scene5StateC1Ev(self);
}

void APS5_VABI _ZN7Nicosia5Scene5StateC1ERKS1_(SceneState* self, const SceneState* other) {
    auto& state = Checked(self, "Scene::State::State", "this");
    CopyState(state, Checked(other, "Scene::State::State", "other"));
}

void APS5_VABI _ZN7Nicosia5Scene5StateC2ERKS1_(SceneState* self, const SceneState* other) {
    _ZN7Nicosia5Scene5StateC1ERKS1_(self, other);
}

void APS5_VABI _ZN7Nicosia5Scene5StateD1Ev(SceneState* self) {
    DestroyState(Checked(self, "Scene::State::~State", "this"));
}

void APS5_VABI _ZN7Nicosia5Scene5StateD2Ev(SceneState* self) {
    _ZN7Nicosia5Scene5StateD1Ev(self);
}

SceneState* APS5_VABI _ZN7Nicosia5Scene5StateaSERKS1_(SceneState* self, const SceneState* other) {
    auto& state = Checked(self, "Scene::State::operator=", "this");
    const auto& source = Checked(other, "Scene::State::operator=", "other");
    state.id = source.id;
    state.platformLayerUpdated = source.platformLayerUpdated;
    PlatformLayer** layers = CopyLayers(source.layers.table);
    HashTableAbi::DeallocateTable<LayerPolicy>(std::exchange(state.layers.table, layers));
    AssignLayer(state.rootLayer, source.rootLayer);
    return self;
}

SceneIntegrationRef APS5_VABI _ZN7Nicosia16SceneIntegration6createERNS_5SceneERNS0_6ClientE(Scene* scene, SceneIntegrationClient* client) {
    Checked(scene, "SceneIntegration::create", "scene");
    Checked(client, "SceneIntegration::create", "client");
    auto* integration = static_cast<SceneIntegration*>(_ZN3WTF10fastMallocEm(sizeof(SceneIntegration)));
    ConstructIntegration(integration, scene, client);
    return SceneIntegrationRef(integration);
}

void APS5_VABI _ZN7Nicosia16SceneIntegrationC1ERNS_5SceneERNS0_6ClientE(SceneIntegration* self, Scene* scene, SceneIntegrationClient* client) {
    ConstructIntegration(self, scene, client);
}

void APS5_VABI _ZN7Nicosia16SceneIntegrationC2ERNS_5SceneERNS0_6ClientE(SceneIntegration* self, Scene* scene, SceneIntegrationClient* client) {
    ConstructIntegration(self, scene, client);
}

void APS5_VABI _ZN7Nicosia16SceneIntegrationD1Ev(SceneIntegration* self) {
    DestroyIntegration(self);
}

void APS5_VABI _ZN7Nicosia16SceneIntegrationD2Ev(SceneIntegration* self) {
    DestroyIntegration(self);
}

void APS5_VABI _ZN7Nicosia16SceneIntegration9setClientERNS0_6ClientE(SceneIntegration* self, SceneIntegrationClient* client) {
    auto& integration = Checked(self, "SceneIntegration::setClient", "this");
    auto& clientObject = Checked(client, "SceneIntegration::setClient", "client");
    HashTableAbi::LockHolder locker(integration.lock);
    integration.client = &clientObject;
}

void APS5_VABI _ZN7Nicosia16SceneIntegration10invalidateEv(SceneIntegration* self) {
    auto& integration = Checked(self, "SceneIntegration::invalidate", "this");
    HashTableAbi::LockHolder locker(integration.lock);
    DerefScene(std::exchange(integration.scene, nullptr));
    integration.client = nullptr;
}

void APS5_VABI _ZN7Nicosia16SceneIntegration13requestUpdateEv(SceneIntegration* self) {
    auto& integration = Checked(self, "SceneIntegration::requestUpdate", "this");
    HashTableAbi::LockHolder locker(integration.lock);
    if (!integration.client) return;
    using RequestUpdate = void(APS5_VABI*)(SceneIntegrationClient*);
    reinterpret_cast<RequestUpdate>(integration.client->vtable[NKWebKit::NicosiaAbi::ClientRequestUpdateSlot])(integration.client);
}

}
