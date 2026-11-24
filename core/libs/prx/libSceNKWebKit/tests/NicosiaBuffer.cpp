#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <thread>
#include <utility>

using NKWebKit::NicosiaBufferAbi::Buffer;
using NKWebKit::NicosiaBufferAbi::BufferRef;
using NKWebKit::NicosiaBufferAbi::PaintingState;

namespace {

void Check(bool condition, int line) {
    if (!condition) {
        std::fprintf(stderr, "NicosiaBuffer check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(cond) Check((cond), __LINE__)

template<typename Exception, typename Function>
bool Throws(Function&& function) {
    try {
        function();
    } catch (const Exception&) {
        return true;
    }
    return false;
}

void TestCreateZeroesPixels() {
    const WebCore::IntSize size{3, 2};
    BufferRef buffer = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&size, 1);
    Require(buffer.ptr != nullptr);
    Require(buffer.ptr->refCount == 1);
    Require(buffer.ptr->size.m_width == 3 && buffer.ptr->size.m_height == 2);
    Require(buffer.ptr->flags == 1);
    Require(buffer.ptr->paintingLock == 0 && buffer.ptr->paintingCondition == 0);
    Require(buffer.ptr->paintingState == PaintingState::Complete);
    Require(buffer.ptr->data != nullptr);
    for (int i = 0; i < 3 * 2 * 4; ++i) Require(buffer.ptr->data[i] == 0);
}

void TestDestructorsReleasePixels() {
    for (auto destroy : {&_ZN7Nicosia6BufferD1Ev, &_ZN7Nicosia6BufferD2Ev}) {
        const WebCore::IntSize size{4, 4};
        BufferRef buffer = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&size, 0);
        destroy(buffer.ptr);
        Require(buffer.ptr->data == nullptr);
    }
}

void TestEmptyAndInvalidSizes() {
    const WebCore::IntSize empty{0, 5};
    BufferRef buffer = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&empty, 0);
    Require(buffer.ptr->size.m_width == 0 && buffer.ptr->size.m_height == 5);

    const WebCore::IntSize negative{-1, 5};
    Require(Throws<std::overflow_error>([&] { BufferRef r = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&negative, 0); }));
    const WebCore::IntSize huge{65536, 16384};
    Require(Throws<std::overflow_error>([&] { BufferRef r = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&huge, 0); }));
    Require(Throws<std::invalid_argument>([] { BufferRef r = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(nullptr, 0); }));
}

void TestWaitUntilPaintingComplete() {
    const WebCore::IntSize size{1, 1};
    BufferRef buffer = _ZN7Nicosia6Buffer6createERKN7WebCore7IntSizeEj(&size, 0);
    _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(buffer.ptr);

    std::atomic_ref<PaintingState>(buffer.ptr->paintingState).store(PaintingState::InProgress);
    std::atomic<bool> returned{false};
    std::thread waiter([&] {
        _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(buffer.ptr);
        returned = true;
    });
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    Require(!returned);
    std::atomic_ref<PaintingState>(buffer.ptr->paintingState).store(PaintingState::Complete);
    waiter.join();
    Require(returned);
    Require(buffer.ptr->paintingLock == 0);
}

void TestNullThis() {
    Require(Throws<std::invalid_argument>([] { _ZN7Nicosia6BufferD1Ev(nullptr); }));
    Require(Throws<std::invalid_argument>([] { _ZN7Nicosia6Buffer25waitUntilPaintingCompleteEv(nullptr); }));
}

}  // namespace

int main() {
    TestCreateZeroesPixels();
    TestDestructorsReleasePixels();
    TestEmptyAndInvalidSizes();
    TestWaitUntilPaintingComplete();
    TestNullThis();
    std::puts("NicosiaBuffer tests passed");
    return 0;
}
