#pragma once

#include "prx/libc/include/general/VabiMacros.hpp"

namespace NKWebKit::FrameLoaderAbi {

enum FrameLoaderState : int {
    CreatingInitialEmptyDocument,
    DisplayingInitialEmptyDocument,
    DisplayingInitialEmptyDocumentPostCommit,
    CommittedFirstRealLoad,
    FirstLayoutDone,
};

struct FrameLoaderStateMachine {
    FrameLoaderState state;
};

static_assert(sizeof(FrameLoaderStateMachine) == 4);

}  // namespace NKWebKit::FrameLoaderAbi

extern "C" {

bool APS5_VABI _ZNK7WebCore23FrameLoaderStateMachine23committingFirstRealLoadEv(const NKWebKit::FrameLoaderAbi::FrameLoaderStateMachine* self);
bool APS5_VABI _ZNK7WebCore23FrameLoaderStateMachine28creatingInitialEmptyDocumentEv(const NKWebKit::FrameLoaderAbi::FrameLoaderStateMachine* self);
bool APS5_VABI _ZNK7WebCore23FrameLoaderStateMachine32isDisplayingInitialEmptyDocumentEv(const NKWebKit::FrameLoaderAbi::FrameLoaderStateMachine* self);
bool APS5_VABI _ZNK7WebCore23FrameLoaderStateMachine15firstLayoutDoneEv(const NKWebKit::FrameLoaderAbi::FrameLoaderStateMachine* self);

}
