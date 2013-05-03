
#include "configxml.h"
#include "parseskkdic.h"
#include "imcrvmgr.h"

void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates);
std::wstring ConvMisc(const std::wstring &key, const std::wstring &candidate);

//ユーザ辞書
USERDIC userdic;
//補完あり
KEYORDER complements;
//補完なし
KEYORDER accompaniments;

void ConvDictionary(const std::wstring &searchkey, const std::wstring &searchkeyorg, SEARCHRESULTS &searchresults)
{
	CANDIDATES::iterator candidates_itrf;
	CANDIDATES::iterator candidates_itrb;
	CANDIDATES candidates;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");

	searchresults.clear();
	searchresults.shrink_to_fit();

	//ユーザ辞書
	ConvUserDic(searchkey, candidates);

	//SKK辞書
	ConvSKKDic(searchkey, candidates);

	//SKK辞書サーバ
	if(serv)
	{
		ConvSKKServer(searchkey, candidates);
	}

	//Unicodeコードポイント
	ConvUnicode(searchkey, candidates);

	//JIS X 0213 面区点番号
	ConvJISX0213(searchkey, candidates);

	//重複候補を削除
	if(candidates.size() > 1)
	{
		for(candidates_itrf = candidates.begin(); candidates_itrf != candidates.end(); candidates_itrf++)
		{
			for(candidates_itrb = candidates_itrf + 1; candidates_itrb != candidates.end(); )
			{
				if(candidates_itrf->first == candidates_itrb->first)
				{
					candidates_itrb = candidates.erase(candidates_itrb);
				}
				else
				{
					candidates_itrb++;
				}
			}
		}
	}

	//候補を変換、結果にセット
	for(candidates_itrf = candidates.begin(); candidates_itrf != candidates.end(); candidates_itrf++)
	{
		searchresults.push_back(SEARCHRESULT(CANDIDATEPAIR(
			std::regex_replace(ConvMisc(searchkeyorg, candidates_itrf->first), re, fmt),
			candidates_itrf->first), candidates_itrf->second));
	}
}

void ConvCandidate(const std::wstring &searchkey, const std::wstring &candidate, std::wstring &conv)
{
	conv = ConvMisc(searchkey, candidate);
}

void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates)
{
	FILE *fpidx;
	ULONGLONG pos;
	long left, mid, right;
	int comp;
	APPDATAXMLDIC xmldic;
	APPDATAXMLDIC::iterator d_itr;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW::iterator r_itr;
	std::wstring candidate;
	std::wstring annotation;
	std::wregex re(L"\\t|\\r|\\n");
	std::wstring fmt(L"");

	_wfopen_s(&fpidx, pathskkcvdicidx, L"rb");
	if(fpidx == NULL)
	{
		return;
	}

	left = 0;
	fseek(fpidx, 0, SEEK_END);
	right = (ftell(fpidx) / sizeof(pos)) - 1;

	while(left <= right)
	{
		mid = (left + right) / 2;
		fseek(fpidx, mid*sizeof(pos), SEEK_SET);
		fread(&pos, sizeof(pos), 1, fpidx);

		if(ReadDicList(pathskkcvdicxml, SectionDictionary, xmldic, pos) != S_OK)
		{
			break;
		}

		d_itr = xmldic.begin();
		if(d_itr != xmldic.end())
		{
			comp = searchkey.compare(d_itr->first);
		}
		else
		{
			break;
		}

		if(comp == 0)
		{
			for(l_itr = d_itr->second.begin(); l_itr != d_itr->second.end(); l_itr++)
			{
				candidate.clear();
				annotation.clear();

				for(r_itr = l_itr->begin(); r_itr != l_itr->end(); r_itr++)
				{
					if(r_itr->first == AttributeCandidate)
					{
						candidate = std::regex_replace(r_itr->second, re, fmt);
					}
					else if(r_itr->first == AttributeAnnotation)
					{
						annotation = std::regex_replace(r_itr->second, re, fmt);
					}
				}

				if(candidate.empty())
				{
					continue;
				}

				candidates.push_back(CANDIDATE(candidate, annotation));
			}
			break;
		}
		else if(comp > 0)
		{
			left = mid + 1;
		}
		else
		{
			right = mid - 1;
		}
	}

	fclose(fpidx);
}

std::wstring ConvMisc(const std::wstring &key, const std::wstring &candidate)
{
	std::wstring ret;
	std::wregex rxnum(L"[0-9]+");
	std::wregex rxint(L"#[0-9]");

	if(std::regex_search(key, rxnum) && std::regex_search(candidate, rxint))
	{
		ret = ConvNum(key, candidate);
	}
	else if(candidate.size() > 2 && candidate[0] == L'(' && candidate[candidate.size() - 1] == L')')
	{
		ret = ConvGaget(key, candidate);
	}

	return ret;
}
