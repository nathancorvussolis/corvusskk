
#include "imcrvmgr.h"

static std::wstring num_to_kanji(std::wstring num, LPCWSTR numtype[], LPCWSTR numtype_1k[], LPCWSTR numtype_10k[])
{
	size_t i, j, k;
	std::wstring ret;

	//0を付加して4の倍数の桁数にする
	k = (4 - num.size() % 4) % 4;
	for(i = 0; i < k; i++)
	{
		num = L"0" + num;
	}

	k = num.size() / 4;
	for(i = 0; i < k; i++)
	{
		for(j = 0; j < 4; j++)
		{
			if(num[i * 4 + j] != L'0')
			{
				//十の位と百の位の「一」は表記しない。
				if((num[i * 4 + j] != L'1') || (j == 0) || (j == 3))
				{
					ret.append(numtype[num[i * 4 + j] - L'0']);
				}
				ret.append(numtype_1k[4 - j - 1]);
			}
		}
		if(num.substr(i * 4, 4) != L"0000")
		{
			ret.append(numtype_10k[k - i - 1]);
		}
	}

	if(ret.empty())
	{
		ret = numtype[0];
	}

	return ret;
}

std::wstring ConvNum(const std::wstring &key, const std::wstring &candidate)
{
	LPCWSTR numtype3[] = {L"〇", L"一", L"二", L"三", L"四", L"五", L"六", L"七", L"八", L"九"};
	LPCWSTR numtype5[] = {L"〇", L"壱", L"弐", L"参", L"四", L"五", L"六", L"七", L"八", L"九"};
	LPCWSTR numtype3_1k[] = {L"", L"十", L"百", L"千"};
	LPCWSTR numtype5_1k[] = {L"", L"拾", L"百", L"千"};
	LPCWSTR numtype_10k[] = {L"", L"万", L"億", L"兆", L"京", L"垓",
		L"𥝱", L"穣", L"溝", L"澗", L"正", L"載", L"極", L"恒河沙", L"阿僧祇", L"那由他", L"不可思議"};
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
				repcandidate.append(numtype3[(keynum[j][k] - L'0'/*0x0030*/)]);
			}
			break;
		case L'3':	//五千五百
			if(keynum[j].size() > (_countof(numtype_10k) * 4))
			{
				repcandidate.append(keynum[j]);
			}
			else
			{
				repcandidate.append(num_to_kanji(keynum[j], numtype3, numtype3_1k, numtype_10k));
			}
			break;
		case L'4':
			repcandidate.append(result.str());
			break;
		case L'5':
			if(keynum[j].size() > (_countof(numtype_10k) * 4))
			{
				repcandidate.append(keynum[j]);
			}
			else
			{
				repcandidate.append(num_to_kanji(keynum[j], numtype5, numtype5_1k, numtype_10k));
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
			repcandidate.push_back((L'０' - L'0')/*(0xFF10-0x0030)*/ + keynum[j][0]);
			repcandidate.append(numtype3[(keynum[j][1] - L'0'/*0x0030*/)]);
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
