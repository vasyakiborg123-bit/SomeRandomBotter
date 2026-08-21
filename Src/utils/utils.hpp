#pragma once

#include <string>
#include <codecvt>
#include <locale>

namespace Utils {

    inline std::wstring widen(std::string const& str) {
        try {
            std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> converter;
            return converter.from_bytes(str);
        } catch(...) {
            return L"Widen Error";
        }
    }

}
