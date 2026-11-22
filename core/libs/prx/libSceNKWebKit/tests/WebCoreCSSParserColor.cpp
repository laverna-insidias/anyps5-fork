#include "prx/libSceNKWebKit/libSceNKWebKit.hpp"
#include "prx/libSceNKWebKit/src/WtfStringAbi.hpp"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

namespace {

using NKWebKit::WtfAbi::String;

void Check(bool value, int line) {
    if (!value) {
        std::fprintf(stderr, "CSSParser::parseColor check failed at line %d\n", line);
        std::abort();
    }
}

#define Require(value) Check((value), __LINE__)

String Make(const char* text) {
    return _ZN3WTF10StringImpl6createEPKhj(reinterpret_cast<const unsigned char*>(text), static_cast<unsigned>(std::strlen(text)));
}

String Make16(const char16_t* text, unsigned length) {
    return _ZN3WTF10StringImpl6createEPKDsj(text, length);
}

std::uint64_t Encoded(std::uint8_t red, std::uint8_t green, std::uint8_t blue, std::uint8_t alpha) {
    return (17ULL << 56) | (1ULL << 50) | (static_cast<std::uint64_t>(red) << 24) | (static_cast<std::uint64_t>(green) << 16) | (static_cast<std::uint64_t>(blue) << 8) | alpha;
}

std::uint64_t Parse(const String& string, bool strict) {
    WebCore::Color color{0xDEADBEEF};
    WebCore::Color* result = _ZN7WebCore9CSSParser10parseColorERKN3WTF6StringEb(&color, string, strict);
    Require(result == &color);
    return color.m_colorAndFlags;
}

std::uint64_t Parse(const char* text, bool strict = true) {
    return Parse(Make(text), strict);
}

bool NeedsFullParser(const char* text, bool strict = true) {
    try {
        Parse(text, strict);
    } catch (const std::runtime_error&) {
        return true;
    }
    return false;
}

void TestHex() {
    Require(Parse("#f00") == Encoded(255, 0, 0, 255));
    Require(Parse("#abcd") == Encoded(0xaa, 0xbb, 0xcc, 0xdd));
    Require(Parse("#FF8000") == Encoded(255, 128, 0, 255));
    Require(Parse("#11223344") == Encoded(0x11, 0x22, 0x33, 0x44));
    Require(Parse("ff8000", false) == Encoded(255, 128, 0, 255));
    Require(Parse("abc", false) == Encoded(0xaa, 0xbb, 0xcc, 255));
    Require(NeedsFullParser("ff8000"));
    Require(NeedsFullParser("#12345"));
    const char16_t wide[] = u"#0000ff";
    Require(Parse(Make16(wide, 7), true) == Encoded(0, 0, 255, 255));
}

void TestFunctions() {
    Require(Parse("RGB(10, 20, 30)") == Encoded(10, 20, 30, 255));
    Require(Parse("rgb(300,-5, 10 )") == Encoded(255, 0, 10, 255));
    Require(Parse("rgb(50%, 100%, 0%)") == Encoded(128, 255, 0, 255));
    Require(Parse("rgb(12.5%, 0%, 200%)") == Encoded(32, 0, 255, 255));
    Require(Parse("rgba(255, 0, 0, 0.5)") == Encoded(255, 0, 0, 128));
    Require(Parse("rgba(0,0,0,.25)") == Encoded(0, 0, 0, 64));
    Require(Parse("rgba(0,0,0,1)") == Encoded(0, 0, 0, 255));
    Require(Parse("rgba(0,0,0,7)") == Encoded(0, 0, 0, 255));
    Require(Parse("rgba(0,0,0,-0.4)") == Encoded(0, 0, 0, 0));
    Require(NeedsFullParser("rgb(10%, 20, 30)"));
    Require(NeedsFullParser("rgb(10 20 30)"));
    Require(NeedsFullParser("hsl(0, 100%, 50%)"));
}

void TestNames() {
    Require(Parse("Red") == Encoded(255, 0, 0, 255));
    Require(Parse("aliceblue") == Encoded(0xf0, 0xf8, 0xff, 255));
    Require(Parse("YELLOWGREEN") == Encoded(0x9a, 0xcd, 0x32, 255));
    Require(Parse("transparent") == Encoded(0, 0, 0, 0));
    Require(Parse("grey") == Encoded(0x80, 0x80, 0x80, 255));
    const char16_t wide[] = u"Navy";
    Require(Parse(Make16(wide, 4), true) == Encoded(0, 0, 0x80, 255));
    Require(NeedsFullParser(" red"));
    Require(NeedsFullParser("notacolor"));
}

void TestEmpty() {
    Require(Parse(String(), true) == 0);
    Require(Parse("") == 0);
}

}  // namespace

int main() {
    TestHex();
    TestFunctions();
    TestNames();
    TestEmpty();
    return 0;
}
