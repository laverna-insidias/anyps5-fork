#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <thread>
#include <vector>

using NKWebKit::NicosiaContentLayerAbi::ContentLayerClient;
using NKWebKit::NicosiaContentLayerAbi::ContentLayerTextureMapperImpl;

namespace {

void Check(bool condition, int line) {
    if (!condition) {
        std::fprintf(stderr, "NicosiaContentLayer check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(cond) Check((cond), __LINE__)

struct FakeClient {
    ContentLayerClient base;
    ContentLayerTextureMapperImpl* impl;
    std::atomic<int> swaps;
    bool lockHeld;
};

void APS5_VABI FakeSwapBuffers(ContentLayerClient* client) {
    auto* fake = reinterpret_cast<FakeClient*>(client);
    fake->lockHeld = fake->impl->lock != 0;
    fake->swaps.fetch_add(1);
}

const void* const FakeClientVtable[] = {
    nullptr,
    nullptr,
    reinterpret_cast<const void*>(&FakeSwapBuffers),
};

ContentLayerTextureMapperImpl MakeImpl(ContentLayerClient* client) {
    ContentLayerTextureMapperImpl impl{};
    impl.client = client;
    impl.pendingUpdate = true;
    return impl;
}

void TestFlushUpdate() {
    ContentLayerTextureMapperImpl impl = MakeImpl(nullptr);
    Require(_ZN7Nicosia29ContentLayerTextureMapperImpl11flushUpdateEv(&impl));
    Require(!impl.pendingUpdate);
    Require(!_ZN7Nicosia29ContentLayerTextureMapperImpl11flushUpdateEv(&impl));
    Require(impl.lock == 0);
    impl.pendingUpdate = true;
    Require(_ZN7Nicosia29ContentLayerTextureMapperImpl11flushUpdateEv(&impl));
}

void TestSwapBuffers() {
    FakeClient client{};
    client.base.vtable = FakeClientVtable;
    ContentLayerTextureMapperImpl impl = MakeImpl(&client.base);
    client.impl = &impl;
    _ZN7Nicosia29ContentLayerTextureMapperImpl19swapBuffersIfNeededEv(&impl);
    Require(client.swaps.load() == 1);
    Require(client.lockHeld);
    Require(impl.lock == 0);
    Require(impl.pendingUpdate);

    impl.client = nullptr;
    _ZN7Nicosia29ContentLayerTextureMapperImpl19swapBuffersIfNeededEv(&impl);
    Require(client.swaps.load() == 1);
}

void TestConcurrentFlush() {
    ContentLayerTextureMapperImpl impl = MakeImpl(nullptr);
    std::atomic<int> granted{0};
    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back([&] {
            for (int j = 0; j < 1000; ++j)
                if (_ZN7Nicosia29ContentLayerTextureMapperImpl11flushUpdateEv(&impl)) granted.fetch_add(1);
        });
    }
    for (auto& thread : threads) thread.join();
    Require(granted.load() == 1);
    Require(impl.lock == 0);
}

void TestNullThis() {
    bool threw = false;
    try {
        _ZN7Nicosia29ContentLayerTextureMapperImpl11flushUpdateEv(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
    threw = false;
    try {
        _ZN7Nicosia29ContentLayerTextureMapperImpl19swapBuffersIfNeededEv(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
}

}  // namespace

int main() {
    TestFlushUpdate();
    TestSwapBuffers();
    TestConcurrentFlush();
    TestNullThis();
    std::puts("NicosiaContentLayer tests passed");
    return 0;
}
