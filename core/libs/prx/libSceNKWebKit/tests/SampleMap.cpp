#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <stdexcept>
#include <vector>

using NKWebKit::SampleMapAbi::DecodeKey;
using NKWebKit::SampleMapAbi::DecodeNode;
using NKWebKit::SampleMapAbi::IteratorRange;
using NKWebKit::SampleMapAbi::MediaSample;
using NKWebKit::SampleMapAbi::PresentationNode;
using NKWebKit::SampleMapAbi::ReverseIterator;
using NKWebKit::SampleMapAbi::ReverseIteratorRange;
using NKWebKit::SampleMapAbi::SampleMap;
using NKWebKit::SampleMapAbi::Tree;
using NKWebKit::SampleMapAbi::TreeNodeBase;
using WTF::MediaTime;

namespace {

void Check(bool condition, int line) {
    if (!condition) {
        std::fprintf(stderr, "SampleMap check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(cond) Check((cond), __LINE__)

struct FakeSample {
    MediaSample base;
    MediaTime presentation;
    MediaTime decode;
    MediaTime duration;
    std::size_t size;
    int flags;
    int destroyed;
};

const FakeSample* AsFake(const MediaSample* sample) {
    return reinterpret_cast<const FakeSample*>(sample);
}

MediaTime APS5_VABI FakePresentationTime(const MediaSample* sample) {
    return AsFake(sample)->presentation;
}

MediaTime APS5_VABI FakeDecodeTime(const MediaSample* sample) {
    return AsFake(sample)->decode;
}

MediaTime APS5_VABI FakeDuration(const MediaSample* sample) {
    return AsFake(sample)->duration;
}

std::size_t APS5_VABI FakeSizeInBytes(const MediaSample* sample) {
    return AsFake(sample)->size;
}

int APS5_VABI FakeFlags(const MediaSample* sample) {
    return AsFake(sample)->flags;
}

void APS5_VABI FakeDeletingDestructor(MediaSample* sample) {
    ++reinterpret_cast<FakeSample*>(sample)->destroyed;
}

const void* const FakeVtable[] = {
    nullptr,
    reinterpret_cast<const void*>(&FakeDeletingDestructor),
    reinterpret_cast<const void*>(&FakePresentationTime),
    reinterpret_cast<const void*>(&FakeDecodeTime),
    reinterpret_cast<const void*>(&FakeDuration),
    nullptr,
    reinterpret_cast<const void*>(&FakeSizeInBytes),
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    reinterpret_cast<const void*>(&FakeFlags),
};

MediaTime Time(int64_t value) {
    return MediaTime(value, 1, MediaTime::Valid);
}

FakeSample MakeSample(int64_t presentation, int64_t decode, int64_t duration, std::size_t size, bool sync) {
    FakeSample sample{};
    sample.base.vtable = FakeVtable;
    sample.base.refCount = 1;
    sample.presentation = Time(presentation);
    sample.decode = Time(decode);
    sample.duration = Time(duration);
    sample.size = size;
    sample.flags = sync ? NKWebKit::SampleMapAbi::MediaSampleIsSync : 0;
    return sample;
}

void InitTree(Tree& tree) {
    tree.beginNode = reinterpret_cast<TreeNodeBase*>(&tree.root);
    tree.root = nullptr;
    tree.size = 0;
}

void InitMap(SampleMap& map) {
    InitTree(map.decodeOrder.samples);
    InitTree(map.decodeOrder.presentationOrder.samples);
    map.totalSize = 0;
}

TreeNodeBase* End(Tree& tree) {
    return reinterpret_cast<TreeNodeBase*>(&tree.root);
}

Tree& Presentation(SampleMap& map) {
    return map.decodeOrder.presentationOrder.samples;
}

Tree& Decode(SampleMap& map) {
    return map.decodeOrder.samples;
}

int64_t PresentationOf(TreeNodeBase* node) {
    return reinterpret_cast<PresentationNode*>(node)->key.m_timeValue;
}

int64_t DecodeOf(TreeNodeBase* node) {
    return reinterpret_cast<DecodeNode*>(node)->key.decodeTime.m_timeValue;
}

TreeNodeBase* Next(TreeNodeBase* node) {
    if (node->right) {
        node = node->right;
        while (node->left) node = node->left;
        return node;
    }
    while (node != node->parent->left) node = node->parent;
    return node->parent;
}

TreeNodeBase* Previous(TreeNodeBase* node) {
    if (node->left) {
        node = node->left;
        while (node->right) node = node->right;
        return node;
    }
    while (node == node->parent->left) node = node->parent;
    return node->parent;
}

TreeNodeBase* Deref(const ReverseIterator& iterator) {
    return Previous(iterator.current);
}

int BlackHeight(TreeNodeBase* node, TreeNodeBase* parent, std::size_t& count) {
    if (!node) return 1;
    Require(node->parent == parent);
    if (!node->isBlack) {
        Require(!node->left || node->left->isBlack);
        Require(!node->right || node->right->isBlack);
    }
    ++count;
    const int left = BlackHeight(node->left, node, count);
    const int right = BlackHeight(node->right, node, count);
    Require(left == right);
    return left + (node->isBlack ? 1 : 0);
}

template<typename KeyOf>
void RequireValidTree(Tree& tree, KeyOf keyOf) {
    std::size_t count = 0;
    if (tree.root) Require(tree.root->isBlack);
    BlackHeight(tree.root, End(tree), count);
    Require(count == tree.size);
    if (!tree.root) {
        Require(tree.beginNode == End(tree));
        return;
    }
    TreeNodeBase* minimum = tree.root;
    while (minimum->left) minimum = minimum->left;
    Require(tree.beginNode == minimum);
    std::size_t walked = 0;
    for (TreeNodeBase* node = tree.beginNode; node != End(tree); node = Next(node)) {
        TreeNodeBase* next = Next(node);
        if (next != End(tree)) Require(keyOf(node) < keyOf(next));
        ++walked;
    }
    Require(walked == tree.size);
}

void RequireValid(SampleMap& map) {
    RequireValidTree(Presentation(map), PresentationOf);
    RequireValidTree(Decode(map), [](TreeNodeBase* node) {
        const auto& key = reinterpret_cast<DecodeNode*>(node)->key;
        return std::pair(key.decodeTime.m_timeValue, key.presentationTime.m_timeValue);
    });
    Require(Presentation(map).size == Decode(map).size);
}

void TestEmptyMap() {
    SampleMap map;
    InitMap(map);
    Require(_ZNK7WebCore9SampleMap5emptyEv(&map));
    const MediaTime time = Time(3);
    auto* presentation = &map.decodeOrder.presentationOrder;
    Require(_ZN7WebCore26PresentationOrderSampleMap30findSampleWithPresentationTimeERKN3WTF9MediaTimeE(presentation, time) == End(Presentation(map)));
    Require(_ZN7WebCore26PresentationOrderSampleMap36findSampleContainingPresentationTimeERKN3WTF9MediaTimeE(presentation, time) == End(Presentation(map)));
    Require(_ZN7WebCore26PresentationOrderSampleMap43findSampleContainingOrAfterPresentationTimeERKN3WTF9MediaTimeE(presentation, time) == End(Presentation(map)));
    Require(_ZN7WebCore26PresentationOrderSampleMap39reverseFindSampleBeforePresentationTimeERKN3WTF9MediaTimeE(presentation, time).current == Presentation(map).beginNode);
    Require(_ZN7WebCore26PresentationOrderSampleMap43reverseFindSampleContainingPresentationTimeERKN3WTF9MediaTimeE(presentation, time).current == Presentation(map).beginNode);
    Require(_ZN7WebCore20DecodeOrderSampleMap37findSyncSamplePriorToPresentationTimeERKN3WTF9MediaTimeES4_(&map.decodeOrder, time, time).current == Decode(map).beginNode);
    Require(_ZN7WebCore20DecodeOrderSampleMap35findSyncSampleAfterPresentationTimeERKN3WTF9MediaTimeES4_(&map.decodeOrder, time, time) == End(Decode(map)));
    _ZN7WebCore9SampleMap5clearEv(&map);
    Require(_ZNK7WebCore9SampleMap5emptyEv(&map));
}

void TestAddRemoveAndClear() {
    SampleMap map;
    InitMap(map);
    FakeSample first = MakeSample(0, 0, 1, 100, true);
    FakeSample second = MakeSample(1, 1, 1, 50, false);
    _ZN7WebCore9SampleMap9addSampleERNS_11MediaSampleE(&map, first.base);
    _ZN7WebCore9SampleMap9addSampleERNS_11MediaSampleE(&map, second.base);
    Require(!_ZNK7WebCore9SampleMap5emptyEv(&map));
    Require(map.totalSize == 150);
    Require(first.base.refCount == 3);
    Require(second.base.refCount == 3);
    RequireValid(map);

    FakeSample duplicate = MakeSample(1, 1, 1, 7, false);
    _ZN7WebCore9SampleMap9addSampleERNS_11MediaSampleE(&map, duplicate.base);
    Require(duplicate.base.refCount == 1);
    Require(map.totalSize == 157);
    Require(Presentation(map).size == 2);

    _ZN7WebCore9SampleMap12removeSampleEPNS_11MediaSampleE(&map, &duplicate.base);
    Require(map.totalSize == 150);
    Require(Presentation(map).size == 1);
    Require(second.base.refCount == 1);
    Require(second.destroyed == 0);
    RequireValid(map);

    _ZN7WebCore9SampleMap5clearEv(&map);
    Require(_ZNK7WebCore9SampleMap5emptyEv(&map));
    Require(map.totalSize == 0);
    Require(first.base.refCount == 1);
    RequireValid(map);

    FakeSample owned = MakeSample(4, 4, 1, 10, true);
    _ZN7WebCore9SampleMap9addSampleERNS_11MediaSampleE(&map, owned.base);
    owned.base.refCount -= 1;
    _ZN7WebCore9SampleMap5clearEv(&map);
    Require(owned.destroyed == 1);
}

void TestRedBlackStress() {
    SampleMap map;
    InitMap(map);
    std::vector<FakeSample> samples;
    samples.reserve(300);
    for (int i = 0; i < 300; ++i) samples.push_back(MakeSample(i * 2, i * 2 - 1, 2, 1, i % 10 == 0));
    std::vector<int> order(samples.size());
    for (std::size_t i = 0; i < order.size(); ++i) order[i] = static_cast<int>(i);
    std::mt19937 random(1234);
    std::shuffle(order.begin(), order.end(), random);
    for (int index : order) {
        _ZN7WebCore9SampleMap9addSampleERNS_11MediaSampleE(&map, samples[index].base);
        RequireValid(map);
    }
    Require(map.totalSize == 300);
    std::shuffle(order.begin(), order.end(), random);
    for (std::size_t i = 0; i < 200; ++i) {
        _ZN7WebCore9SampleMap12removeSampleEPNS_11MediaSampleE(&map, &samples[order[i]].base);
        RequireValid(map);
    }
    Require(map.totalSize == 100);
    for (std::size_t i = 0; i < 200; ++i) Require(samples[order[i]].base.refCount == 1);
    for (std::size_t i = 200; i < 300; ++i) Require(samples[order[i]].base.refCount == 3);
    for (std::size_t i = 200; i < 300; ++i) {
        _ZN7WebCore9SampleMap12removeSampleEPNS_11MediaSampleE(&map, &samples[order[i]].base);
        RequireValid(map);
    }
    Require(_ZNK7WebCore9SampleMap5emptyEv(&map));
    for (const FakeSample& sample : samples) Require(sample.destroyed == 0);
}

struct Fixture {
    Fixture() {
        InitMap(map);
        samples.push_back(MakeSample(0, 0, 10, 1, true));
        samples.push_back(MakeSample(30, 10, 10, 1, false));
        samples.push_back(MakeSample(10, 20, 10, 1, false));
        samples.push_back(MakeSample(20, 30, 10, 1, false));
        samples.push_back(MakeSample(40, 40, 10, 1, true));
        samples.push_back(MakeSample(60, 50, 10, 1, false));
        samples.push_back(MakeSample(50, 60, 10, 1, false));
        for (FakeSample& sample : samples) _ZN7WebCore9SampleMap9addSampleERNS_11MediaSampleE(&map, sample.base);
        RequireValid(map);
    }
    ~Fixture() { _ZN7WebCore9SampleMap5clearEv(&map); }

    SampleMap map;
    std::vector<FakeSample> samples;
};

void TestPresentationFinds() {
    Fixture fixture;
    SampleMap& map = fixture.map;
    auto* presentation = &map.decodeOrder.presentationOrder;
    TreeNodeBase* end = End(Presentation(map));

    Require(PresentationOf(_ZN7WebCore26PresentationOrderSampleMap30findSampleWithPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(20))) == 20);
    Require(_ZN7WebCore26PresentationOrderSampleMap30findSampleWithPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(25)) == end);

    Require(PresentationOf(_ZN7WebCore26PresentationOrderSampleMap36findSampleContainingPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(25))) == 20);
    Require(PresentationOf(_ZN7WebCore26PresentationOrderSampleMap36findSampleContainingPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(0))) == 0);
    Require(_ZN7WebCore26PresentationOrderSampleMap36findSampleContainingPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(-1)) == end);
    Require(_ZN7WebCore26PresentationOrderSampleMap36findSampleContainingPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(70)) == end);

    Require(PresentationOf(_ZN7WebCore26PresentationOrderSampleMap43findSampleContainingOrAfterPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(-5))) == 0);
    Require(PresentationOf(_ZN7WebCore26PresentationOrderSampleMap43findSampleContainingOrAfterPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(15))) == 10);
    Require(_ZN7WebCore26PresentationOrderSampleMap43findSampleContainingOrAfterPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(70)) == end);

    Require(PresentationOf(_ZN7WebCore26PresentationOrderSampleMap43findSampleStartingOnOrAfterPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(30))) == 30);
    Require(PresentationOf(_ZN7WebCore26PresentationOrderSampleMap43findSampleStartingOnOrAfterPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(31))) == 40);
    Require(PresentationOf(_ZN7WebCore26PresentationOrderSampleMap39findSampleStartingAfterPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(30))) == 40);
    Require(_ZN7WebCore26PresentationOrderSampleMap39findSampleStartingAfterPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(60)) == end);

    ReverseIterator containing = _ZN7WebCore26PresentationOrderSampleMap43reverseFindSampleContainingPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(45));
    Require(PresentationOf(Deref(containing)) == 40);
    Require(containing.unused == containing.current);
    Require(_ZN7WebCore26PresentationOrderSampleMap43reverseFindSampleContainingPresentationTimeERKN3WTF9MediaTimeE(presentation, Time(80)).current == Presentation(map).beginNode);

    Require(PresentationOf(Deref(_ZN7WebCore26PresentationOrderSampleMap39reverseFindSampleBeforePresentationTimeERKN3WTF9MediaTimeE(presentation, Time(35)))) == 30);
    Require(PresentationOf(Deref(_ZN7WebCore26PresentationOrderSampleMap39reverseFindSampleBeforePresentationTimeERKN3WTF9MediaTimeE(presentation, Time(30)))) == 30);
    Require(PresentationOf(Deref(_ZN7WebCore26PresentationOrderSampleMap39reverseFindSampleBeforePresentationTimeERKN3WTF9MediaTimeE(presentation, Time(99)))) == 60);
    Require(_ZN7WebCore26PresentationOrderSampleMap39reverseFindSampleBeforePresentationTimeERKN3WTF9MediaTimeE(presentation, Time(-1)).current == Presentation(map).beginNode);

    IteratorRange between = _ZN7WebCore26PresentationOrderSampleMap35findSamplesBetweenPresentationTimesERKN3WTF9MediaTimeES4_(presentation, Time(10), Time(40));
    Require(PresentationOf(between.first) == 10);
    Require(PresentationOf(between.second) == 40);
    IteratorRange none = _ZN7WebCore26PresentationOrderSampleMap35findSamplesBetweenPresentationTimesERKN3WTF9MediaTimeES4_(presentation, Time(11), Time(19));
    Require(none.first == end && none.second == end);

    IteratorRange fromEnd = _ZN7WebCore26PresentationOrderSampleMap42findSamplesBetweenPresentationTimesFromEndERKN3WTF9MediaTimeES4_(presentation, Time(10), Time(40));
    Require(PresentationOf(fromEnd.first) == 10);
    Require(PresentationOf(fromEnd.second) == 40);
    IteratorRange tail = _ZN7WebCore26PresentationOrderSampleMap42findSamplesBetweenPresentationTimesFromEndERKN3WTF9MediaTimeES4_(presentation, Time(45), Time(100));
    Require(PresentationOf(tail.first) == 50);
    Require(tail.second == end);
    IteratorRange noneFromEnd = _ZN7WebCore26PresentationOrderSampleMap42findSamplesBetweenPresentationTimesFromEndERKN3WTF9MediaTimeES4_(presentation, Time(11), Time(19));
    Require(noneFromEnd.first == end && noneFromEnd.second == end);
}

void TestDecodeFinds() {
    Fixture fixture;
    SampleMap& map = fixture.map;
    auto* decode = &map.decodeOrder;
    TreeNodeBase* end = End(Decode(map));

    const DecodeKey key{Time(20), Time(10)};
    Require(DecodeOf(_ZN7WebCore20DecodeOrderSampleMap23findSampleWithDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(decode, key)) == 20);
    const DecodeKey missing{Time(20), Time(11)};
    Require(_ZN7WebCore20DecodeOrderSampleMap23findSampleWithDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(decode, missing) == end);
    Require(DecodeOf(_ZN7WebCore20DecodeOrderSampleMap24findSampleAfterDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(decode, key)) == 30);
    Require(DecodeOf(_ZN7WebCore20DecodeOrderSampleMap24findSampleAfterDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(decode, missing)) == 30);

    Require(DecodeOf(Deref(_ZN7WebCore20DecodeOrderSampleMap30reverseFindSampleWithDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(decode, key))) == 20);
    Require(_ZN7WebCore20DecodeOrderSampleMap30reverseFindSampleWithDecodeKeyERKSt4pairIN3WTF9MediaTimeES3_E(decode, missing).current == Decode(map).beginNode);

    ReverseIterator prior = _ZN7WebCore20DecodeOrderSampleMap37findSyncSamplePriorToPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(35), Time(100));
    Require(DecodeOf(Deref(prior)) == 0);
    Require(_ZN7WebCore20DecodeOrderSampleMap37findSyncSamplePriorToPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(35), Time(10)).current == Decode(map).beginNode);
    Require(DecodeOf(Deref(_ZN7WebCore20DecodeOrderSampleMap37findSyncSamplePriorToPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(65), Time(30)))) == 40);
    Require(_ZN7WebCore20DecodeOrderSampleMap37findSyncSamplePriorToPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(-1), Time(100)).current == Decode(map).beginNode);

    Require(DecodeOf(_ZN7WebCore20DecodeOrderSampleMap35findSyncSampleAfterPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(5), Time(40))) == 40);
    Require(_ZN7WebCore20DecodeOrderSampleMap35findSyncSampleAfterPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(5), Time(30)) == end);
    Require(DecodeOf(_ZN7WebCore20DecodeOrderSampleMap35findSyncSampleAfterPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(0), Time(0))) == 0);
    Require(_ZN7WebCore20DecodeOrderSampleMap35findSyncSampleAfterPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(55), Time(100)) == end);
    Require(_ZN7WebCore20DecodeOrderSampleMap35findSyncSampleAfterPresentationTimeERKN3WTF9MediaTimeES4_(decode, Time(61), Time(100)) == end);

    ReverseIteratorRange dependents = _ZN7WebCore20DecodeOrderSampleMap20findDependentSamplesEPNS_11MediaSampleE(decode, &fixture.samples[3].base);
    Require(DecodeOf(Deref(dependents.first)) == 30);
    Require(DecodeOf(Deref(dependents.second)) == 0);
    std::vector<int64_t> walked;
    for (TreeNodeBase* base = dependents.first.current; base != dependents.second.current; base = Previous(base)) walked.push_back(DecodeOf(Previous(base)));
    Require((walked == std::vector<int64_t>{30, 20, 10}));

    IteratorRange between = _ZN7WebCore20DecodeOrderSampleMap28findSamplesBetweenDecodeKeysERKSt4pairIN3WTF9MediaTimeES3_ES6_(decode, DecodeKey{Time(10), Time(0)}, DecodeKey{Time(40), Time(40)});
    Require(DecodeOf(between.first) == 10);
    Require(DecodeOf(between.second) == 40);
    IteratorRange reversed = _ZN7WebCore20DecodeOrderSampleMap28findSamplesBetweenDecodeKeysERKSt4pairIN3WTF9MediaTimeES3_ES6_(decode, DecodeKey{Time(40), Time(40)}, DecodeKey{Time(10), Time(0)});
    Require(reversed.first == end && reversed.second == end);
    IteratorRange empty = _ZN7WebCore20DecodeOrderSampleMap28findSamplesBetweenDecodeKeysERKSt4pairIN3WTF9MediaTimeES3_ES6_(decode, DecodeKey{Time(11), Time(0)}, DecodeKey{Time(19), Time(0)});
    Require(empty.first == end && empty.second == end);
}

void TestNullArguments() {
    SampleMap map;
    InitMap(map);
    bool threw = false;
    try {
        _ZN7WebCore9SampleMap12removeSampleEPNS_11MediaSampleE(&map, nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
    threw = false;
    try {
        _ZN7WebCore9SampleMap5clearEv(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
}

}  // namespace

int main() {
    TestEmptyMap();
    TestAddRemoveAndClear();
    TestRedBlackStress();
    TestPresentationFinds();
    TestDecodeFinds();
    TestNullArguments();
    std::puts("SampleMap tests passed");
    return 0;
}
