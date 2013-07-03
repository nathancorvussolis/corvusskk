
#ifndef PARSESKKDIC_H
#define PARSESKKDIC_H

typedef std::pair<std::wstring, std::wstring> SKKDICCANDIDATE;
typedef std::vector<SKKDICCANDIDATE> SKKDICCANDIDATES;

extern LPCWSTR EntriesAri;
extern LPCWSTR EntriesNasi;

int ReadSKKDicLine(FILE *fp, WCHAR bom, int &okuri, std::wstring &key, SKKDICCANDIDATES &d);
void ParseSKKDicCandiate(const std::wstring &s, SKKDICCANDIDATES &d);

#endif //PARSESKKDIC_H
