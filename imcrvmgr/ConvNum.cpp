
#include "imcrvmgr.h"

static std::wstring num_to_kanji(std::wstring num, LPCWSTR kannum[], LPCWSTR kancl1[], LPCWSTR kancl2[])
{
	size_t i, m, n, p, q, r;
	std::vector< std::wstring > sepnum;
	bool kancl2flg;
	std::wstring ret;

	r = num.size();
	if((r % 4) != 0)
	{
		// 1234567890 -> "12" 34567890
		sepnum.push_back(num.substr(0, (r % 4)));
	}
	m = ((r - (r % 4)) / 4);
	for(i = 0; i < m; i++)
	{
		// 1234567890 -> 12 "3456" "7890"
		sepnum.push_back(num.substr(((r % 4) + (i * 4)), 4));
	}
	m = sepnum.size();
	for(p = 0; p < m; p++)
	{
		kancl2flg = false;
		n = sepnum[p].size();
		for(q = 0; q < n; q++)
		{
			if(sepnum[p][q] != L'0' || (m == 1 && n == 1))
			{
				//十の位と百の位の「一」は表記しない。
				if((sepnum[p][q] != L'1') ||	//二～九
					((n == 4) && (q == 0)) ||	//千の位
					(q == n - 1))  				//一の位
				{
					ret.append(kannum[sepnum[p][q] - L'0']);
				}
				ret.append(kancl1[(n - q - 1) % 4]);
				kancl2flg = true;
			}
		}
		if(kancl2flg)
		{
			ret.append(kancl2[m - p - 1]);
		}
	}
	return ret;
}

std::wstring ConvNum(const std::wstring &key, const std::wstring &candidate)
{
	LPCWSTR kannum3[] = {L"〇", L"一", L"二", L"三", L"四", L"五", L"六", L"七", L"八", L"九"};
	LPCWSTR kancl31[] = {L"",   L"十", L"百", L"千"};
	LPCWSTR kancl32[] = {L"",   L"万", L"億", L"兆", L"京"};
	LPCWSTR kannum5[] = {L"〇", L"壱", L"弐", L"参", L"四", L"五", L"六", L"七", L"八", L"九"};
	LPCWSTR kancl51[] = {L"", L"拾", L"百", L"千"};
	LPCWSTR kancl52[] = {L"", L"万", L"億", L"兆", L"京"};
	LPCWSTR sep8 = L",";
	const int sepnum8 = 3;
	size_t i, j, k, r;
	std::wregex rxnum(L"[0-9]+");
	std::wregex rxint(L"#[0-9]");
	std::wstring searchkey_tmp = key;
	std::wstring candidate_tmp = candidate;
	std::wstring repcandidate;
	std::wsmatch result;
	std::vector< std::wstring > keynum;

	while(std::regex_search(searchkey_tmp, result, rxnum))
	{
		keynum.push_back(result.str());
		searchkey_tmp = result.suffix();
	}

	j = 0;
	while(std::regex_search(candidate_tmp, result, rxint) && (j < keynum.size()))
	{
		repcandidate.append(result.prefix());
		switch(result.str().back())
		{
		case L'0':	//5500
			repcandidate.append(keynum[j]);
			break;
		case L'1':	//５５００
			for(k = 0; k < keynum[j].size(); k++)
			{
				repcandidate.push_back((L'０' - L'0')/*(0xFF10-0x0030)*/ + keynum[j][k]);
			}
			break;
		case L'2':	//五五〇〇
			for(k = 0; k<keynum[j].size(); k++)
			{
				repcandidate.append(kannum3[(keynum[j][k] - L'0'/*0x0030*/)]);
			}
			break;
		case L'3':	//五千五百
			if(keynum[j].size() > 20)	// 10^20 < X
			{
				repcandidate.append(keynum[j]);
			}
			else
			{
				repcandidate.append(num_to_kanji(keynum[j], kannum3, kancl31, kancl32));
			}
			break;
		case L'4':
			repcandidate.append(result.str());
			break;
		case L'5':
			if(keynum[j].size() > 20)	// 10^20 < X
			{
				repcandidate.append(keynum[j]);
			}
			else
			{
				repcandidate.append(num_to_kanji(keynum[j], kannum5, kancl51, kancl52));
			}
			break;
		case L'6':
			repcandidate.append(result.str());
			break;
		case L'7':
			repcandidate.append(result.str());
			break;
		case L'8':
			r = keynum[j].size();
			if (r % sepnum8 != 0)
			{
				repcandidate.append(keynum[j].substr(0, r % sepnum8) + sep8);
			}
			for(i = 0; i < (r - r % sepnum8) / sepnum8; i++)
			{
				repcandidate.append(keynum[j].substr(r % sepnum8 + i * sepnum8, sepnum8) + sep8);
			}
			repcandidate = repcandidate.substr(0, repcandidate.size() - wcslen(sep8));
			break;
		case L'9':	//３四
			r = keynum[j].size();
			if(r != 2)
			{
				repcandidate.append(result.str());
				break;
			}
			repcandidate.push_back((L'０'-L'0')/*(0xFF10-0x0030)*/ + keynum[j][0]);
			repcandidate.append(kannum3[(keynum[j][1] - L'0'/*0x0030*/)]);
			break;
		default:
			repcandidate.append(keynum[j]);
			break;
		}
		candidate_tmp = result.suffix();
		++j;
	}
	repcandidate.append(candidate_tmp);
	return repcandidate;
}
