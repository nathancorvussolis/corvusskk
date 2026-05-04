
#include "imcrvcnf.h"
#include "utf8.h"
#include "resource.h"

#define WM_USER_FETCH_DONE	(WM_USER + 1)

#define RELEASES_PAGE_URL	L"https://github.com/nathancorvussolis/corvusskk/releases"
#define LATEST_API_URL		L"https://api.github.com/repos/nathancorvussolis/corvusskk/releases/latest"

struct ReleaseInfo
{
	std::wstring name;
	std::wstring html_url;
	std::wstring body;
	bool isNewer;
	bool ok;
};

static std::thread fetchThread;
static std::atomic_bool fetchCancel{false};
static std::atomic_bool fetchInProgress{false};
static std::wstring latestReleaseUrl;

// --- JSON parsing helpers (top-level string extraction only) ---

static size_t SkipWs(const std::string &s, size_t pos)
{
	while (pos < s.size())
	{
		char c = s[pos];
		if (c != ' ' && c != '\t' && c != '\n' && c != '\r') break;
		++pos;
	}
	return pos;
}

static size_t SkipJsonString(const std::string &s, size_t pos)
{
	// pos points at opening "
	++pos;
	while (pos < s.size())
	{
		char c = s[pos];
		if (c == '\\')
		{
			pos += 2;
		}
		else if (c == '"')
		{
			return pos + 1;
		}
		else
		{
			++pos;
		}
	}
	return pos;
}

static size_t SkipJsonContainer(const std::string &s, size_t pos)
{
	// pos points at { or [
	char open = s[pos];
	char close = (open == '{') ? '}' : ']';
	++pos;
	int depth = 1;
	while (pos < s.size() && depth > 0)
	{
		char c = s[pos];
		if (c == '"')
		{
			pos = SkipJsonString(s, pos);
		}
		else if (c == '{' || c == '[')
		{
			++depth;
			++pos;
		}
		else if (c == '}' || c == ']')
		{
			--depth;
			++pos;
		}
		else
		{
			++pos;
		}
	}
	return pos;
}

static size_t SkipJsonValue(const std::string &s, size_t pos)
{
	pos = SkipWs(s, pos);
	if (pos >= s.size()) return pos;
	char c = s[pos];
	if (c == '"') return SkipJsonString(s, pos);
	if (c == '{' || c == '[') return SkipJsonContainer(s, pos);
	while (pos < s.size())
	{
		char ch = s[pos];
		if (ch == ',' || ch == '}' || ch == ']') break;
		++pos;
	}
	return pos;
}

// /repos/.../releases/latest が返すトップレベルのオブジェクトから
// 指定キーの文字列値を取り出す。エスケープ解除前の生の中身を返す。
static bool FindTopLevelStringValue(const std::string &json, const std::string &key, std::string &outRaw)
{
	size_t pos = json.find('{');
	if (pos == std::string::npos) return false;
	++pos;

	while (pos < json.size())
	{
		pos = SkipWs(json, pos);
		if (pos >= json.size()) return false;
		if (json[pos] == '}') return false;
		if (json[pos] == ',') { ++pos; continue; }
		if (json[pos] != '"') return false;

		size_t keyStart = pos + 1;
		size_t keyEnd = SkipJsonString(json, pos);
		if (keyEnd <= keyStart) return false;
		std::string thisKey(json, keyStart, keyEnd - 1 - keyStart);
		pos = keyEnd;

		pos = SkipWs(json, pos);
		if (pos >= json.size() || json[pos] != ':') return false;
		++pos;
		pos = SkipWs(json, pos);

		if (thisKey == key && pos < json.size() && json[pos] == '"')
		{
			size_t valStart = pos + 1;
			size_t valEnd = SkipJsonString(json, pos);
			if (valEnd <= valStart) return false;
			outRaw.assign(json, valStart, valEnd - 1 - valStart);
			return true;
		}

		pos = SkipJsonValue(json, pos);
	}
	return false;
}

static std::wstring UnescapeJsonStringW(const std::string &escaped)
{
	std::string buf;
	buf.reserve(escaped.size());
	for (size_t i = 0; i < escaped.size(); )
	{
		if (escaped[i] == '\\' && i + 1 < escaped.size())
		{
			char c = escaped[i + 1];
			switch (c)
			{
			case '"':  buf += '"';  i += 2; break;
			case '\\': buf += '\\'; i += 2; break;
			case '/':  buf += '/';  i += 2; break;
			case 'b':  buf += '\b'; i += 2; break;
			case 'f':  buf += '\f'; i += 2; break;
			case 'n':  buf += '\n'; i += 2; break;
			case 'r':  buf += '\r'; i += 2; break;
			case 't':  buf += '\t'; i += 2; break;
			case 'u':
				if (i + 5 < escaped.size())
				{
					char hex[5] = { escaped[i + 2], escaped[i + 3], escaped[i + 4], escaped[i + 5], 0 };
					unsigned int cp = (unsigned int)strtoul(hex, nullptr, 16);
					if (cp < 0x80)
					{
						buf += (char)cp;
					}
					else if (cp < 0x800)
					{
						buf += (char)(0xC0 | (cp >> 6));
						buf += (char)(0x80 | (cp & 0x3F));
					}
					else
					{
						// サロゲートペアは単一 \uXXXX 単位で UTF-8 化する
						// (リリースノート用途では実用上問題なし)
						buf += (char)(0xE0 | (cp >> 12));
						buf += (char)(0x80 | ((cp >> 6) & 0x3F));
						buf += (char)(0x80 | (cp & 0x3F));
					}
					i += 6;
				}
				else
				{
					i += 2;
				}
				break;
			default:
				buf += c;
				i += 2;
				break;
			}
		}
		else
		{
			buf += escaped[i];
			++i;
		}
	}
	return utf8_string_to_wstring(buf);
}

// マルチライン Edit 用に LF を CRLF に正規化する
static std::wstring NormalizeNewlinesW(const std::wstring &src)
{
	std::wstring out;
	out.reserve(src.size() + src.size() / 8);
	for (size_t i = 0; i < src.size(); ++i)
	{
		WCHAR c = src[i];
		if (c == L'\r')
		{
			// 直後の \n とまとめて CRLF にする
			out += L"\r\n";
			if (i + 1 < src.size() && src[i + 1] == L'\n') ++i;
		}
		else if (c == L'\n')
		{
			out += L"\r\n";
		}
		else
		{
			out += c;
		}
	}
	return out;
}

// --- Version compare ---

struct SemVer { int major; int minor; int patch; };

static bool ParseSemVer(LPCWSTR s, SemVer &out)
{
	if (s == nullptr) return false;
	// 先頭の 'v' などを許容
	while (*s != L'\0' && (*s < L'0' || *s > L'9')) ++s;
	if (*s == L'\0') return false;

	WCHAR *endp = nullptr;
	long maj = wcstol(s, &endp, 10);
	if (endp == s || *endp != L'.') return false;
	s = endp + 1;
	long min = wcstol(s, &endp, 10);
	if (endp == s || *endp != L'.') return false;
	s = endp + 1;
	long pat = wcstol(s, &endp, 10);
	if (endp == s) return false;

	out.major = (int)maj;
	out.minor = (int)min;
	out.patch = (int)pat;
	return true;
}

static int CompareSemVer(const SemVer &a, const SemVer &b)
{
	if (a.major != b.major) return (a.major < b.major) ? -1 : 1;
	if (a.minor != b.minor) return (a.minor < b.minor) ? -1 : 1;
	if (a.patch != b.patch) return (a.patch < b.patch) ? -1 : 1;
	return 0;
}

// --- HTTP fetch ---

static bool FetchLatestReleaseJson(std::string &outJson)
{
	HINTERNET hInet = InternetOpenW(TEXTSERVICE_NAME L"/" TEXTSERVICE_VER,
		INTERNET_OPEN_TYPE_PRECONFIG, nullptr, nullptr, 0);
	if (hInet == nullptr) return false;

	// 通信中にダイアログを閉じてもなるべく早く抜けるためタイムアウトを短めに設定
	DWORD timeoutMs = 15000;
	InternetSetOptionW(hInet, INTERNET_OPTION_CONNECT_TIMEOUT, &timeoutMs, sizeof(timeoutMs));
	InternetSetOptionW(hInet, INTERNET_OPTION_RECEIVE_TIMEOUT, &timeoutMs, sizeof(timeoutMs));
	InternetSetOptionW(hInet, INTERNET_OPTION_SEND_TIMEOUT, &timeoutMs, sizeof(timeoutMs));

	LPCWSTR headers =
		L"Accept: application/vnd.github+json\r\n"
		L"X-GitHub-Api-Version: 2022-11-28\r\n";
	DWORD flags = INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE;

	HINTERNET hUrl = InternetOpenUrlW(hInet, LATEST_API_URL, headers, (DWORD)wcslen(headers), flags, 0);
	if (hUrl == nullptr)
	{
		InternetCloseHandle(hInet);
		return false;
	}

	DWORD statusCode = 0;
	DWORD len = sizeof(statusCode);
	if (HttpQueryInfoW(hUrl, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &statusCode, &len, 0) == FALSE
		|| statusCode != HTTP_STATUS_OK)
	{
		InternetCloseHandle(hUrl);
		InternetCloseHandle(hInet);
		return false;
	}

	outJson.clear();
	CHAR rbuf[RECVBUFSIZE];
	while (true)
	{
		if (fetchCancel) break;

		DWORD bytesRead = 0;
		ZeroMemory(rbuf, sizeof(rbuf));
		if (InternetReadFile(hUrl, rbuf, sizeof(rbuf), &bytesRead) == FALSE)
		{
			InternetCloseHandle(hUrl);
			InternetCloseHandle(hInet);
			return false;
		}
		if (bytesRead == 0) break;
		outJson.append(rbuf, bytesRead);

		// 想定よりも極端に大きいレスポンスは打ち切る (DoS 防御)
		if (outJson.size() > 1024 * 1024) break;
	}

	InternetCloseHandle(hUrl);
	InternetCloseHandle(hInet);

	return !fetchCancel;
}

static void FetchThreadProc(HWND hDlg)
{
	ReleaseInfo *info = new ReleaseInfo();
	info->ok = false;
	info->isNewer = false;

	std::string json;
	if (FetchLatestReleaseJson(json))
	{
		std::string nameRaw, urlRaw, bodyRaw;
		if (!FindTopLevelStringValue(json, "name", nameRaw) || nameRaw.empty())
		{
			FindTopLevelStringValue(json, "tag_name", nameRaw);
		}
		FindTopLevelStringValue(json, "html_url", urlRaw);
		FindTopLevelStringValue(json, "body", bodyRaw);

		info->name = UnescapeJsonStringW(nameRaw);
		info->html_url = UnescapeJsonStringW(urlRaw);
		info->body = NormalizeNewlinesW(UnescapeJsonStringW(bodyRaw));

		SemVer cur, lat;
		if (ParseSemVer(TEXTSERVICE_VER, cur) && ParseSemVer(info->name.c_str(), lat))
		{
			info->isNewer = (CompareSemVer(lat, cur) > 0);
		}
		info->ok = !info->name.empty();
	}

	if (PostMessageW(hDlg, WM_USER_FETCH_DONE, 0, (LPARAM)info) == FALSE)
	{
		delete info;
	}
}

static void StartFetch(HWND hDlg)
{
	if (fetchInProgress) return;

	fetchInProgress = true;
	fetchCancel = false;

	EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHECK_UPDATE), FALSE);
	SetDlgItemTextW(hDlg, IDC_EDIT_LATEST_VERSION, L"確認中...");

	try
	{
		if (fetchThread.joinable())
		{
			fetchThread.detach();
		}
		fetchThread = std::thread(FetchThreadProc, hDlg);
	}
	catch (...)
	{
		fetchInProgress = false;
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHECK_UPDATE), TRUE);
		SetDlgItemTextW(hDlg, IDC_EDIT_LATEST_VERSION, L"取得に失敗しました");
	}
}

INT_PTR CALLBACK DlgProcVersion(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		SetDlgItemTextW(hDlg, IDC_EDIT_CURRENT_VERSION, TEXTSERVICE_VER);
		SetDlgItemTextW(hDlg, IDC_EDIT_LATEST_VERSION, L"未確認");
		SetDlgItemTextW(hDlg, IDC_EDIT_RELEASE_NOTE, L"");
		latestReleaseUrl.clear();
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDC_BUTTON_CHECK_UPDATE:
			StartFetch(hDlg);
			return TRUE;

		case IDC_BUTTON_OPEN_RELEASE_PAGE:
		{
			LPCWSTR url = latestReleaseUrl.empty() ? RELEASES_PAGE_URL : latestReleaseUrl.c_str();
			ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
			return TRUE;
		}

		default:
			break;
		}
		break;

	case WM_USER_FETCH_DONE:
	{
		ReleaseInfo *info = (ReleaseInfo *)lParam;
		fetchInProgress = false;
		EnableWindow(GetDlgItem(hDlg, IDC_BUTTON_CHECK_UPDATE), TRUE);

		if (info != nullptr && info->ok)
		{
			std::wstring latestText = info->name;
			if (info->isNewer)
			{
				latestText += L"  (新しいバージョンがあります)";
			}
			else
			{
				latestText += L"  (最新です)";
			}
			SetDlgItemTextW(hDlg, IDC_EDIT_LATEST_VERSION, latestText.c_str());
			SetDlgItemTextW(hDlg, IDC_EDIT_RELEASE_NOTE, info->body.c_str());

			if (!info->html_url.empty())
			{
				latestReleaseUrl = info->html_url;
			}
		}
		else
		{
			SetDlgItemTextW(hDlg, IDC_EDIT_LATEST_VERSION, L"取得に失敗しました");
		}

		delete info;
		return TRUE;
	}

	case WM_DESTROY:
	{
		fetchCancel = true;

		if (fetchThread.joinable())
		{
			HANDLE h = fetchThread.native_handle();
			if (h != nullptr && WaitForSingleObject(h, 200) == WAIT_OBJECT_0)
			{
				fetchThread.join();
			}
			else
			{
				fetchThread.detach();
			}
		}

		// PostMessage 済みでまだ処理されていない通知の中身を解放
		MSG msg;
		while (PeekMessageW(&msg, hDlg, WM_USER_FETCH_DONE, WM_USER_FETCH_DONE, PM_REMOVE))
		{
			delete (ReleaseInfo *)msg.lParam;
		}

		return TRUE;
	}

	default:
		break;
	}

	return FALSE;
}
