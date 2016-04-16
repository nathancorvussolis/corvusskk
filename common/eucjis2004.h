
#ifndef EUCJIS2004_H
#define EUCJIS2004_H

// Unicode Code PointをUTF-16へ変換
//   戻り値
//      0 : ucpがU+10FFFFより大きい
//          firstまたはsecondがnullptr
//      1 : BMP内
//      2 : BMP外
//   引数
//      ucp     : Unicode符号位置
//      first   : BMPの文字または上位サロゲート
//      second  : 下位サロゲート
size_t UcpToWideChar(UCSCHAR ucp, PWCHAR first, PWCHAR second);

// EUC 1文字分をUnicode Code Pointへ変換
//   戻り値
//      変換に使用されたサイズ(char単位)
//   引数
//      src     : 変換元のEUC-JIS-2004文字列
//      srcsize : 変換元のEUC-JIS-2004文字列のサイズ
//      ucp1    : Unicode符号位置1つ目
//      ucp2    : Unicode符号位置2つ目(結合文字がある場合、なければ0)
size_t EucJis2004ToUcp(LPCSTR src, size_t srcsize, PUCSCHAR ucp1, PUCSCHAR ucp2);

// EUC-JIS-2004をUTF-16へ変換
//   戻り値
//      TRUE  : 成功
//      FALSE : 失敗
//   引数
//      src     : 変換元のEUC-JIS-2004文字列
//                nullptrのとき戻り値はFALSEになる
//      srcsize : nullptrのときsrcの終端NULLまで変換する
//                nullptr以外のとき指定されたサイズまたはsrcの終端NULLの短いほうまで変換する
//                変換に使用されたサイズ(char単位)が戻る
//      dst     : 変換先のUTF-16LE文字列バッファ
//                nullptrは許容される
//      dstsize : dstのサイズ
//                変換結果の終端NULLを含むサイズ(wchar_t単位)が戻る
//                nullptrのとき戻り値はFALSEになる
BOOL EucJis2004ToWideChar(LPCSTR src, size_t *srcsize, LPWSTR dst, size_t *dstsize);

// UTF-16をEUC-JIS-2004へ変換
//   戻り値
//      TRUE  : 成功
//      FALSE : 失敗
//   引数
//      src     : 変換元のUTF-16LE文字列
//                nullptrのとき戻り値はFALSEになる
//      srcsize : nullptrのときsrcの終端NULLまで変換する
//                nullptr以外のとき指定されたサイズまたはsrcの終端NULLの短いほうまで変換する
//                変換に使用されたサイズ(wchar_t単位)が戻る
//      dst     : 変換先のEUC-JIS-2004文字列バッファ
//                nullptrは許容される
//      dstsize : dstのサイズ
//                変換結果の終端NULLを含むサイズ(char単位)が戻る
//                nullptrのとき戻り値はFALSEになる
BOOL WideCharToEucJis2004(LPCWSTR src, size_t *srcsize, LPSTR dst, size_t *dstsize);

std::string wstring_to_eucjis2004_string(const std::wstring &s);
std::wstring eucjis2004_string_to_wstring(const std::string &s);

#define WCTOEUC(w) wstring_to_eucjis2004_string(w).c_str()
#define EUCTOWC(u) eucjis2004_string_to_wstring(u).c_str()

#endif //EUCJIS2004_H
