#pragma once

// EUC 1文字分をUnicode Code Pointへ変換
//   戻り値
//      変換に使用されたサイズ(char単位)
//   引数
//      src     : 変換元のEUC-JP文字列
//      srcsize : 変換元のEUC-JP文字列のサイズ
//      ucp1    : Unicode符号位置1つ目
//      ucp2    : Unicode符号位置2つ目(結合文字がある場合、なければ0)
size_t EucJPToUcp(LPCSTR src, size_t srcsize, PUCSCHAR ucp1, PUCSCHAR ucp2);

// EUC-JPをUTF-16へ変換
//   戻り値
//      TRUE  : 成功
//      FALSE : 失敗
//   引数
//      src     : 変換元のEUC-JP文字列
//                nullptrのとき戻り値はFALSEになる
//      srcsize : nullptrのときsrcの終端NULLまで変換する
//                nullptr以外のとき指定されたサイズまたはsrcの終端NULLの短いほうまで変換する
//                変換に使用されたサイズ(char単位)が戻る
//      dst     : 変換先のUTF-16LE文字列バッファ
//                nullptrは許容される
//      dstsize : dstのサイズ
//                変換結果の終端NULLを含むサイズ(wchar_t単位)が戻る
//                nullptrのとき戻り値はFALSEになる
BOOL EucJPToWideChar(LPCSTR src, size_t *srcsize, LPWSTR dst, size_t *dstsize);

// UTF-16をEUC-JPへ変換
//   戻り値
//      TRUE  : 成功
//      FALSE : 失敗
//   引数
//      src     : 変換元のUTF-16LE文字列
//                nullptrのとき戻り値はFALSEになる
//      srcsize : nullptrのときsrcの終端NULLまで変換する
//                nullptr以外のとき指定されたサイズまたはsrcの終端NULLの短いほうまで変換する
//                変換に使用されたサイズ(wchar_t単位)が戻る
//      dst     : 変換先のEUC-JP文字列バッファ
//                nullptrは許容される
//      dstsize : dstのサイズ
//                変換結果の終端NULLを含むサイズ(char単位)が戻る
//                nullptrのとき戻り値はFALSEになる
BOOL WideCharToEucJP(LPCWSTR src, size_t *srcsize, LPSTR dst, size_t *dstsize);

std::wstring eucjp_string_to_wstring(const std::string &s);
std::string wstring_to_eucjp_string(const std::wstring &s);
