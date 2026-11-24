#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <utility>

using NKWebKit::OriginLockAbi::OriginLock;
using NKWebKit::WtfAbi::String;

namespace {

void Check(bool condition, int line) {
    if (!condition) {
        std::fprintf(stderr, "OriginLock check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(cond) Check((cond), __LINE__)

void TestReleasesLockFileName() {
    String name;
    _ZN3WTF6StringC1EPKc(&name, "/data/origin/.lock");
    Require(name.impl != nullptr);
    const unsigned before = name.impl->refCount;
    for (auto destroy : {&_ZN7WebCore10OriginLockD1Ev, &_ZN7WebCore10OriginLockD2Ev}) {
        name.impl->refCount += 2;
        OriginLock lock{2, name.impl, 0};
        destroy(&lock);
        Require(lock.lockFileName == nullptr);
        Require(name.impl->refCount == before);
    }
}

void TestLastReferenceAndNullName() {
    String name;
    _ZN3WTF6StringC1EPKc(&name, "/data/origin/.lock");
    OriginLock owner{2, std::exchange(name.impl, nullptr), 0};
    _ZN7WebCore10OriginLockD2Ev(&owner);
    Require(owner.lockFileName == nullptr);

    OriginLock empty{2, nullptr, 0};
    _ZN7WebCore10OriginLockD1Ev(&empty);
    Require(empty.lockFileName == nullptr);
}

void TestNullThis() {
    bool threw = false;
    try {
        _ZN7WebCore10OriginLockD1Ev(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
}

}  // namespace

int main() {
    TestReleasesLockFileName();
    TestLastReferenceAndNullName();
    TestNullThis();
    std::puts("OriginLock tests passed");
    return 0;
}
