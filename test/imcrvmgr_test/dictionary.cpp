
#include "common.h"
#include "parseskkdic.h"
#include "eucjis2004.h"
#include "eucjp.h"
#include "dictionary.h"

WCHAR krnlobjsddl[MAX_SECURITYDESC];	//SDDL
WCHAR mgrpipename[MAX_PIPENAME];	//名前付きパイプ

HANDLE hPipe = INVALID_HANDLE_VALUE;

WCHAR pipebuf[PIPEBUFSIZE];

OKURI_BLOCK okuri_block[OKURI_BLOCK_NUM] = {
	{L'a', {L'あ'}}, {L'i', {L'い'}}, {L'u', {L'う'}}, {L'e', {L'え'}}, {L'o', {L'お'}},
	{L'k', {L'か', L'き', L'く', L'け', L'こ'}},
	{L's', {L'さ', L'し', L'す', L'せ', L'そ'}},
	{L't', {L'た', L'ち', L'つ', L'て', L'つ'}},
	{L'n', {L'な', L'に', L'ぬ', L'ね', L'の', L'ん'}},
	{L'h', {L'は', L'ひ', L'ふ', L'へ', L'ほ'}},
	{L'm', {L'ま', L'み', L'む', L'め', L'も', L'ん'}},
	{L'y', {L'や', L'ゆ', L'よ'}},
	{L'r', {L'ら', L'り', L'る', L'れ', L'ろ'}},
	{L'w', {L'わ', L'を'}},
	{L'g', {L'が', L'ぎ', L'ぐ', L'げ', L'ご'}},
	{L'z', {L'ざ', L'じ', L'ず', L'ぜ', L'ぞ'}},
	{L'd', {L'だ', L'ぢ', L'づ', L'で', L'ど'}},
	{L'b', {L'ば', L'び', L'ぶ', L'べ', L'ぼ'}},
	{L'p', {L'ぱ', L'ぴ', L'ぷ', L'ぺ', L'ぽ'}},
	{L'c', {L'ち', L'ち', L'ち', L'ち', L'ち'}},
};

void _CreateIpcName()
{
	ZeroMemory(krnlobjsddl, sizeof(krnlobjsddl));
	ZeroMemory(mgrpipename, sizeof(mgrpipename));

	LPWSTR pszUserSid = nullptr;

	if (GetUserSid(&pszUserSid))
	{
		// SDDL_ALL_APP_PACKAGES / SDDL_RESTRICTED_CODE / SDDL_LOCAL_SYSTEM / SDDL_BUILTIN_ADMINISTRATORS / User SID
		_snwprintf_s(krnlobjsddl, _TRUNCATE, L"D:%s(A;;GA;;;RC)(A;;GA;;;SY)(A;;GA;;;BA)(A;;GA;;;%s)",
			(IsWindowsVersion62OrLater() ? L"(A;;GA;;;AC)" : L""), pszUserSid);

		// (SDDL_MANDATORY_LABEL, SDDL_NO_WRITE_UP, SDDL_ML_LOW)
		wcsncat_s(krnlobjsddl, L"S:(ML;;NW;;;LW)", _TRUNCATE);

		LocalFree(pszUserSid);
	}

	LPWSTR pszUserUUID = nullptr;

	if (GetUserUUID(&pszUserUUID))
	{
		_snwprintf_s(mgrpipename, _TRUNCATE, L"%s%s", IMCRVMGRPIPE, pszUserUUID);

		LocalFree(pszUserUUID);
	}
}

BOOL _ConnectDic()
{
	DWORD dwMode;

	if (WaitNamedPipeW(mgrpipename, NMPWAIT_USE_DEFAULT_WAIT) == 0)
	{
		return FALSE;
	}

	hPipe = CreateFileW(mgrpipename, GENERIC_WRITE | GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
		nullptr, OPEN_EXISTING, SECURITY_SQOS_PRESENT | SECURITY_EFFECTIVE_ONLY | SECURITY_IDENTIFICATION, nullptr);
	if (hPipe == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	dwMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
	if (SetNamedPipeHandleState(hPipe, &dwMode, nullptr, nullptr) == FALSE)
	{
		_DisconnectDic();
		return FALSE;
	}

	return TRUE;
}

void _DisconnectDic()
{
	if (hPipe != INVALID_HANDLE_VALUE)
	{
		CloseHandle(hPipe);
		hPipe = INVALID_HANDLE_VALUE;
	}
}

BOOL _SearchDic(WCHAR command, CANDIDATES &candidates, const std::wstring &searchkey, const std::wstring &searchkey_org, const std::wstring &okurikey)
{
	BOOL ret = FALSE;

	DWORD bytesWrite, bytesRead;
	std::wstring s, se, fmt, scd, scr, sad, sar;
	std::wregex r;
	std::wsmatch m;

	candidates.clear();

	_ConnectDic();

	ZeroMemory(pipebuf, sizeof(pipebuf));

	_snwprintf_s(pipebuf, _TRUNCATE, L"%c\n%s\t%s\t%s\n",
		command, searchkey.c_str(), searchkey_org.c_str(), okurikey.c_str());

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(pipebuf, sizeof(pipebuf));

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

	if (pipebuf[0] != REP_OK)
	{
		ret = TRUE;
		goto exit;
	}

	s.assign(pipebuf);
	r.assign(L"(.*)\t(.*)\t(.*)\t(.*)\n");

	while (std::regex_search(s, m, r))
	{
		se = m.str();
		s = m.suffix().str();

		fmt.assign(L"$1");
		scd = std::regex_replace(se, r, fmt);
		fmt.assign(L"$2");
		scr = std::regex_replace(se, r, fmt);
		fmt.assign(L"$3");
		sad = std::regex_replace(se, r, fmt);
		fmt.assign(L"$4");
		sar = std::regex_replace(se, r, fmt);

		if (scd.empty())
		{
			scd = scr;
		}
		if (sad.empty())
		{
			sad = sar;
		}

		candidates.push_back(std::make_pair(
			std::make_pair(scd, sad), std::make_pair(scr, sar)));
	}

	ret = TRUE;

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();

	return ret;
}

BOOL _AddUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate, const std::wstring &annotation, const std::wstring &okurikey)
{
	BOOL ret = FALSE;

	DWORD bytesWrite, bytesRead;

	_ConnectDic();

	ZeroMemory(pipebuf, sizeof(pipebuf));

	_snwprintf_s(pipebuf, _TRUNCATE, L"%c\n%s\t%s\t%s\t%s\n",
		command, key.c_str(), candidate.c_str(), annotation.c_str(), okurikey.c_str());

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(pipebuf, sizeof(pipebuf));

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

	ret = TRUE;

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();

	return ret;
}

BOOL _DelUserDic(WCHAR command, const std::wstring &key, const std::wstring &candidate)
{
	BOOL ret = FALSE;

	DWORD bytesWrite, bytesRead;

	_ConnectDic();

	ZeroMemory(pipebuf, sizeof(pipebuf));

	_snwprintf_s(pipebuf, _TRUNCATE, L"%c\n%s\t%s\n",
		command, key.c_str(), candidate.c_str());

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	ZeroMemory(pipebuf, sizeof(pipebuf));

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

	ret = TRUE;

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();

	return ret;
}

BOOL _SaveUserDic()
{
	return _CommandDic(REQ_USER_SAVE);
}

BOOL _CommandDic(WCHAR command)
{
	BOOL ret = FALSE;

	DWORD bytesWrite, bytesRead;

	_ConnectDic();

	pipebuf[0] = command;
	pipebuf[1] = L'\n';
	pipebuf[2] = L'\0';

	bytesWrite = (DWORD)((wcslen(pipebuf) + 1) * sizeof(WCHAR));
	if (WriteFile(hPipe, pipebuf, bytesWrite, &bytesWrite, nullptr) == FALSE)
	{
		goto exit;
	}

	bytesRead = 0;
	if (ReadFile(hPipe, pipebuf, sizeof(pipebuf), &bytesRead, nullptr) == FALSE)
	{
		goto exit;
	}

	ret = TRUE;

exit:
	ZeroMemory(pipebuf, sizeof(pipebuf));

	_DisconnectDic();

	return ret;
}

HRESULT CheckMultiByteFile(LPCWSTR path, SKKDICENCODING encoding)
{
	HRESULT hr = S_OK;
	FILE *fp = nullptr;
	CHAR buf[READBUFSIZE];
	std::string strbuf;
	size_t len;

	_wfopen_s(&fp, path, modeRB);
	if (fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	while (true)
	{
		strbuf.clear();

		while (fgets(buf, _countof(buf), fp) != nullptr)
		{
			strbuf += buf;

			if (!strbuf.empty() && strbuf.back() == '\n')
			{
				break;
			}
		}

		if (ferror(fp) != 0)
		{
			hr = E_MAKESKKDIC_FILEIO;
			break;
		}

		if (strbuf.empty())
		{
			break;
		}

		switch (encoding)
		{
		case enc_euc_jis_2004: //EUC-JIS-2004
			if (EucJis2004ToWideChar(strbuf.c_str(), nullptr, nullptr, &len) == FALSE)
			{
				hr = S_FALSE;
			}
			break;
		case enc_euc_jp: //EUC-JP
			if (EucJPToWideChar(strbuf.c_str(), nullptr, nullptr, &len) == FALSE)
			{
				hr = S_FALSE;
			}
			break;
		case enc_utf_8: //UTF-8
			if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS,
				strbuf.c_str(), -1, nullptr, 0) == 0)
			{
				hr = S_FALSE;
			}
			break;
		default:
			hr = S_FALSE;
			break;
		}

		if (hr == S_FALSE)
		{
			break;
		}
	}

	fclose(fp);

	return hr;
}

HRESULT CheckWideCharFile(LPCWSTR path)
{
	HRESULT hr = S_OK;
	FILE *fp = nullptr;
	WCHAR wbuf[READBUFSIZE / sizeof(WCHAR)];
	std::wstring wstrbuf;

	_wfopen_s(&fp, path, modeRB);
	if (fp == nullptr)
	{
		return E_MAKESKKDIC_FILEIO;
	}

	while (true)
	{
		wstrbuf.clear();

		while (fgetws(wbuf, _countof(wbuf), fp) != nullptr)
		{
			wstrbuf += wbuf;

			if (!wstrbuf.empty() && wstrbuf.back() == L'\n')
			{
				break;
			}
		}

		if (ferror(fp) != 0)
		{
			hr = E_MAKESKKDIC_FILEIO;
			break;
		}

		if (wstrbuf.empty())
		{
			break;
		}

		if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS,
			wstrbuf.c_str(), -1, nullptr, 0, nullptr, nullptr) == 0)
		{
			hr = S_FALSE;
			break;
		}
	}

	fclose(fp);

	return hr;
}
