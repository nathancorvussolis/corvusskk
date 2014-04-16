
#ifndef PARSESKKDIC_H
#define PARSESKKDIC_H

typedef std::pair< std::wstring, std::wstring > SKKDICCANDIDATE;
typedef std::vector< SKKDICCANDIDATE > SKKDICCANDIDATES;

typedef std::pair< std::wstring, SKKDICCANDIDATES > SKKDICOKURIBLOCK;
typedef std::vector< SKKDICOKURIBLOCK > SKKDICOKURIBLOCKS;

extern LPCWSTR EntriesAri;
extern LPCWSTR EntriesNasi;

int ReadSKKDicLine(FILE *fp, WCHAR bom, int &okuri, std::wstring &key,
	SKKDICCANDIDATES &c, SKKDICOKURIBLOCKS &o);
void ParseSKKDicCandiate(const std::wstring &s, SKKDICCANDIDATES &c);
void ParseSKKDicOkuriBlock(const std::wstring &s, SKKDICOKURIBLOCKS &o);

#endif //PARSESKKDIC_H
