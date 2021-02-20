#pragma once

//変換済み検索結果
typedef std::pair< std::wstring, std::wstring > CANDIDATEBASE; //候補、注釈
typedef std::pair< CANDIDATEBASE, CANDIDATEBASE > CANDIDATE; //表示用、辞書登録用
typedef std::vector< CANDIDATE > CANDIDATES;

//検索結果
typedef std::pair< std::wstring, std::wstring > SKKDICCANDIDATE; //候補、注釈
typedef std::vector< SKKDICCANDIDATE > SKKDICCANDIDATES;

//送りありエントリのブロック
typedef std::pair< std::wstring, SKKDICCANDIDATES > SKKDICOKURIBLOCK; //送り仮名、候補
typedef std::vector< SKKDICOKURIBLOCK > SKKDICOKURIBLOCKS;
struct OKURIBLOCKS { //avoid C4503
	SKKDICOKURIBLOCKS o;
};
typedef std::pair< std::wstring, OKURIBLOCKS > USEROKURIENTRY; //見出し語、送りブロック
typedef std::map< std::wstring, OKURIBLOCKS > USEROKURI;

//見出し語順序
typedef std::vector< std::wstring > KEYORDER;

//辞書
typedef std::pair< std::wstring, SKKDICCANDIDATES > SKKDICENTRY; //見出し語、候補
typedef std::map< std::wstring, SKKDICCANDIDATES > SKKDIC;

extern LPCWSTR EntriesAri;
extern LPCWSTR EntriesNasi;

int ReadSKKDicLine(FILE *fp, WCHAR bom, int &okuri, std::wstring &key,
	SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o);
void ParseSKKDicCandiate(const std::wstring &s, SKKDICCANDIDATES &c);
void ParseSKKDicOkuriBlock(const std::wstring &s, SKKDICOKURIBLOCKS &o);
std::wstring ParseConcat(const std::wstring &s);
std::wstring MakeConcat(const std::wstring &s);
std::wstring EscapeGadgetString(const std::wstring &s);
