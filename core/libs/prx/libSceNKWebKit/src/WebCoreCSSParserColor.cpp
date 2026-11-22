// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/css/parser/CSSParserFastPaths.cpp@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
// Derived from neonmodder123/PS5-WebKit-11.x/Source/WebCore/platform/ColorData.gperf@80f77e94c63ee7202eec3d4b5b1a021dd89011ba
// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016-2022 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "prx/libc/include/General.hpp"
#include "prx/libSceNKWebKit/include/WebCoreExports.hpp"
#include "prx/libSceNKWebKit/src/WebCoreTypes.hpp"
#include "prx/libSceNKWebKit/src/WtfStringAbi.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <optional>
#include <string_view>

namespace WebCore {

namespace {

constexpr std::uint64_t kFlagsShift = 48;
constexpr std::uint64_t kColorSpaceShift = 56;
constexpr std::uint64_t kValidFlag = 1ULL << (kFlagsShift + 2);
constexpr std::uint64_t kColorSpaceSRGB = 17ULL << kColorSpaceShift;

enum class Unit { Unknown, Number, Percentage };

struct NamedColor {
    std::string_view name;
    std::uint32_t argb;
};

constexpr std::array<NamedColor, 152> kNamedColors{{
    {"aliceblue", 0xfff0f8ff},
    {"alpha", 0x00000000},
    {"antiquewhite", 0xfffaebd7},
    {"aqua", 0xff00ffff},
    {"aquamarine", 0xff7fffd4},
    {"azure", 0xfff0ffff},
    {"beige", 0xfff5f5dc},
    {"bisque", 0xffffe4c4},
    {"black", 0xff000000},
    {"blanchedalmond", 0xffffebcd},
    {"blue", 0xff0000ff},
    {"blueviolet", 0xff8a2be2},
    {"brown", 0xffa52a2a},
    {"burlywood", 0xffdeb887},
    {"cadetblue", 0xff5f9ea0},
    {"chartreuse", 0xff7fff00},
    {"chocolate", 0xffd2691e},
    {"coral", 0xffff7f50},
    {"cornflowerblue", 0xff6495ed},
    {"cornsilk", 0xfffff8dc},
    {"crimson", 0xffdc143c},
    {"cyan", 0xff00ffff},
    {"darkblue", 0xff00008b},
    {"darkcyan", 0xff008b8b},
    {"darkgoldenrod", 0xffb8860b},
    {"darkgray", 0xffa9a9a9},
    {"darkgreen", 0xff006400},
    {"darkgrey", 0xffa9a9a9},
    {"darkkhaki", 0xffbdb76b},
    {"darkmagenta", 0xff8b008b},
    {"darkolivegreen", 0xff556b2f},
    {"darkorange", 0xffff8c00},
    {"darkorchid", 0xff9932cc},
    {"darkred", 0xff8b0000},
    {"darksalmon", 0xffe9967a},
    {"darkseagreen", 0xff8fbc8f},
    {"darkslateblue", 0xff483d8b},
    {"darkslategray", 0xff2f4f4f},
    {"darkslategrey", 0xff2f4f4f},
    {"darkturquoise", 0xff00ced1},
    {"darkviolet", 0xff9400d3},
    {"deeppink", 0xffff1493},
    {"deepskyblue", 0xff00bfff},
    {"dimgray", 0xff696969},
    {"dimgrey", 0xff696969},
    {"dodgerblue", 0xff1e90ff},
    {"firebrick", 0xffb22222},
    {"floralwhite", 0xfffffaf0},
    {"forestgreen", 0xff228b22},
    {"fuchsia", 0xffff00ff},
    {"gainsboro", 0xffdcdcdc},
    {"ghostwhite", 0xfff8f8ff},
    {"gold", 0xffffd700},
    {"goldenrod", 0xffdaa520},
    {"gray", 0xff808080},
    {"green", 0xff008000},
    {"greenyellow", 0xffadff2f},
    {"grey", 0xff808080},
    {"honeydew", 0xfff0fff0},
    {"hotpink", 0xffff69b4},
    {"indianred", 0xffcd5c5c},
    {"indigo", 0xff4b0082},
    {"ivory", 0xfffffff0},
    {"khaki", 0xfff0e68c},
    {"lavender", 0xffe6e6fa},
    {"lavenderblush", 0xfffff0f5},
    {"lawngreen", 0xff7cfc00},
    {"lemonchiffon", 0xfffffacd},
    {"lightblue", 0xffadd8e6},
    {"lightcoral", 0xfff08080},
    {"lightcyan", 0xffe0ffff},
    {"lightgoldenrodyellow", 0xfffafad2},
    {"lightgray", 0xffd3d3d3},
    {"lightgreen", 0xff90ee90},
    {"lightgrey", 0xffd3d3d3},
    {"lightpink", 0xffffb6c1},
    {"lightsalmon", 0xffffa07a},
    {"lightseagreen", 0xff20b2aa},
    {"lightskyblue", 0xff87cefa},
    {"lightslateblue", 0xff8470ff},
    {"lightslategray", 0xff778899},
    {"lightslategrey", 0xff778899},
    {"lightsteelblue", 0xffb0c4de},
    {"lightyellow", 0xffffffe0},
    {"lime", 0xff00ff00},
    {"limegreen", 0xff32cd32},
    {"linen", 0xfffaf0e6},
    {"magenta", 0xffff00ff},
    {"maroon", 0xff800000},
    {"mediumaquamarine", 0xff66cdaa},
    {"mediumblue", 0xff0000cd},
    {"mediumorchid", 0xffba55d3},
    {"mediumpurple", 0xff9370db},
    {"mediumseagreen", 0xff3cb371},
    {"mediumslateblue", 0xff7b68ee},
    {"mediumspringgreen", 0xff00fa9a},
    {"mediumturquoise", 0xff48d1cc},
    {"mediumvioletred", 0xffc71585},
    {"midnightblue", 0xff191970},
    {"mintcream", 0xfff5fffa},
    {"mistyrose", 0xffffe4e1},
    {"moccasin", 0xffffe4b5},
    {"navajowhite", 0xffffdead},
    {"navy", 0xff000080},
    {"oldlace", 0xfffdf5e6},
    {"olive", 0xff808000},
    {"olivedrab", 0xff6b8e23},
    {"orange", 0xffffa500},
    {"orangered", 0xffff4500},
    {"orchid", 0xffda70d6},
    {"palegoldenrod", 0xffeee8aa},
    {"palegreen", 0xff98fb98},
    {"paleturquoise", 0xffafeeee},
    {"palevioletred", 0xffdb7093},
    {"papayawhip", 0xffffefd5},
    {"peachpuff", 0xffffdab9},
    {"peru", 0xffcd853f},
    {"pink", 0xffffc0cb},
    {"plum", 0xffdda0dd},
    {"powderblue", 0xffb0e0e6},
    {"purple", 0xff800080},
    {"rebeccapurple", 0xff663399},
    {"red", 0xffff0000},
    {"rosybrown", 0xffbc8f8f},
    {"royalblue", 0xff4169e1},
    {"saddlebrown", 0xff8b4513},
    {"salmon", 0xfffa8072},
    {"sandybrown", 0xfff4a460},
    {"seagreen", 0xff2e8b57},
    {"seashell", 0xfffff5ee},
    {"sienna", 0xffa0522d},
    {"silver", 0xffc0c0c0},
    {"skyblue", 0xff87ceeb},
    {"slateblue", 0xff6a5acd},
    {"slategray", 0xff708090},
    {"slategrey", 0xff708090},
    {"snow", 0xfffffafa},
    {"springgreen", 0xff00ff7f},
    {"steelblue", 0xff4682b4},
    {"tan", 0xffd2b48c},
    {"teal", 0xff008080},
    {"thistle", 0xffd8bfd8},
    {"tomato", 0xffff6347},
    {"transparent", 0x00000000},
    {"turquoise", 0xff40e0d0},
    {"violet", 0xffee82ee},
    {"violetred", 0xffd02090},
    {"wheat", 0xfff5deb3},
    {"white", 0xffffffff},
    {"whitesmoke", 0xfff5f5f5},
    {"yellow", 0xffffff00},
    {"yellowgreen", 0xff9acd32},
}};

SRGBA<std::uint8_t> FromARGB(std::uint32_t value) {
    return {static_cast<std::uint8_t>(value >> 16), static_cast<std::uint8_t>(value >> 8), static_cast<std::uint8_t>(value), static_cast<std::uint8_t>(value >> 24)};
}

SRGBA<std::uint8_t> FromRGBA(std::uint32_t value) {
    return {static_cast<std::uint8_t>(value >> 24), static_cast<std::uint8_t>(value >> 16), static_cast<std::uint8_t>(value >> 8), static_cast<std::uint8_t>(value)};
}

Color MakeColor(SRGBA<std::uint8_t> color) {
    std::uint64_t packed = (static_cast<std::uint64_t>(color.red) << 24)
                         | (static_cast<std::uint64_t>(color.green) << 16)
                         | (static_cast<std::uint64_t>(color.blue) << 8)
                         | static_cast<std::uint64_t>(color.alpha);
    return Color{packed | kColorSpaceSRGB | kValidFlag};
}

std::uint8_t PrescaledToByte(float value) {
    return static_cast<std::uint8_t>(std::clamp(std::lround(value), 0l, 255l));
}

std::uint8_t AlphaToByte(float value) {
    return static_cast<std::uint8_t>(std::clamp(std::lround(value * 255.0f), 0l, 255l));
}

template<typename CharacterType> bool IsWhitespace(CharacterType c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\f' || c == '\r';
}

template<typename CharacterType> bool IsDigit(CharacterType c) {
    return c >= '0' && c <= '9';
}

template<typename CharacterType> std::optional<std::uint32_t> HexValue(CharacterType c) {
    if (IsDigit(c)) {
        return static_cast<std::uint32_t>(c - '0');
    }
    if (c >= 'a' && c <= 'f') {
        return static_cast<std::uint32_t>(c - 'a' + 10);
    }
    if (c >= 'A' && c <= 'F') {
        return static_cast<std::uint32_t>(c - 'A' + 10);
    }
    return std::nullopt;
}

template<typename CharacterType> bool CaselessEqual(CharacterType c, char lower) {
    return c == lower || c == lower - 'a' + 'A';
}

template<typename CharacterType> int CheckForValidDouble(const CharacterType* string, const CharacterType* end, char terminator) {
    int length = static_cast<int>(end - string);
    if (length < 1) {
        return 0;
    }
    bool decimalMarkSeen = false;
    int processedLength = 0;
    for (int i = 0; i < length; ++i) {
        if (string[i] == terminator) {
            processedLength = i;
            break;
        }
        if (!IsDigit(string[i])) {
            if (!decimalMarkSeen && string[i] == '.') {
                decimalMarkSeen = true;
            } else {
                return 0;
            }
        }
    }
    if (decimalMarkSeen && processedLength == 1) {
        return 0;
    }
    return processedLength;
}

template<typename CharacterType> int ParseDouble(const CharacterType* string, const CharacterType* end, char terminator, double& value) {
    int length = CheckForValidDouble(string, end, terminator);
    if (!length) {
        return 0;
    }
    int position = 0;
    double localValue = 0;
    for (; position < length; ++position) {
        if (string[position] == '.') {
            break;
        }
        localValue = localValue * 10 + (string[position] - '0');
    }
    if (++position == length) {
        value = localValue;
        return length;
    }
    double fraction = 0;
    double scale = 1;
    constexpr double maxScale = 1000000;
    while (position < length && scale < maxScale) {
        fraction = fraction * 10 + (string[position++] - '0');
        scale *= 10;
    }
    value = localValue + fraction / scale;
    return length;
}

template<typename CharacterType> std::optional<std::uint8_t> ParseColorIntOrPercentage(const CharacterType*& string, const CharacterType* end, char terminator, Unit& expect) {
    const CharacterType* current = string;
    double localValue = 0;
    bool negative = false;
    while (current != end && IsWhitespace(*current)) {
        ++current;
    }
    if (current != end && *current == '-') {
        negative = true;
        ++current;
    }
    if (current == end || !IsDigit(*current)) {
        return std::nullopt;
    }
    while (current != end && IsDigit(*current)) {
        double newValue = localValue * 10 + (*current++ - '0');
        if (newValue >= 255) {
            localValue = 255;
            while (current != end && IsDigit(*current)) {
                ++current;
            }
            break;
        }
        localValue = newValue;
    }
    if (current == end) {
        return std::nullopt;
    }
    if (expect == Unit::Number && (*current == '.' || *current == '%')) {
        return std::nullopt;
    }
    if (*current == '.') {
        double percentage = 0;
        int numCharactersParsed = ParseDouble(current, end, '%', percentage);
        if (!numCharactersParsed) {
            return std::nullopt;
        }
        current += numCharactersParsed;
        if (*current != '%') {
            return std::nullopt;
        }
        localValue += percentage;
    }
    if (expect == Unit::Percentage && *current != '%') {
        return std::nullopt;
    }
    if (*current == '%') {
        expect = Unit::Percentage;
        localValue = localValue / 100.0 * 255.0;
        if (localValue > 255) {
            localValue = 255;
        }
        ++current;
    } else {
        expect = Unit::Number;
    }
    while (current != end && IsWhitespace(*current)) {
        ++current;
    }
    if (current == end || *current++ != terminator) {
        return std::nullopt;
    }
    string = current;
    return negative ? 0 : PrescaledToByte(static_cast<float>(localValue));
}

template<typename CharacterType> bool IsTenthAlpha(const CharacterType* string, int length) {
    if (length == 3 && string[0] == '0' && string[1] == '.' && IsDigit(string[2])) {
        return true;
    }
    return length == 2 && string[0] == '.' && IsDigit(string[1]);
}

template<typename CharacterType> std::optional<std::uint8_t> ParseAlphaValue(const CharacterType*& string, const CharacterType* end, char terminator) {
    while (string != end && IsWhitespace(*string)) {
        ++string;
    }
    bool negative = false;
    if (string != end && *string == '-') {
        negative = true;
        ++string;
    }
    int length = static_cast<int>(end - string);
    if (length < 2) {
        return std::nullopt;
    }
    if (string[length - 1] != terminator || !IsDigit(string[length - 2])) {
        return std::nullopt;
    }
    if (string[0] != '0' && string[0] != '1' && string[0] != '.') {
        if (CheckForValidDouble(string, end, terminator)) {
            string = end;
            return negative ? 0 : 255;
        }
        return std::nullopt;
    }
    if (length == 2 && string[0] != '.') {
        std::uint8_t result = !negative && string[0] == '1' ? 255 : 0;
        string = end;
        return result;
    }
    if (IsTenthAlpha(string, length - 1)) {
        static constexpr std::uint8_t tenthAlphaValues[] = {0, 26, 51, 77, 102, 128, 153, 179, 204, 230};
        std::uint8_t result = negative ? 0 : tenthAlphaValues[string[length - 2] - '0'];
        string = end;
        return result;
    }
    double alpha = 0;
    if (!ParseDouble(string, end, terminator, alpha)) {
        return std::nullopt;
    }
    string = end;
    return negative ? 0 : AlphaToByte(static_cast<float>(alpha));
}

template<typename CharacterType> bool MightBeFunction(const CharacterType* characters, unsigned length, std::string_view name) {
    if (length < name.size() + 1 || characters[name.size()] != '(') {
        return false;
    }
    for (std::size_t i = 0; i < name.size(); ++i) {
        if (!CaselessEqual(characters[i], name[i])) {
            return false;
        }
    }
    return true;
}

std::optional<SRGBA<std::uint8_t>> FinishParsingHexColor(std::uint32_t value, unsigned length) {
    switch (length) {
    case 3:
        return FromARGB(0xFF000000 | (value & 0xF00) << 12 | (value & 0xF00) << 8 | (value & 0xF0) << 8 | (value & 0xF0) << 4 | (value & 0xF) << 4 | (value & 0xF));
    case 4:
        return FromARGB((value & 0xF) << 28 | (value & 0xF) << 24 | (value & 0xF000) << 8 | (value & 0xF000) << 4 | (value & 0xF00) << 4 | (value & 0xF00) | (value & 0xF0) | (value & 0xF0) >> 4);
    case 6:
        return FromARGB(0xFF000000 | value);
    case 8:
        return FromRGBA(value);
    }
    return std::nullopt;
}

template<typename CharacterType> std::optional<SRGBA<std::uint8_t>> ParseHexColor(const CharacterType* characters, unsigned length) {
    if (length != 3 && length != 4 && length != 6 && length != 8) {
        return std::nullopt;
    }
    std::uint32_t value = 0;
    for (unsigned i = 0; i < length; ++i) {
        auto digit = HexValue(characters[i]);
        if (!digit) {
            return std::nullopt;
        }
        value = (value << 4) | *digit;
    }
    return FinishParsingHexColor(value, length);
}

template<typename CharacterType> std::optional<SRGBA<std::uint8_t>> ParseNumericColor(const CharacterType* characters, unsigned length, bool strict) {
    if (length >= 4 && characters[0] == '#') {
        if (auto hexColor = ParseHexColor(characters + 1, length - 1)) {
            return hexColor;
        }
    }
    if (!strict && (length == 3 || length == 6)) {
        if (auto hexColor = ParseHexColor(characters, length)) {
            return hexColor;
        }
    }
    Unit expect = Unit::Unknown;
    const CharacterType* end = characters + length;
    if (MightBeFunction(characters, length, "rgba")) {
        const CharacterType* current = characters + 5;
        auto red = ParseColorIntOrPercentage(current, end, ',', expect);
        if (!red) {
            return std::nullopt;
        }
        auto green = ParseColorIntOrPercentage(current, end, ',', expect);
        if (!green) {
            return std::nullopt;
        }
        auto blue = ParseColorIntOrPercentage(current, end, ',', expect);
        if (!blue) {
            return std::nullopt;
        }
        auto alpha = ParseAlphaValue(current, end, ')');
        if (!alpha || current != end) {
            return std::nullopt;
        }
        return SRGBA<std::uint8_t>{*red, *green, *blue, *alpha};
    }
    if (MightBeFunction(characters, length, "rgb")) {
        const CharacterType* current = characters + 4;
        auto red = ParseColorIntOrPercentage(current, end, ',', expect);
        if (!red) {
            return std::nullopt;
        }
        auto green = ParseColorIntOrPercentage(current, end, ',', expect);
        if (!green) {
            return std::nullopt;
        }
        auto blue = ParseColorIntOrPercentage(current, end, ')', expect);
        if (!blue || current != end) {
            return std::nullopt;
        }
        return SRGBA<std::uint8_t>{*red, *green, *blue, 255};
    }
    return std::nullopt;
}

template<typename CharacterType> std::optional<SRGBA<std::uint8_t>> ParseNamedColor(const CharacterType* characters, unsigned length) {
    char buffer[64];
    if (length > sizeof(buffer) - 1) {
        return std::nullopt;
    }
    for (unsigned i = 0; i < length; ++i) {
        auto character = characters[i];
        if (!character || character > 0x7F) {
            return std::nullopt;
        }
        buffer[i] = static_cast<char>(character >= 'A' && character <= 'Z' ? character - 'A' + 'a' : character);
    }
    std::string_view name(buffer, length);
    auto found = std::lower_bound(kNamedColors.begin(), kNamedColors.end(), name, [](const NamedColor& entry, std::string_view key) { return entry.name < key; });
    if (found == kNamedColors.end() || found->name != name) {
        return std::nullopt;
    }
    return FromARGB(found->argb);
}

template<typename CharacterType> std::optional<SRGBA<std::uint8_t>> ParseSimpleColor(const CharacterType* characters, unsigned length, bool strict) {
    if (auto color = ParseNumericColor(characters, length, strict)) {
        return color;
    }
    return ParseNamedColor(characters, length);
}

std::optional<SRGBA<std::uint8_t>> ParseSimpleColor(const NKWebKit::WtfAbi::StringView& view, bool strict) {
    if (view.is8Bit) {
        return ParseSimpleColor(static_cast<const std::uint8_t*>(view.data), view.length, strict);
    }
    return ParseSimpleColor(static_cast<const char16_t*>(view.data), view.length, strict);
}

}  // namespace
}  // namespace WebCore

extern "C" {
WebCore::Color* APS5_VABI _ZN7WebCore9CSSParser10parseColorERKN3WTF6StringEb(WebCore::Color* result, const NKWebKit::WtfAbi::String& string, bool strict) {
    auto view = NKWebKit::WtfAbi::ViewOf(string.impl);
    if (auto color = WebCore::ParseSimpleColor(view, strict)) {
        *result = WebCore::MakeColor(*color);
        return result;
    }
    if (!view.length) {
        *result = WebCore::Color{};
        return result;
    }
    NotImplemented_nid_no_patch("CSSParser::parseColor: colors outside the hex, rgb(), rgba() and named-color fast path need the full CSS property parser");
}
}
