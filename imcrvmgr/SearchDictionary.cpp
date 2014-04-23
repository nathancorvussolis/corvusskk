
#include "configxml.h"
#include "parseskkdic.h"
#include "imcrvmgr.h"

//ユーザー辞書
SKKDIC userdic;
USEROKURI userokuri;
//補完あり
KEYORDER complements;
//補完なし
KEYORDER accompaniments;

void SearchDictionary(const std::wstring &searchkey, const std::wstring &okuri, SKKDICCANDIDATES &sc)
{
    std::wstring candidate;
	SKKDICCANDIDATES::iterator sc_itrf;
	SKKDICCANDIDATES::iterator sc_itrb;
	std::wregex re;
	std::wstring fmt;

	//ユーザー辞書
	candidate += SearchUserDic(searchkey, okuri);
	
	//SKK辞書
	candidate += SearchSKKDic(searchkey);
	
	//SKK辞書サーバー
	candidate += SearchSKKServer(searchkey);

	//Unicodeコードポイント
	candidate += SearchUnicode(searchkey);
	
	//JIS X 0213 面区点番号
	candidate += SearchJISX0213(searchkey);

	re.assign(L"/\n/");
	fmt.assign(L"/");
	candidate = std::regex_replace(candidate, re, fmt);

	re.assign(L"[\\x00-\\x19]");
	fmt.assign(L"");
	candidate = std::regex_replace(candidate, re, fmt);

	ParseSKKDicCandiate(candidate, sc);

	//重複候補を削除
	if(sc.size() > 1)
	{
		for(sc_itrf = sc.begin(); sc_itrf != sc.end(); sc_itrf++)
		{
			for(sc_itrb = sc_itrf + 1; sc_itrb != sc.end(); )
			{
				if(sc_itrf->first == sc_itrb->first)
				{
					sc_itrb = sc.erase(sc_itrb);
				}
				else
				{
					sc_itrb++;
				}
			}
		}
	}
}

std::wstring SearchSKKDic(const std::wstring &searchkey)
{
	std::wstring candidate;
	FILE *fpdic, *fpidx;
	std::wstring key;
	long pos, left, mid, right;
	int comp;
	WCHAR wbuf[DICBUFSIZE];
	WCHAR *p;

	_wfopen_s(&fpidx, pathskkidx, RB);
	if(fpidx == NULL)
	{
		return candidate;
	}
	_wfopen_s(&fpdic, pathskkdic, RB);
	if(fpdic == NULL)
	{
		fclose(fpidx);
		return candidate;
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
					candidate = p;
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

	return candidate;
}

std::wstring ConvertCandidate(const std::wstring &key, const std::wstring &candidate)
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
