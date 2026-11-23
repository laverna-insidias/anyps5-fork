// Derived from Source/WebCore/Modules/mediasource/SampleMap.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
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

#include "prx/libc/include/General.hpp"
#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/include/MediaExports.hpp"
#include "prx/libSceNKWebKit/include/SampleMapExports.hpp"
#include "prx/libSceNKWebKit/include/WtfExports.hpp"

#include <algorithm>
#include <atomic>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>

namespace {

using NKWebKit::SampleMapAbi::DecodeKey;
using NKWebKit::SampleMapAbi::DecodeNode;
using NKWebKit::SampleMapAbi::DecodeOrderSampleMap;
using NKWebKit::SampleMapAbi::IteratorRange;
using NKWebKit::SampleMapAbi::MediaSample;
using NKWebKit::SampleMapAbi::PresentationNode;
using NKWebKit::SampleMapAbi::PresentationOrderSampleMap;
using NKWebKit::SampleMapAbi::ReverseIterator;
using NKWebKit::SampleMapAbi::ReverseIteratorRange;
using NKWebKit::SampleMapAbi::SampleMap;
using NKWebKit::SampleMapAbi::Tree;
using NKWebKit::SampleMapAbi::TreeNodeBase;
using WTF::MediaTime;

template<typename T>
T& Checked(T* value, const char* caller, const char* name) {
    if (!value) throw std::invalid_argument(std::string("WebCore::") + caller + ": null " + name);
    return *value;
}

bool Less(const MediaTime& a, const MediaTime& b) {
    return _ZNK3WTF9MediaTimeltERKS0_(&a, b);
}

bool Less(const DecodeKey& a, const DecodeKey& b) {
    if (Less(a.decodeTime, b.decodeTime)) return true;
    if (Less(b.decodeTime, a.decodeTime)) return false;
    return Less(a.presentationTime, b.presentationTime);
}

MediaTime Plus(const MediaTime& a, const MediaTime& b) {
    return _ZNK3WTF9MediaTimeplERKS0_(&a, b);
}

MediaTime Minus(const MediaTime& a, const MediaTime& b) {
    return _ZNK3WTF9MediaTimemiERKS0_(&a, b);
}

template<typename Result>
Result CallSample(const MediaSample* sample, std::size_t slot) {
    using Function = Result(APS5_VABI*)(const MediaSample*);
    return reinterpret_cast<Function>(sample->vtable[slot])(sample);
}

MediaTime PresentationTime(const MediaSample* sample) {
    return CallSample<MediaTime>(sample, NKWebKit::SampleMapAbi::MediaSamplePresentationTime);
}

MediaTime DecodeTime(const MediaSample* sample) {
    return CallSample<MediaTime>(sample, NKWebKit::SampleMapAbi::MediaSampleDecodeTime);
}

MediaTime PresentationEndTime(const MediaSample* sample) {
    return Plus(PresentationTime(sample), CallSample<MediaTime>(sample, NKWebKit::SampleMapAbi::MediaSampleDuration));
}

std::size_t SizeInBytes(const MediaSample* sample) {
    return CallSample<std::size_t>(sample, NKWebKit::SampleMapAbi::MediaSampleSizeInBytes);
}

bool IsSync(const MediaSample* sample) {
    return CallSample<int>(sample, NKWebKit::SampleMapAbi::MediaSampleFlags) & NKWebKit::SampleMapAbi::MediaSampleIsSync;
}

DecodeKey KeyOf(const MediaSample* sample) {
    return DecodeKey{DecodeTime(sample), PresentationTime(sample)};
}

void RefSample(MediaSample* sample) {
    std::atomic_ref<unsigned>(sample->refCount).fetch_add(1);
}

void DerefSample(MediaSample* sample) {
    if (std::atomic_ref<unsigned>(sample->refCount).fetch_sub(1) != 1) return;
    using DeletingDestructor = void(APS5_VABI*)(MediaSample*);
    reinterpret_cast<DeletingDestructor>(sample->vtable[NKWebKit::SampleMapAbi::MediaSampleDeletingDestructor])(sample);
}

TreeNodeBase* EndNode(Tree& tree) {
    return reinterpret_cast<TreeNodeBase*>(&tree.root);
}

bool IsLeftChild(TreeNodeBase* node) {
    return node == node->parent->left;
}

TreeNodeBase* Minimum(TreeNodeBase* node) {
    while (node->left) node = node->left;
    return node;
}

TreeNodeBase* Maximum(TreeNodeBase* node) {
    while (node->right) node = node->right;
    return node;
}

TreeNodeBase* Next(TreeNodeBase* node) {
    if (node->right) return Minimum(node->right);
    while (!IsLeftChild(node)) node = node->parent;
    return node->parent;
}

TreeNodeBase* Previous(TreeNodeBase* node) {
    if (node->left) return Maximum(node->left);
    while (IsLeftChild(node)) node = node->parent;
    return node->parent;
}

void ReplaceChild(TreeNodeBase* node, TreeNodeBase* replacement) {
    if (IsLeftChild(node))
        node->parent->left = replacement;
    else
        node->parent->right = replacement;
}

void RotateLeft(TreeNodeBase* node) {
    TreeNodeBase* pivot = node->right;
    node->right = pivot->left;
    if (pivot->left) pivot->left->parent = node;
    pivot->parent = node->parent;
    ReplaceChild(node, pivot);
    pivot->left = node;
    node->parent = pivot;
}

void RotateRight(TreeNodeBase* node) {
    TreeNodeBase* pivot = node->left;
    node->left = pivot->right;
    if (pivot->right) pivot->right->parent = node;
    pivot->parent = node->parent;
    ReplaceChild(node, pivot);
    pivot->right = node;
    node->parent = pivot;
}

bool IsBlack(TreeNodeBase* node) {
    return !node || node->isBlack;
}

void BalanceAfterInsert(Tree& tree, TreeNodeBase* node) {
    node->isBlack = node == tree.root;
    while (node != tree.root && !node->parent->isBlack) {
        TreeNodeBase* parent = node->parent;
        TreeNodeBase* grandparent = parent->parent;
        const bool parentIsLeft = parent == grandparent->left;
        TreeNodeBase* uncle = parentIsLeft ? grandparent->right : grandparent->left;
        if (!IsBlack(uncle)) {
            parent->isBlack = true;
            uncle->isBlack = true;
            grandparent->isBlack = grandparent == tree.root;
            node = grandparent;
            continue;
        }
        if (parentIsLeft && node == parent->right) {
            RotateLeft(parent);
            parent = node;
        } else if (!parentIsLeft && node == parent->left) {
            RotateRight(parent);
            parent = node;
        }
        parent->isBlack = true;
        grandparent->isBlack = false;
        if (parentIsLeft)
            RotateRight(grandparent);
        else
            RotateLeft(grandparent);
        break;
    }
}

void BalanceAfterErase(Tree& tree, TreeNodeBase* node, TreeNodeBase* parent) {
    while (node != tree.root && IsBlack(node)) {
        if (node == parent->left) {
            TreeNodeBase* sibling = parent->right;
            if (!sibling->isBlack) {
                sibling->isBlack = true;
                parent->isBlack = false;
                RotateLeft(parent);
                sibling = parent->right;
            }
            if (IsBlack(sibling->left) && IsBlack(sibling->right)) {
                sibling->isBlack = false;
                node = parent;
                parent = node->parent;
                continue;
            }
            if (IsBlack(sibling->right)) {
                sibling->left->isBlack = true;
                sibling->isBlack = false;
                RotateRight(sibling);
                sibling = parent->right;
            }
            sibling->isBlack = parent->isBlack;
            parent->isBlack = true;
            sibling->right->isBlack = true;
            RotateLeft(parent);
        } else {
            TreeNodeBase* sibling = parent->left;
            if (!sibling->isBlack) {
                sibling->isBlack = true;
                parent->isBlack = false;
                RotateRight(parent);
                sibling = parent->left;
            }
            if (IsBlack(sibling->left) && IsBlack(sibling->right)) {
                sibling->isBlack = false;
                node = parent;
                parent = node->parent;
                continue;
            }
            if (IsBlack(sibling->left)) {
                sibling->right->isBlack = true;
                sibling->isBlack = false;
                RotateLeft(sibling);
                sibling = parent->left;
            }
            sibling->isBlack = parent->isBlack;
            parent->isBlack = true;
            sibling->left->isBlack = true;
            RotateRight(parent);
        }
        node = tree.root;
        break;
    }
    if (node) node->isBlack = true;
}

void Unlink(Tree& tree, TreeNodeBase* node) {
    if (tree.beginNode == node) tree.beginNode = Next(node);
    TreeNodeBase* removed = node->left && node->right ? Minimum(node->right) : node;
    TreeNodeBase* child = removed->left ? removed->left : removed->right;
    TreeNodeBase* childParent = removed->parent;
    if (child) child->parent = childParent;
    ReplaceChild(removed, child);
    const bool removedBlack = removed->isBlack;
    if (removed != node) {
        if (childParent == node) childParent = removed;
        removed->parent = node->parent;
        ReplaceChild(node, removed);
        removed->left = node->left;
        if (removed->left) removed->left->parent = removed;
        removed->right = node->right;
        if (removed->right) removed->right->parent = removed;
        removed->isBlack = node->isBlack;
    }
    --tree.size;
    if (removedBlack && tree.root) BalanceAfterErase(tree, child, childParent);
}

template<typename Node>
const auto& NodeKey(TreeNodeBase* node) {
    return reinterpret_cast<Node*>(node)->key;
}

template<typename Node>
MediaSample* NodeSample(TreeNodeBase* node) {
    return reinterpret_cast<Node*>(node)->sample;
}

template<typename Node, typename Key>
TreeNodeBase* LowerBound(Tree& tree, const Key& key) {
    TreeNodeBase* result = EndNode(tree);
    for (TreeNodeBase* node = tree.root; node;) {
        if (!Less(NodeKey<Node>(node), key)) {
            result = node;
            node = node->left;
        } else
            node = node->right;
    }
    return result;
}

template<typename Node, typename Key>
TreeNodeBase* UpperBound(Tree& tree, const Key& key) {
    TreeNodeBase* result = EndNode(tree);
    for (TreeNodeBase* node = tree.root; node;) {
        if (Less(key, NodeKey<Node>(node))) {
            result = node;
            node = node->left;
        } else
            node = node->right;
    }
    return result;
}

template<typename Node, typename Key>
TreeNodeBase* Find(Tree& tree, const Key& key) {
    TreeNodeBase* node = LowerBound<Node>(tree, key);
    if (node == EndNode(tree) || Less(key, NodeKey<Node>(node))) return EndNode(tree);
    return node;
}

template<typename Node, typename Key>
void Insert(Tree& tree, const Key& key, MediaSample* sample) {
    TreeNodeBase* parent = EndNode(tree);
    TreeNodeBase** link = &tree.root;
    while (*link) {
        parent = *link;
        if (Less(key, NodeKey<Node>(parent)))
            link = &parent->left;
        else if (Less(NodeKey<Node>(parent), key))
            link = &parent->right;
        else
            return;
    }
    auto* node = static_cast<Node*>(_ZN3WTF10fastMallocEm(sizeof(Node)));
    node->base = TreeNodeBase{nullptr, nullptr, parent, false};
    node->key = key;
    node->sample = sample;
    RefSample(sample);
    *link = &node->base;
    if (tree.beginNode->left) tree.beginNode = tree.beginNode->left;
    BalanceAfterInsert(tree, &node->base);
    ++tree.size;
}

template<typename Node>
void DestroyNode(TreeNodeBase* node) {
    DerefSample(NodeSample<Node>(node));
    _ZN3WTF8fastFreeEPv(node);
}

template<typename Node, typename Key>
void Erase(Tree& tree, const Key& key) {
    TreeNodeBase* node = Find<Node>(tree, key);
    if (node == EndNode(tree)) return;
    Unlink(tree, node);
    DestroyNode<Node>(node);
}

template<typename Node>
void DestroySubtree(TreeNodeBase* node) {
    while (node) {
        DestroySubtree<Node>(node->right);
        TreeNodeBase* left = node->left;
        DestroyNode<Node>(node);
        node = left;
    }
}

template<typename Node>
void Clear(Tree& tree) {
    DestroySubtree<Node>(std::exchange(tree.root, nullptr));
    tree.beginNode = EndNode(tree);
    tree.size = 0;
}

class ReverseCursor {
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = TreeNodeBase*;
    using difference_type = std::ptrdiff_t;
    using pointer = TreeNodeBase**;
    using reference = TreeNodeBase*;

    explicit ReverseCursor(TreeNodeBase* base) : current(base) {}
    TreeNodeBase* operator*() const { return Previous(current); }
    ReverseCursor& operator++() {
        current = Previous(current);
        return *this;
    }
    ReverseCursor operator++(int) {
        ReverseCursor copy = *this;
        ++*this;
        return copy;
    }
    ReverseCursor& operator--() {
        current = Next(current);
        return *this;
    }
    ReverseCursor operator--(int) {
        ReverseCursor copy = *this;
        --*this;
        return copy;
    }
    bool operator==(const ReverseCursor& other) const { return current == other.current; }
    bool operator!=(const ReverseCursor& other) const { return current != other.current; }

    TreeNodeBase* current;
};

ReverseIterator MakeReverse(TreeNodeBase* base) {
    return ReverseIterator{base, base};
}

ReverseIterator MakeReverse(ReverseCursor cursor) {
    return MakeReverse(cursor.current);
}

ReverseCursor RBegin(Tree& tree) {
    return ReverseCursor(EndNode(tree));
}

ReverseCursor REnd(Tree& tree) {
    return ReverseCursor(tree.beginNode);
}

struct IsSyncSample {
    bool operator()(TreeNodeBase* node) const { return IsSync(NodeSample<DecodeNode>(node)); }
};

ReverseCursor FindSyncSamplePrior(Tree& tree, ReverseCursor from) {
    return std::find_if(from, REnd(tree), IsSyncSample());
}

TreeNodeBase* FindSampleStartingOnOrAfter(PresentationOrderSampleMap& map, const MediaTime& time) {
    return LowerBound<PresentationNode>(map.samples, time);
}

ReverseCursor ReverseFindSampleBefore(PresentationOrderSampleMap& map, const MediaTime& time) {
    Tree& tree = map.samples;
    if (!tree.size) return REnd(tree);
    TreeNodeBase* found = UpperBound<PresentationNode>(tree, time);
    if (found == EndNode(tree)) return RBegin(tree);
    if (found == tree.beginNode) return REnd(tree);
    return ReverseCursor(found);
}

TreeNodeBase* FindWithDecodeKey(DecodeOrderSampleMap& map, const DecodeKey& key) {
    return Find<DecodeNode>(map.samples, key);
}

ReverseCursor ReverseFindWithDecodeKey(DecodeOrderSampleMap& map, const DecodeKey& key) {
    TreeNodeBase* found = FindWithDecodeKey(map, key);
    if (found == EndNode(map.samples)) return REnd(map.samples);
    return ReverseCursor(Next(found));
}

}  // namespace

extern "C" {

bool APS5_VABI _ZNK7WebCore9SampleMap5emptyEv(const SampleMap* self) {
    return !Checked(self, "SampleMap::empty", "this").decodeOrder.presentationOrder.samples.size;
}

void APS5_VABI _ZN7WebCore9SampleMap5clearEv(SampleMap* self) {
    auto& map = Checked(self, "SampleMap::clear", "this");
    Clear<PresentationNode>(map.decodeOrder.presentationOrder.samples);
    Clear<DecodeNode>(map.decodeOrder.samples);
    map.totalSize = 0;
}

void APS5_VABI _ZN7WebCore9SampleMap9addSampleERNS_11MediaSampleE(SampleMap* self, MediaSample& sample) {
    auto& map = Checked(self, "SampleMap::addSample", "this");
    const MediaTime presentationTime = PresentationTime(&sample);
    Insert<PresentationNode>(map.decodeOrder.presentationOrder.samples, presentationTime, &sample);
    Insert<DecodeNode>(map.decodeOrder.samples, DecodeKey{DecodeTime(&sample), presentationTime}, &sample);
    map.totalSize += SizeInBytes(&sample);
}

void APS5_VABI _ZN7WebCore9SampleMap12removeSampleEPNS_11MediaSampleE(SampleMap* self, MediaSample* sample) {
    auto& map = Checked(self, "SampleMap::removeSample", "this");
    Checked(sample, "SampleMap::removeSample", "sample");
    const MediaTime presentationTime = PresentationTime(sample);
    map.totalSize -= SizeInBytes(sample);
    const DecodeKey decodeKey{DecodeTime(sample), presentationTime};
    Erase<PresentationNode>(map.decodeOrder.presentationOrder.samples, presentationTime);
    Erase<DecodeNode>(map.decodeOrder.samples, decodeKey);
}

TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap30findSampleWithPresentationTimeERKN3WTF9MediaTimeE(PresentationOrderSampleMap* self, const MediaTime& time) {
    return Find<PresentationNode>(Checked(self, "PresentationOrderSampleMap::findSampleWithPresentationTime", "this").samples, time);
}

TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap36findSampleContainingPresentationTimeERKN3WTF9MediaTimeE(PresentationOrderSampleMap* self, const MediaTime& time) {
    Tree& tree = Checked(self, "PresentationOrderSampleMap::findSampleContainingPresentationTime", "this").samples;
    TreeNodeBase* found = UpperBound<PresentationNode>(tree, time);
    if (found == tree.beginNode) return EndNode(tree);
    found = Previous(found);
    if (Less(time, PresentationEndTime(NodeSample<PresentationNode>(found)))) return found;
    return EndNode(tree);
}

TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap43findSampleContainingOrAfterPresentationTimeERKN3WTF9MediaTimeE(PresentationOrderSampleMap* self, const MediaTime& time) {
    Tree& tree = Checked(self, "PresentationOrderSampleMap::findSampleContainingOrAfterPresentationTime", "this").samples;
    if (!tree.size) return EndNode(tree);
    TreeNodeBase* found = UpperBound<PresentationNode>(tree, time);
    if (found == tree.beginNode) return found;
    TreeNodeBase* previous = Previous(found);
    if (Less(time, PresentationEndTime(NodeSample<PresentationNode>(previous)))) return previous;
    return found;
}

TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap43findSampleStartingOnOrAfterPresentationTimeERKN3WTF9MediaTimeE(PresentationOrderSampleMap* self, const MediaTime& time) {
    return FindSampleStartingOnOrAfter(Checked(self, "PresentationOrderSampleMap::findSampleStartingOnOrAfterPresentationTime", "this"), time);
}

TreeNodeBase* APS5_VABI _ZN7WebCore26PresentationOrderSampleMap39findSampleStartingAfterPresentationTimeERKN3WTF9MediaTimeE(PresentationOrderSampleMap* self, const MediaTime& time) {
    return UpperBound<PresentationNode>(Checked(self, "PresentationOrderSampleMap::findSampleStartingAfterPresentationTime", "this").samples, time);
}

ReverseIterator APS5_VABI _ZN7WebCore26PresentationOrderSampleMap43reverseFindSampleContainingPresentationTimeERKN3WTF9MediaTimeE(PresentationOrderSampleMap* self, const MediaTime& time) {
    Tree& tree = Checked(self, "PresentationOrderSampleMap::reverseFindSampleContainingPresentationTime", "this").samples;
    const auto range = std::equal_range(RBegin(tree), REnd(tree), time, [](const auto& a, const auto& b) {
        if constexpr (std::is_same_v<std::decay_t<decltype(a)>, TreeNodeBase*>)
            return Less(b, PresentationTime(NodeSample<PresentationNode>(a)));
        else
            return !Less(a, PresentationEndTime(NodeSample<PresentationNode>(b)));
    });
    if (range.first == range.second) return MakeReverse(REnd(tree));
    return MakeReverse(range.first);
}

ReverseIterator APS5_VABI _ZN7WebCore26PresentationOrderSampleMap39reverseFindSampleBeforePresentationTimeERKN3WTF9MediaTimeE(PresentationOrderSampleMap* self, const MediaTime& time) {
    return MakeReverse(ReverseFindSampleBefore(Checked(self, "PresentationOrderSampleMap::reverseFindSampleBeforePresentationTime", "this"), time));
}

IteratorRange APS5_VABI _ZN7WebCore26PresentationOrderSampleMap35findSamplesBetweenPresentationTimesERKN3WTF9MediaTimeES4_(PresentationOrderSampleMap* self, const MediaTime& beginTime, const MediaTime& endTime) {
    Tree& tree = Checked(self, "PresentationOrderSampleMap::findSamplesBetweenPresentationTimes", "this").samples;
    TreeNodeBase* lower = LowerBound<PresentationNode>(tree, beginTime);
    TreeNodeBase* upper = LowerBound<PresentationNode>(tree, endTime);
    if (lower == upper) return {EndNode(tree), EndNode(tree)};
    return {lower, upper};
}

IteratorRange APS5_VABI _ZN7WebCore26PresentationOrderSampleMap42findSamplesBetweenPresentationTimesFromEndERKN3WTF9MediaTimeES4_(PresentationOrderSampleMap* self, const MediaTime& beginTime, const MediaTime& endTime) {
    Tree& tree = Checked(self, "PresentationOrderSampleMap::findSamplesBetweenPresentationTimesFromEnd", "this").samples;
    const ReverseCursor rangeEnd = std::find_if(RBegin(tree), REnd(tree), [&](TreeNodeBase* node) { return Less(NodeKey<PresentationNode>(node), endTime); });
    const ReverseCursor rangeStart = std::find_if(rangeEnd, REnd(tree), [&](TreeNodeBase* node) { return Less(NodeKey<PresentationNode>(node), beginTime); });
    if (rangeStart == rangeEnd) return {EndNode(tree), EndNode(tree)};
    return {rangeStart.current, rangeEnd.current};
}

TreeNodeBase* APS5_VABI _ZN7WebCore20DecodeOrderSampleMap23findSampleWithDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(DecodeOrderSampleMap* self, const DecodeKey& key) {
    return FindWithDecodeKey(Checked(self, "DecodeOrderSampleMap::findSampleWithDecodeKey", "this"), key);
}

TreeNodeBase* APS5_VABI _ZN7WebCore20DecodeOrderSampleMap24findSampleAfterDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(DecodeOrderSampleMap* self, const DecodeKey& key) {
    return UpperBound<DecodeNode>(Checked(self, "DecodeOrderSampleMap::findSampleAfterDecodeKey", "this").samples, key);
}

ReverseIterator APS5_VABI _ZN7WebCore20DecodeOrderSampleMap30reverseFindSampleWithDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(DecodeOrderSampleMap* self, const DecodeKey& key) {
    return MakeReverse(ReverseFindWithDecodeKey(Checked(self, "DecodeOrderSampleMap::reverseFindSampleWithDecodeKey", "this"), key));
}

ReverseIterator APS5_VABI _ZN7WebCore20DecodeOrderSampleMap37findSyncSamplePriorToPresentationTimeERKN3WTF9MediaTimeES4_(DecodeOrderSampleMap* self, const MediaTime& time, const MediaTime& threshold) {
    auto& map = Checked(self, "DecodeOrderSampleMap::findSyncSamplePriorToPresentationTime", "this");
    const ReverseCursor current = ReverseFindSampleBefore(map.presentationOrder, time);
    if (current == REnd(map.presentationOrder.samples)) return MakeReverse(REnd(map.samples));
    const ReverseCursor decodeCurrent = ReverseFindWithDecodeKey(map, KeyOf(NodeSample<PresentationNode>(*current)));
    const ReverseCursor found = FindSyncSamplePrior(map.samples, decodeCurrent);
    if (found == REnd(map.samples)) return MakeReverse(REnd(map.samples));
    if (Less(PresentationTime(NodeSample<DecodeNode>(*found)), Minus(time, threshold))) return MakeReverse(REnd(map.samples));
    return MakeReverse(found);
}

TreeNodeBase* APS5_VABI _ZN7WebCore20DecodeOrderSampleMap35findSyncSampleAfterPresentationTimeERKN3WTF9MediaTimeES4_(DecodeOrderSampleMap* self, const MediaTime& time, const MediaTime& threshold) {
    auto& map = Checked(self, "DecodeOrderSampleMap::findSyncSampleAfterPresentationTime", "this");
    TreeNodeBase* current = FindSampleStartingOnOrAfter(map.presentationOrder, time);
    if (current == EndNode(map.presentationOrder.samples)) return EndNode(map.samples);
    TreeNodeBase* decodeCurrent = FindWithDecodeKey(map, KeyOf(NodeSample<PresentationNode>(current)));
    const MediaTime upperBound = Plus(time, threshold);
    TreeNodeBase* found = decodeCurrent;
    while (found != EndNode(map.samples) && !IsSync(NodeSample<DecodeNode>(found))) found = Next(found);
    if (found == EndNode(map.samples)) return found;
    if (Less(upperBound, PresentationTime(NodeSample<DecodeNode>(found)))) return EndNode(map.samples);
    return found;
}

ReverseIteratorRange APS5_VABI _ZN7WebCore20DecodeOrderSampleMap20findDependentSamplesEPNS_11MediaSampleE(DecodeOrderSampleMap* self, MediaSample* sample) {
    auto& map = Checked(self, "DecodeOrderSampleMap::findDependentSamples", "this");
    Checked(sample, "DecodeOrderSampleMap::findDependentSamples", "sample");
    const ReverseCursor current = ReverseFindWithDecodeKey(map, KeyOf(sample));
    return ReverseIteratorRange{MakeReverse(current), MakeReverse(FindSyncSamplePrior(map.samples, current))};
}

IteratorRange APS5_VABI _ZN7WebCore20DecodeOrderSampleMap28findSamplesBetweenDecodeKeysERKSt4pairIN3WTF9MediaTimeES3_ES6_(DecodeOrderSampleMap* self, const DecodeKey& beginKey, const DecodeKey& endKey) {
    Tree& tree = Checked(self, "DecodeOrderSampleMap::findSamplesBetweenDecodeKeys", "this").samples;
    if (Less(endKey, beginKey)) return {EndNode(tree), EndNode(tree)};
    TreeNodeBase* lower = LowerBound<DecodeNode>(tree, beginKey);
    TreeNodeBase* upper = LowerBound<DecodeNode>(tree, endKey);
    if (lower == upper) return {EndNode(tree), EndNode(tree)};
    return {lower, upper};
}

}
