#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdio>
#include <cstdlib>
#include <stdexcept>

using NKWebKit::FrameLoaderAbi::FrameLoaderState;
using NKWebKit::FrameLoaderAbi::FrameLoaderStateMachine;

namespace {

void Check(bool condition, int line) {
    if (!condition) {
        std::fprintf(stderr, "FrameLoaderStateMachine check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(cond) Check((cond), __LINE__)

struct Expected {
    FrameLoaderState state;
    bool committingFirstRealLoad;
    bool creatingInitialEmptyDocument;
    bool isDisplayingInitialEmptyDocument;
    bool firstLayoutDone;
};

void TestEveryState() {
    const Expected table[] = {
        {NKWebKit::FrameLoaderAbi::CreatingInitialEmptyDocument, false, true, false, false},
        {NKWebKit::FrameLoaderAbi::DisplayingInitialEmptyDocument, true, false, true, false},
        {NKWebKit::FrameLoaderAbi::DisplayingInitialEmptyDocumentPostCommit, false, false, true, false},
        {NKWebKit::FrameLoaderAbi::CommittedFirstRealLoad, false, false, false, false},
        {NKWebKit::FrameLoaderAbi::FirstLayoutDone, false, false, false, true},
    };
    for (const Expected& row : table) {
        const FrameLoaderStateMachine machine{row.state};
        Require(_ZNK7WebCore23FrameLoaderStateMachine23committingFirstRealLoadEv(&machine) == row.committingFirstRealLoad);
        Require(_ZNK7WebCore23FrameLoaderStateMachine28creatingInitialEmptyDocumentEv(&machine) == row.creatingInitialEmptyDocument);
        Require(_ZNK7WebCore23FrameLoaderStateMachine32isDisplayingInitialEmptyDocumentEv(&machine) == row.isDisplayingInitialEmptyDocument);
        Require(_ZNK7WebCore23FrameLoaderStateMachine15firstLayoutDoneEv(&machine) == row.firstLayoutDone);
    }
}

template<typename Function>
bool Throws(Function function) {
    try {
        function();
    } catch (const std::invalid_argument&) {
        return true;
    }
    return false;
}

void TestNullThis() {
    Require(Throws([] { _ZNK7WebCore23FrameLoaderStateMachine23committingFirstRealLoadEv(nullptr); }));
    Require(Throws([] { _ZNK7WebCore23FrameLoaderStateMachine28creatingInitialEmptyDocumentEv(nullptr); }));
    Require(Throws([] { _ZNK7WebCore23FrameLoaderStateMachine32isDisplayingInitialEmptyDocumentEv(nullptr); }));
    Require(Throws([] { _ZNK7WebCore23FrameLoaderStateMachine15firstLayoutDoneEv(nullptr); }));
}

}  // namespace

int main() {
    TestEveryState();
    TestNullThis();
    std::puts("FrameLoaderStateMachine tests passed");
    return 0;
}
