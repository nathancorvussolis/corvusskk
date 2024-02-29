#pragma once

std::string wstring_to_utf8_string(const std::wstring &s);
std::wstring utf8_string_to_wstring(const std::string &s);

#define WCTOU8(w) wstring_to_utf8_string(w).c_str()
#define U8TOWC(u) utf8_string_to_wstring(u).c_str()

#define TOWELLFORMED(w) U8TOWC(WCTOU8(w))
