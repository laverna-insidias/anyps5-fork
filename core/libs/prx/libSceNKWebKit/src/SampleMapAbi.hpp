// Derived from Source/WebCore/Modules/mediasource/SampleMap.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

// Derived from Source/WebCore/platform/MediaSample.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2013-2021 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#pragma once

#include "prx/libSceNKWebKit/src/MediaTypes.hpp"

#include <cstddef>
#include <cstdint>

namespace NKWebKit::SampleMapAbi {

using NKWebKit::MediaAbi::MediaTime;

struct MediaSample {
    const void* const* vtable;
    unsigned refCount;
};

enum MediaSampleSlot : std::size_t {
    MediaSampleDeletingDestructor = 1,
    MediaSamplePresentationTime = 2,
    MediaSampleDecodeTime = 3,
    MediaSampleDuration = 4,
    MediaSampleSizeInBytes = 6,
    MediaSampleFlags = 13,
};

inline constexpr int MediaSampleIsSync = 1;

struct TreeNodeBase {
    TreeNodeBase* left;
    TreeNodeBase* right;
    TreeNodeBase* parent;
    bool isBlack;
};

struct Tree {
    TreeNodeBase* beginNode;
    TreeNodeBase* root;
    std::size_t size;
};

struct DecodeKey {
    MediaTime decodeTime;
    MediaTime presentationTime;
};

struct PresentationNode {
    TreeNodeBase base;
    MediaTime key;
    MediaSample* sample;
};

struct DecodeNode {
    TreeNodeBase base;
    DecodeKey key;
    MediaSample* sample;
};

struct ReverseIterator {
    TreeNodeBase* unused;
    TreeNodeBase* current;
};

struct IteratorRange {
    TreeNodeBase* first;
    TreeNodeBase* second;
};

struct ReverseIteratorRange {
    ReverseIterator first;
    ReverseIterator second;
};

struct PresentationOrderSampleMap {
    Tree samples;
};

struct DecodeOrderSampleMap {
    Tree samples;
    PresentationOrderSampleMap presentationOrder;
};

struct SampleMap {
    DecodeOrderSampleMap decodeOrder;
    std::size_t totalSize;
};

static_assert(offsetof(MediaSample, refCount) == 8);
static_assert(offsetof(TreeNodeBase, isBlack) == 24);
static_assert(offsetof(Tree, root) == 8);
static_assert(sizeof(Tree) == 24);
static_assert(sizeof(DecodeKey) == 32);
static_assert(offsetof(PresentationNode, key) == 32);
static_assert(offsetof(PresentationNode, sample) == 48);
static_assert(offsetof(DecodeNode, key) == 32);
static_assert(offsetof(DecodeNode, sample) == 64);
static_assert(sizeof(ReverseIterator) == 16);
static_assert(sizeof(ReverseIteratorRange) == 32);
static_assert(offsetof(DecodeOrderSampleMap, presentationOrder) == 24);
static_assert(offsetof(SampleMap, totalSize) == 48);
static_assert(sizeof(SampleMap) == 56);

}  // namespace NKWebKit::SampleMapAbi
