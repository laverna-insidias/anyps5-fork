// Derived from Source/WebCore/loader/FrameLoaderStateMachine.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer. 
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution. 
 * 3.  Neither the name of Google, Inc. nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/FrameLoaderStateMachineExports.hpp"

#include <stdexcept>
#include <string>

namespace {

using NKWebKit::FrameLoaderAbi::FrameLoaderState;
using NKWebKit::FrameLoaderAbi::FrameLoaderStateMachine;

FrameLoaderState StateOf(const FrameLoaderStateMachine* self, const char* caller) {
    if (!self) throw std::invalid_argument(std::string("WebCore::FrameLoaderStateMachine::") + caller + ": null this");
    return self->state;
}

}  // namespace

extern "C" {

bool APS5_VABI _ZNK7WebCore23FrameLoaderStateMachine23committingFirstRealLoadEv(const FrameLoaderStateMachine* self) {
    return StateOf(self, "committingFirstRealLoad") == NKWebKit::FrameLoaderAbi::DisplayingInitialEmptyDocument;
}

bool APS5_VABI _ZNK7WebCore23FrameLoaderStateMachine28creatingInitialEmptyDocumentEv(const FrameLoaderStateMachine* self) {
    return StateOf(self, "creatingInitialEmptyDocument") == NKWebKit::FrameLoaderAbi::CreatingInitialEmptyDocument;
}

bool APS5_VABI _ZNK7WebCore23FrameLoaderStateMachine32isDisplayingInitialEmptyDocumentEv(const FrameLoaderStateMachine* self) {
    const FrameLoaderState state = StateOf(self, "isDisplayingInitialEmptyDocument");
    return state == NKWebKit::FrameLoaderAbi::DisplayingInitialEmptyDocument || state == NKWebKit::FrameLoaderAbi::DisplayingInitialEmptyDocumentPostCommit;
}

bool APS5_VABI _ZNK7WebCore23FrameLoaderStateMachine15firstLayoutDoneEv(const FrameLoaderStateMachine* self) {
    return StateOf(self, "firstLayoutDone") == NKWebKit::FrameLoaderAbi::FirstLayoutDone;
}

}
