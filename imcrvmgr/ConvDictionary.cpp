
#include "configxml.h"
#include "parseskkdic.h"
#include "imcrvmgr.h"

void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates);
std::wstring ConvMisc(const std::wstring &key, const std::wstring &candidate);

//ユーザー辞書
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
	std::wregex re(L"[\\x00-\\x19]");
	std::wstring fmt(L"");

	//ユーザー辞書
	ConvUserDic(searchkey, candidates);

	//SKK辞書
	ConvSKKDic(searchkey, candidates);

	//SKK辞書サーバー
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

void ConvSKKDic(const std::wstring &searchkey, CANDIDATES &candidates)
{
	FILE *fpdic, *fpidx;
	std::wstring key;
	long pos, left, mid, right;
	int comp;
	WCHAR wbuf[DICBUFSIZE];
	WCHAR *p;

	_wfopen_s(&fpidx, pathskkidx, RB);
	if(fpidx == NULL)
	{
		return;
	}
	_wfopen_s(&fpdic, pathskkdic, RB);
	if(fpdic == NULL)
	{
		fclose(fpidx);
		return;
	}

	key = searchkey + L"\x20";

	left = 0;
	fseek(fpidx, 0, SEEK_END);
	right = (ftell(fpidx) / sizeof(pos)) - 1;

	while(left <= right)
	{
		mid = (left + right) / 2;
		fseek(fpidx, mid * sizeof(pos), SEEK_SET);
		fread(&pos, sizeof(pos), 1, fpidx);

		fseek(fpdic, pos, SEEK_SET);
		memset(wbuf, 0, sizeof(wbuf));
		fgetws(wbuf, _countof(wbuf), fpdic);

		comp = wcsncmp(key.c_str(), wbuf, key.size());
		if(comp == 0)
		{
			if((p = wcschr(wbuf, L'\x20')) != NULL)
			{
				if((p = wcschr(p, L'/')) != NULL)
				{
					ParseSKKDicCandiate(p, candidates);
				}
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
	
	fclose(fpdic);
	fclose(fpidx);
}

void ConvCandidate(const std::wstring &searchkey, const std::wstring &candidate, std::wstring &conv)
{
	conv = ConvMisc(searchkey, candidate);
}

std::wstring ConvMisc(const std::wstring &key, const std::wstring &candidate)
{
	std::wstring ret;
	std::wstring candidate_tmp = candidate;
	std::wregex rxnum(L"[0-9]+");
	std::wregex rxint(L"#[0-9]");

	if(std::regex_search(key, rxnum) && std::regex_search(candidate_tmp, rxint))
	{
		ret = ConvNum(key, candidate_tmp);
		candidate_tmp = ret;
	}
	
	//concat関数で"/"関数が\057にエスケープされる為2回実行する
	for(int i = 0; i < 2; i++)
	{
		if(candidate_tmp.size() > 2 &&
			candidate_tmp[0] == L'(' && candidate_tmp[candidate_tmp.size() - 1] == L')')
		{
			ret = ConvGadget(key, candidate_tmp);
			candidate_tmp = ret;
		}
	}

	return ret;
}
