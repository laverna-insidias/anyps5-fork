#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <stdexcept>
#include <string>
#include <utility>

namespace {

using NKWebKit::PasteboardAbi::Entry;
using NKWebKit::PasteboardAbi::PasteboardCustomData;
using NKWebKit::PasteboardAbi::SharedBuffer;
using NKWebKit::WtfAbi::String;
using NKWebKit::WtfAbi::StringImpl;
using NKWebKit::WtfAbi::Vector;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "PasteboardCustomData check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(value) Check((value), __LINE__)

String Make(const char* text) {
    return _ZN3WTF10StringImpl6createEPKhj(reinterpret_cast<const unsigned char*>(text), static_cast<unsigned>(std::strlen(text)));
}

std::string Text(const String& string) {
    if (!string.impl) return "<null>";
    return std::string(static_cast<const char*>(string.impl->data), string.impl->length);
}

template <typename T>
struct Storage {
    alignas(T) unsigned char bytes[sizeof(T)];
    T* get() { return reinterpret_cast<T*>(bytes); }
};

void MakeEntry(Entry* entry, const char* type, const char* customData, const char* platform) {
    _ZN7WebCore20PasteboardCustomData5EntryC1Ev(entry);
    entry->type = Make(type);
    entry->customData = Make(customData);
    entry->platformData.string = Make(platform);
}

void TestEntryDefault() {
    Storage<Entry> storage;
    std::memset(storage.bytes, 0xcc, sizeof(storage.bytes));
    _ZN7WebCore20PasteboardCustomData5EntryC2Ev(storage.get());
    Entry& entry = *storage.get();
    Require(!entry.type.impl && !entry.customData.impl);
    Require(entry.platformData.index == 0 && !entry.platformData.string.impl);
    entry.~Entry();
}

void TestEntryCopyAndMove() {
    Storage<Entry> first;
    MakeEntry(first.get(), "text/plain", "custom", "platform");
    const unsigned before = first.get()->type.impl->refCount;

    Storage<Entry> copy;
    _ZN7WebCore20PasteboardCustomData5EntryC1ERKS1_(copy.get(), *first.get());
    Require(copy.get()->type.impl == first.get()->type.impl);
    Require(first.get()->type.impl->refCount == before + 2);
    Require(Text(copy.get()->customData) == "custom");
    Require(copy.get()->platformData.index == 0 && Text(copy.get()->platformData.string) == "platform");

    Storage<Entry> moved;
    _ZN7WebCore20PasteboardCustomData5EntryC2EOS1_(moved.get(), std::move(*copy.get()));
    Require(!copy.get()->type.impl && !copy.get()->customData.impl && !copy.get()->platformData.string.impl);
    Require(Text(moved.get()->type) == "text/plain" && Text(moved.get()->platformData.string) == "platform");
    copy.get()->~Entry();
    moved.get()->~Entry();
    Require(first.get()->type.impl->refCount == before);
    first.get()->~Entry();
}

void TestEntryAssignment() {
    Storage<Entry> source;
    Storage<Entry> target;
    MakeEntry(source.get(), "a", "b", "c");
    MakeEntry(target.get(), "x", "y", "z");
    Require(_ZN7WebCore20PasteboardCustomData5EntryaSERKS1_(target.get(), *source.get()) == target.get());
    Require(Text(target.get()->type) == "a" && Text(target.get()->customData) == "b" && Text(target.get()->platformData.string) == "c");
    Require(Text(source.get()->type) == "a");

    Storage<Entry> other;
    MakeEntry(other.get(), "d", "e", "f");
    Require(_ZN7WebCore20PasteboardCustomData5EntryaSEOS1_(target.get(), std::move(*other.get())) == target.get());
    Require(Text(target.get()->type) == "d" && Text(target.get()->platformData.string) == "f");
    Require(!other.get()->type.impl && !other.get()->platformData.string.impl);
    source.get()->~Entry();
    target.get()->~Entry();
    other.get()->~Entry();
}

void TestEntryBuffer() {
    SharedBuffer buffer{2};
    Storage<Entry> entry;
    _ZN7WebCore20PasteboardCustomData5EntryC1Ev(entry.get());
    entry.get()->platformData.string.~String();
    entry.get()->platformData.buffer = &buffer;
    entry.get()->platformData.index = 1;
    NKWebKit::PasteboardAbi::Ref(&buffer);
    Require(buffer.refCount == 3);

    Storage<Entry> copy;
    _ZN7WebCore20PasteboardCustomData5EntryC1ERKS1_(copy.get(), *entry.get());
    Require(copy.get()->platformData.index == 1 && copy.get()->platformData.buffer == &buffer);
    Require(buffer.refCount == 4);

    Storage<Entry> moved;
    _ZN7WebCore20PasteboardCustomData5EntryC1EOS1_(moved.get(), std::move(*copy.get()));
    Require(copy.get()->platformData.index == 1 && !copy.get()->platformData.buffer);
    Require(buffer.refCount == 4);
    copy.get()->~Entry();
    Require(buffer.refCount == 4);

    Storage<Entry> text;
    MakeEntry(text.get(), "t", "u", "v");
    _ZN7WebCore20PasteboardCustomData5EntryaSERKS1_(text.get(), *moved.get());
    Require(text.get()->platformData.index == 1 && buffer.refCount == 5);
    _ZN7WebCore20PasteboardCustomData5EntryaSERKS1_(moved.get(), *entry.get());
    Require(buffer.refCount == 5);

    Storage<Entry> plain;
    MakeEntry(plain.get(), "p", "q", "r");
    _ZN7WebCore20PasteboardCustomData5EntryaSERKS1_(text.get(), *plain.get());
    Require(text.get()->platformData.index == 0 && Text(text.get()->platformData.string) == "r");
    Require(buffer.refCount == 4);

    plain.get()->~Entry();
    text.get()->~Entry();
    moved.get()->~Entry();
    entry.get()->~Entry();
    Require(buffer.refCount == 2);
}

void TestEntryValueless() {
    Storage<Entry> entry;
    _ZN7WebCore20PasteboardCustomData5EntryC1Ev(entry.get());
    entry.get()->platformData.string.~String();
    entry.get()->platformData.index = 0xff;
    Storage<Entry> copy;
    _ZN7WebCore20PasteboardCustomData5EntryC1ERKS1_(copy.get(), *entry.get());
    Require(copy.get()->platformData.index == 0 && !copy.get()->platformData.string.impl);
    copy.get()->~Entry();
    entry.get()->~Entry();
}

void TestCustomData() {
    Storage<PasteboardCustomData> empty;
    _ZN7WebCore20PasteboardCustomDataC1Ev(empty.get());
    Require(!empty.get()->origin.impl && empty.get()->data.size == 0 && !empty.get()->data.buffer);
    _ZN7WebCore20PasteboardCustomDataD1Ev(empty.get());

    Vector<Entry> entries;
    for (const char* type : {"text/plain", "text/html", "text/uri-list"}) {
        Storage<Entry> entry;
        MakeEntry(entry.get(), type, "c", "p");
        entries.append(*entry.get());
        entry.get()->~Entry();
    }
    String origin = Make("https://example.com");
    StringImpl* originImpl = origin.impl;
    Storage<PasteboardCustomData> data;
    _ZN7WebCore20PasteboardCustomDataC2EON3WTF6StringEONS1_6VectorINS0_5EntryELm0ENS1_15CrashOnOverflowELm16ENS1_10FastMallocEEE(data.get(), std::move(origin), std::move(entries));
    Require(!origin.impl && entries.size == 0);
    Require(data.get()->origin.impl == originImpl && data.get()->data.size == 3);
    Require(Text(data.get()->data[2].type) == "text/uri-list");

    Storage<PasteboardCustomData> copy;
    _ZN7WebCore20PasteboardCustomDataC1ERKS0_(copy.get(), *data.get());
    Require(copy.get()->origin.impl == originImpl && copy.get()->data.size == 3);
    Require(copy.get()->data.buffer != data.get()->data.buffer);
    Require(copy.get()->data[1].type.impl == data.get()->data[1].type.impl);

    Storage<PasteboardCustomData> moved;
    _ZN7WebCore20PasteboardCustomDataC2EOS0_(moved.get(), std::move(*copy.get()));
    Require(!copy.get()->origin.impl && copy.get()->data.size == 0 && !copy.get()->data.buffer);
    Require(moved.get()->data.size == 3);

    Storage<PasteboardCustomData> assigned;
    _ZN7WebCore20PasteboardCustomDataC1Ev(assigned.get());
    Require(_ZN7WebCore20PasteboardCustomDataaSERKS0_(assigned.get(), *moved.get()) == assigned.get());
    Require(assigned.get()->origin.impl == originImpl && assigned.get()->data.size == 3);
    Require(Text(assigned.get()->data[0].platformData.string) == "p");
    _ZN7WebCore20PasteboardCustomDataaSERKS0_(assigned.get(), *assigned.get());
    Require(assigned.get()->data.size == 3 && Text(assigned.get()->data[0].type) == "text/plain");

    const unsigned before = originImpl->refCount;
    _ZN7WebCore20PasteboardCustomDataD2Ev(assigned.get());
    _ZN7WebCore20PasteboardCustomDataD1Ev(moved.get());
    _ZN7WebCore20PasteboardCustomDataD1Ev(copy.get());
    Require(originImpl->refCount == before - 4);
    _ZN7WebCore20PasteboardCustomDataD1Ev(data.get());
}

void TestNullObject() {
    bool threw = false;
    try {
        _ZN7WebCore20PasteboardCustomDataC1Ev(nullptr);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);
}

}  // namespace

int main() {
    TestEntryDefault();
    TestEntryCopyAndMove();
    TestEntryAssignment();
    TestEntryBuffer();
    TestEntryValueless();
    TestCustomData();
    TestNullObject();
    return 0;
}
