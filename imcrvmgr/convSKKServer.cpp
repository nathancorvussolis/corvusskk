
#include "imcrvmgr.h"
#include "eucjis2004.h"

void GetSKKServerVersion();
void AnalyzeSKKServer(const std::wstring &res, CANDIDATES &candidates);

#define BUFSIZE		0x2000
#define RBUFSIZE	0x800
#define KEYSIZE		0x100

//client
#define SKK_REQ		0x31 //'1'
#define SKK_VER		0x32 //'2'
//server
#define SKK_HIT		0x31 //'1'

SOCKET sock = INVALID_SOCKET;

void ConvSKKServer(const std::wstring &text, CANDIDATES &candidates)
{
	CHAR key[KEYSIZE];
	size_t size;
	CHAR buf[BUFSIZE];
	size_t idxbuf = 0;
	CHAR rbuf[RBUFSIZE];
	int n, nn;
	WCHAR wbuf[BUFSIZE];

	size = _countof(key) - 2;
	if(WideCharToEucJis2004(text.c_str(), NULL, key + 1, &size))
	{
		key[0] = SKK_REQ;
		key[size + 1] = 0x20;
		key[size + 2] = 0x00;
		size += 2;
	}
	else
	{
		return;
	}

	ZeroMemory(buf, sizeof(buf));

	if(sock == INVALID_SOCKET)
	{
		ConnectSKKServer();
	}

	GetSKKServerVersion();

	if(send(sock, key, (int)size, 0) == SOCKET_ERROR)
	{
		DisconnectSKKServer();
		goto end;
	}

	while(true)
	{
		ZeroMemory(rbuf, sizeof(rbuf));
		n = recv(sock, rbuf, sizeof(rbuf), 0);
		if(n == SOCKET_ERROR || n == 0)
		{
			DisconnectSKKServer();
			goto end;
		}

		nn = n;

		if((sizeof(buf) - idxbuf) < (size_t)n)
		{
			n = (int)(sizeof(buf) - idxbuf);
		}

		if(n > 0)
		{
			memcpy_s(buf + idxbuf, sizeof(buf) - idxbuf, rbuf, n);
			idxbuf += n;
		}

		if(nn < sizeof(rbuf))
		{
			if(rbuf[nn - 1] == 0x0A/*LF*/)
			{
				break;
			}
		}
	}

end:
	if(idxbuf > 0 && buf[0] == SKK_HIT)
	{
		size = _countof(wbuf);
		if(EucJis2004ToWideChar(buf, NULL, wbuf, &size))
		{
			std::wstring res(&wbuf[1]);
			if(!res.empty())
			{
				AnalyzeSKKServer(res, candidates);
			}
		}
	}
}

void ConnectSKKServer()
{
	ADDRINFOW aiwHints;
	ADDRINFOW *paiwResult;
	ADDRINFOW *paiw;

	ZeroMemory(&aiwHints, sizeof(aiwHints));
	aiwHints.ai_family = AF_UNSPEC;
	aiwHints.ai_socktype = SOCK_STREAM;
	aiwHints.ai_protocol = IPPROTO_TCP;

	if(GetAddrInfoW(host, port, &aiwHints, &paiwResult) != 0)
	{
		return;
	}

	for(paiw = paiwResult; paiw != NULL; paiw = paiw->ai_next)
	{
		sock = socket(paiw->ai_family, paiw->ai_socktype, paiw->ai_protocol); 
		if(sock == INVALID_SOCKET)
		{
			continue;
		}

		if(setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
		if(setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}

		if(connect(sock, paiw->ai_addr, (int)paiw->ai_addrlen) == SOCKET_ERROR)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
		else
		{
			break;
		}
	}

	FreeAddrInfoW(paiwResult);
}

void DisconnectSKKServer()
{
	if(sock != INVALID_SOCKET)
	{
		shutdown(sock, SD_BOTH);
		closesocket(sock);
		sock = INVALID_SOCKET;
	}
}

void GetSKKServerVersion()
{
	int n;
	CHAR rbuf[RBUFSIZE];
	CHAR sbuf = SKK_VER;

	if(send(sock, &sbuf, 1, 0) == SOCKET_ERROR)
	{
		DisconnectSKKServer();
		ConnectSKKServer();
	}
	else {
		ZeroMemory(rbuf, sizeof(rbuf));
		n = recv(sock, rbuf, sizeof(rbuf), 0);
		if(n == SOCKET_ERROR || n == 0)
		{
			DisconnectSKKServer();
			ConnectSKKServer();
		}
	}
}

void AnalyzeSKKServer(const std::wstring &res, CANDIDATES &candidates)
{
	std::vector<std::wstring> es;
	std::vector<std::wstring>::iterator es_itr;
	size_t i, is, ie;
	std::wstring s;
	std::wregex re;
	std::wstring fmt;

	//エントリを「/」で分割
	i = 0;
	s = res;
	while(i < s.size())
	{
		is = s.find_first_of(L'/', i);
		ie = s.find_first_of(L'/', is + 1);
		if(ie == std::wstring::npos)
		{
			break;
		}
		es.push_back(s.substr(i + 1, ie - is - 1));
		i = ie;
	}

	// 「;」→「\t」、concatを置換
	for(es_itr = es.begin(); es_itr != es.end(); es_itr++)
	{
		s = *es_itr;

		re.assign(L";");
		fmt.assign(L"\t");
		s = std::regex_replace(s, re, fmt);

		if(s.find_first_of(L'\t') == 0)
		{
			continue;
		}

		re.assign(L".*\\(concat \".*\"\\).*");
		if(std::regex_match(s, re))
		{
			re.assign(L"(.*)\\(concat \"(.*)\"\\)(.*)");
			fmt.assign(L"$1$2$3");
			s = std::regex_replace(s, re, fmt);	//annotation if annotation has / candidate if annotaion doesnot has
			s = std::regex_replace(s, re, fmt);	//candidate if annotaion has

			re.assign(L"\\\\057");
			fmt.assign(L"/");
			s = std::regex_replace(s, re, fmt);

			re.assign(L"\\\\073");
			fmt.assign(L";");
			s = std::regex_replace(s, re, fmt);
		}

		if(s.find_first_of(L'\t') == std::wstring::npos)
		{
			candidates.push_back(CANDIDATE(s, L""));
		}
		else
		{
			candidates.push_back(
				CANDIDATE(s.substr(0, s.find_first_of(L'\t')), s.substr(s.find_first_of(L'\t') + 1)));
		}
	}
}
