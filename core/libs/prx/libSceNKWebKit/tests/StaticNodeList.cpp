#include "prx/libc/include/general/VabiMacros.hpp"
#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

static void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "static node list check failed at line %d\n", line);
        std::fflush(stderr);
        std::_Exit(1);
    }
}
#define Require(value) Check((value), __LINE__)

int main() {
    int first = 0;
    int second = 0;
    int third = 0;
    void* nodes[4] = {&first, &second, &third, &first};
    alignas(8) unsigned char list[40];
    std::memset(list, 0xA5, sizeof(list));
    void** buffer = nodes;
    std::uint32_t capacity = 4;
    std::uint32_t size = 3;
    std::memcpy(list + 24, &buffer, sizeof(buffer));
    std::memcpy(list + 32, &capacity, sizeof(capacity));
    std::memcpy(list + 36, &size, sizeof(size));

    Require(_ZNK7WebCore14StaticNodeList6lengthEv(list) == 3);
    Require(_ZNK7WebCore14StaticNodeList4itemEj(list, 0) == &first);
    Require(_ZNK7WebCore14StaticNodeList4itemEj(list, 1) == &second);
    Require(_ZNK7WebCore14StaticNodeList4itemEj(list, 2) == &third);
    Require(_ZNK7WebCore14StaticNodeList4itemEj(list, 3) == nullptr);
    Require(_ZNK7WebCore14StaticNodeList4itemEj(list, 0xFFFFFFFFu) == nullptr);

    size = 0;
    std::memcpy(list + 36, &size, sizeof(size));
    Require(_ZNK7WebCore14StaticNodeList4itemEj(list, 0) == nullptr);

    bool threw = false;
    try {
        _ZNK7WebCore14StaticNodeList4itemEj(nullptr, 0);
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    Require(threw);

    std::puts("libSceNKWebKit static node list tests passed");
    return 0;
}
