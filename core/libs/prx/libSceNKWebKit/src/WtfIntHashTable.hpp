// Derived from Source/WTF/wtf/HashTable.h@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
/*
 * Copyright (C) 2005-2022 Apple Inc. All rights reserved.
 * Copyright (C) 2008 David Levin <levin@chromium.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "prx/libSceNKWebKit/include/WtfExports.hpp"
#include "prx/libSceNKWebKit/src/WtfHashMapStringAbi.hpp"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <thread>

namespace NKWebKit::WtfAbi::HashTableAbi {

inline constexpr std::size_t MetadataSize = 4 * sizeof(unsigned);
inline constexpr unsigned MinimumTableSize = 8;
inline constexpr unsigned MinLoad = 6;
inline constexpr unsigned MaxSmallTableCapacity = 1024;
inline constexpr int TableSizeOffset = -1;
inline constexpr int TableSizeMaskOffset = -2;
inline constexpr int KeyCountOffset = -3;
inline constexpr int DeletedCountOffset = -4;

inline unsigned IntHash(std::uint32_t key) {
    key += ~(key << 15);
    key ^= (key >> 10);
    key += (key << 3);
    key ^= (key >> 6);
    key += ~(key << 11);
    key ^= (key >> 16);
    return key;
}

inline unsigned IntHash(std::uint64_t key) {
    key += ~(key << 32);
    key ^= (key >> 22);
    key += ~(key << 13);
    key ^= (key >> 8);
    key += (key << 3);
    key ^= (key >> 15);
    key += ~(key << 27);
    key ^= (key >> 31);
    return static_cast<unsigned>(key);
}

inline unsigned PtrHash(const void* key) {
    return IntHash(static_cast<std::uint64_t>(reinterpret_cast<std::uintptr_t>(key)));
}

template<typename Bucket>
unsigned* Metadata(Bucket* table) {
    return reinterpret_cast<unsigned*>(table);
}

template<typename Bucket>
unsigned TableSize(Bucket* table) {
    return table ? Metadata(table)[TableSizeOffset] : 0U;
}

template<typename Bucket>
unsigned KeyCount(Bucket* table) {
    return table ? Metadata(table)[KeyCountOffset] : 0U;
}

template<typename Bucket>
unsigned DeletedCount(Bucket* table) {
    return table ? Metadata(table)[DeletedCountOffset] : 0U;
}

inline bool ShouldExpand(std::uint64_t keyAndDeletedCount, std::uint64_t tableSize) {
    if (tableSize <= MaxSmallTableCapacity) return keyAndDeletedCount * 4 >= tableSize * 3;
    return keyAndDeletedCount * 2 >= tableSize;
}

template<typename Policy>
bool IsLive(typename Policy::Bucket& bucket) {
    return !Policy::IsEmpty(bucket) && !Policy::IsDeleted(bucket);
}

template<typename Policy>
typename Policy::Bucket* AllocateTable(unsigned tableSize) {
    using Bucket = typename Policy::Bucket;
    const std::size_t bytes = MetadataSize + std::size_t(tableSize) * sizeof(Bucket);
    auto* storage = static_cast<unsigned char*>(Policy::EmptyValueIsZero ? _ZN3WTF16fastZeroedMallocEm(bytes) : _ZN3WTF10fastMallocEm(bytes));
    auto* table = reinterpret_cast<Bucket*>(storage + MetadataSize);
    Metadata(table)[TableSizeOffset] = tableSize;
    Metadata(table)[TableSizeMaskOffset] = tableSize - 1U;
    Metadata(table)[KeyCountOffset] = 0U;
    Metadata(table)[DeletedCountOffset] = 0U;
    if constexpr (!Policy::EmptyValueIsZero)
        for (unsigned i = 0; i < tableSize; ++i) Policy::ConstructEmpty(table[i]);
    return table;
}

template<typename Policy>
void DeallocateTable(typename Policy::Bucket* table) {
    if (!table) return;
    const unsigned tableSize = TableSize(table);
    for (unsigned i = 0; i < tableSize; ++i)
        if (!Policy::IsDeleted(table[i])) Policy::DestroyValue(table[i]);
    _ZN3WTF8fastFreeEPv(reinterpret_cast<unsigned char*>(table) - MetadataSize);
}

template<typename Policy>
typename Policy::Bucket* LookupForReinsert(typename Policy::Bucket* table, typename Policy::Key key) {
    const unsigned sizeMask = Metadata(table)[TableSizeMaskOffset];
    unsigned i = Policy::Hash(key) & sizeMask;
    unsigned probeCount = 0;
    while (!Policy::IsEmpty(table[i])) {
        ++probeCount;
        i = (i + probeCount) & sizeMask;
    }
    return table + i;
}

template<typename Policy>
typename Policy::Bucket* Rehash(typename Policy::Bucket*& tableRef, unsigned newTableSize, typename Policy::Bucket* entry) {
    using Bucket = typename Policy::Bucket;
    Bucket* oldTable = tableRef;
    const unsigned oldTableSize = TableSize(oldTable);
    Bucket* table = AllocateTable<Policy>(newTableSize);
    Metadata(table)[KeyCountOffset] = KeyCount(oldTable);
    Bucket* newEntry = nullptr;
    for (unsigned i = 0; i < oldTableSize; ++i) {
        Bucket& oldEntry = oldTable[i];
        if (!IsLive<Policy>(oldEntry)) continue;
        Bucket* reinserted = LookupForReinsert<Policy>(table, Policy::KeyOf(oldEntry));
        Policy::Move(oldEntry, *reinserted);
        if (&oldEntry == entry) newEntry = reinserted;
    }
    tableRef = table;
    DeallocateTable<Policy>(oldTable);
    return newEntry;
}

template<typename Policy>
typename Policy::Bucket* Expand(typename Policy::Bucket*& tableRef, typename Policy::Bucket* entry) {
    const unsigned oldSize = TableSize(tableRef);
    unsigned newSize;
    if (!oldSize)
        newSize = MinimumTableSize;
    else if (KeyCount(tableRef) * MinLoad < oldSize * 2U)
        newSize = oldSize;
    else {
        if (oldSize > 0x40000000U) throw std::length_error("WTF::HashTable: table cannot grow further");
        newSize = oldSize * 2U;
    }
    return Rehash<Policy>(tableRef, newSize, entry);
}

template<typename Policy>
typename Policy::Bucket* Find(typename Policy::Bucket* table, typename Policy::Key key) {
    if (!table) return nullptr;
    const unsigned sizeMask = Metadata(table)[TableSizeMaskOffset];
    unsigned i = Policy::Hash(key) & sizeMask;
    unsigned probeCount = 0;
    while (true) {
        auto& entry = table[i];
        if (Policy::IsEmpty(entry)) return nullptr;
        if (!Policy::IsDeleted(entry) && Policy::KeyOf(entry) == key) return &entry;
        ++probeCount;
        i = (i + probeCount) & sizeMask;
    }
}

template<typename Policy, typename Fill>
bool Add(typename Policy::Bucket*& tableRef, typename Policy::Key key, Fill fill) {
    using Bucket = typename Policy::Bucket;
    if (!tableRef) Expand<Policy>(tableRef, nullptr);
    Bucket* table = tableRef;
    const unsigned sizeMask = Metadata(table)[TableSizeMaskOffset];
    unsigned i = Policy::Hash(key) & sizeMask;
    unsigned probeCount = 0;
    Bucket* deletedEntry = nullptr;
    Bucket* entry;
    while (true) {
        entry = table + i;
        if (Policy::IsEmpty(*entry)) break;
        if (Policy::IsDeleted(*entry))
            deletedEntry = entry;
        else if (Policy::KeyOf(*entry) == key)
            return false;
        ++probeCount;
        i = (i + probeCount) & sizeMask;
    }
    if (deletedEntry) {
        Policy::ConstructEmpty(*deletedEntry);
        entry = deletedEntry;
        Metadata(table)[DeletedCountOffset] -= 1U;
    }
    fill(*entry);
    Metadata(table)[KeyCountOffset] += 1U;
    if (ShouldExpand(std::uint64_t(Metadata(table)[KeyCountOffset]) + Metadata(table)[DeletedCountOffset], TableSize(table)))
        Expand<Policy>(tableRef, entry);
    return true;
}

template<typename Policy>
void Remove(typename Policy::Bucket*& tableRef, typename Policy::Bucket* entry) {
    if (!entry) return;
    Policy::DestroyValue(*entry);
    Policy::SetDeleted(*entry);
    unsigned* metadata = Metadata(tableRef);
    metadata[DeletedCountOffset] += 1U;
    metadata[KeyCountOffset] -= 1U;
    const unsigned tableSize = metadata[TableSizeOffset];
    if (metadata[KeyCountOffset] * MinLoad < tableSize && tableSize > MinimumTableSize) Rehash<Policy>(tableRef, tableSize / 2U, nullptr);
}

template<typename Policy, typename Copy>
typename Policy::Bucket* CopyTable(typename Policy::Bucket* other, Copy copy) {
    const unsigned otherKeyCount = KeyCount(other);
    if (!otherKeyCount) return nullptr;
    auto* table = AllocateTable<Policy>(StringHashMapBestTableSize(otherKeyCount));
    Metadata(table)[KeyCountOffset] = otherKeyCount;
    const unsigned otherSize = TableSize(other);
    for (unsigned i = 0; i < otherSize; ++i) {
        if (!IsLive<Policy>(other[i])) continue;
        copy(other[i], *LookupForReinsert<Policy>(table, Policy::KeyOf(other[i])));
    }
    return table;
}

template<typename Policy, typename Function>
void ForEachLive(typename Policy::Bucket* table, Function function) {
    const unsigned tableSize = TableSize(table);
    for (unsigned i = 0; i < tableSize; ++i)
        if (IsLive<Policy>(table[i])) function(table[i]);
}

class LockHolder {
public:
    explicit LockHolder(std::uint8_t& byte) : lock(byte) {
        for (std::uint8_t expected = 0; !lock.compare_exchange_weak(expected, 1, std::memory_order_acquire); expected = 0)
            std::this_thread::yield();
    }
    ~LockHolder() { lock.store(0, std::memory_order_release); }
    LockHolder(const LockHolder&) = delete;
    LockHolder& operator=(const LockHolder&) = delete;

private:
    std::atomic_ref<std::uint8_t> lock;
};

}  // namespace NKWebKit::WtfAbi::HashTableAbi
