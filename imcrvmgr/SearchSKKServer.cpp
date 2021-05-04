
#include "eucjis2004.h"
#include "eucjp.h"
#include "utf8.h"
#include "imcrvmgr.h"

SOCKET skksocket = INVALID_SOCKET;

std::wstring SearchSKKServer(const std::wstring &searchkey)
{
	std::string key;
	std::wstring candidate;
	CHAR rbuf[RECVBUFSIZE];
	std::string buf;

	if (serv)
	{
		if (skksocket == INVALID_SOCKET)
		{
			StartConnectSKKServer();
			return candidate;
		}
	}
	else
	{
		DisconnectSKKServer();
		return candidate;
	}

	key.push_back(SKK_REQ);
	std::string skey;
	switch (encoding)
	{
	case 0:
		skey = wstring_to_eucjis2004_string(searchkey);
		if (!skey.empty())
		{
			key += skey;
		}
		else
		{
			skey = wstring_to_eucjp_string(searchkey);
			if (!skey.empty())
			{
				key += skey;
			}
			else
			{
				return candidate;
			}
		}
		break;
	case 1:
		skey = wstring_to_utf8_string(searchkey);
		if (!skey.empty())
		{
			key += skey;
		}
		else
		{
			return candidate;
		}
		break;
	default:
		return candidate;
		break;
	}
	key.push_back('\x20'/*SP*/);

	if (send(skksocket, key.c_str(), (int)key.size(), 0) == SOCKET_ERROR)
	{
		StartConnectSKKServer();
		goto end;
	}

	while (true)
	{
		ZeroMemory(rbuf, sizeof(rbuf));
		int n = recv(skksocket, rbuf, sizeof(rbuf) - 1, 0);
		if (n == SOCKET_ERROR || n <= 0)
		{
			StartConnectSKKServer();
			goto end;
		}

		buf += rbuf;

		if (n <= _countof(rbuf) && rbuf[n - 1] == '\n'/*LF*/)
		{
			break;
		}
	}

end:
	if (buf.size() > 1 && buf.front() == SKK_HIT)
	{
		std::string s;
		std::smatch m;
		static const std::regex r("/[^/]+");
		std::wstring c;

		s = buf.substr(1);
		while (std::regex_search(s, m, r))
		{
			switch (encoding)
			{
			case 0:
				c = eucjis2004_string_to_wstring(m.str());
				if (c.empty())
				{
					c = eucjp_string_to_wstring(m.str());
				}
				candidate += c;
				break;
			case 1:
				candidate += utf8_string_to_wstring(m.str());
				break;
			default:
				break;
			}

			s = m.suffix().str();
		}
	}

	return candidate;
}

void ConnectSKKServer()
{
	SOCKET sock;
	ADDRINFOW *paiwResult;
	ADDRINFOW *paiw;
	u_long mode;
	timeval tv;
	fd_set fdw, fde;

	ADDRINFOW aiwHints = {};
	aiwHints.ai_family = AF_UNSPEC;
	aiwHints.ai_socktype = SOCK_STREAM;
	aiwHints.ai_protocol = IPPROTO_TCP;

	if (GetAddrInfoW(host, port, &aiwHints, &paiwResult) != 0)
	{
		return;
	}

	for (paiw = paiwResult; paiw != nullptr; paiw = paiw->ai_next)
	{
		sock = socket(paiw->ai_family, paiw->ai_socktype, paiw->ai_protocol);
		if (sock == INVALID_SOCKET)
		{
			continue;
		}

		if (setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}
		if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == SOCKET_ERROR)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
			continue;
		}

		mode = 1;
		ioctlsocket(sock, FIONBIO, &mode);

		if (connect(sock, paiw->ai_addr, (int)paiw->ai_addrlen) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				closesocket(sock);
				sock = INVALID_SOCKET;
				continue;
			}
		}

		mode = 0;
		ioctlsocket(sock, FIONBIO, &mode);

		tv.tv_sec = (timeout - (timeout % 1000)) / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;

		FD_ZERO(&fdw);
		FD_ZERO(&fde);
		FD_SET(sock, &fdw);
		FD_SET(sock, &fde);

		select(0, nullptr, &fdw, &fde, &tv);

		if (FD_ISSET(sock, &fdw))
		{
			//connected
			skksocket = sock;
			break;
		}

		shutdown(sock, SD_BOTH);
		closesocket(sock);
	}

	FreeAddrInfoW(paiwResult);
}

void CloseSKKServSocket()
{
	if (skksocket != INVALID_SOCKET)
	{
		shutdown(skksocket, SD_BOTH);
		closesocket(skksocket);
		skksocket = INVALID_SOCKET;
	}
}

unsigned __stdcall ConnectSKKServerThread(void *p)
{
	if (TryEnterCriticalSection(&csSKKSocket))	// !
	{
		CloseSKKServSocket();

		ConnectSKKServer();

		LeaveCriticalSection(&csSKKSocket);	// !
	}

	return 0;
}

void StartConnectSKKServer()
{
	HANDLE h = reinterpret_cast<HANDLE>(
		_beginthreadex(nullptr, 0, ConnectSKKServerThread, nullptr, 0, nullptr));

	if (h != nullptr)
	{
		CloseHandle(h);
	}
}

void DisconnectSKKServer()
{
	if (TryEnterCriticalSection(&csSKKSocket))	// !
	{
		CloseSKKServSocket();

		LeaveCriticalSection(&csSKKSocket);	// !
	}
}

void CleanUpSKKServer()
{
	EnterCriticalSection(&csSKKSocket);	// !

	CloseSKKServSocket();

	LeaveCriticalSection(&csSKKSocket);	// !
}

std::wstring GetSKKServerInfo(CHAR req)
{
	std::wstring ret;

	if (serv)
	{
		if (skksocket == INVALID_SOCKET)
		{
			StartConnectSKKServer();
			return ret;
		}
	}
	else
	{
		DisconnectSKKServer();
		return ret;
	}

	if (send(skksocket, &req, 1, 0) == SOCKET_ERROR)
	{
		StartConnectSKKServer();
	}
	else
	{
		CHAR rbuf[RECVBUFSIZE];
		std::string buf;

		while (true)
		{
			ZeroMemory(rbuf, sizeof(rbuf));
			int n = recv(skksocket, rbuf, sizeof(rbuf) - 1, 0);
			if (n == SOCKET_ERROR || n <= 0)
			{
				StartConnectSKKServer();
				break;
			}

			buf += rbuf;

			if (rbuf[n - 1] == '\x20'/*SP*/)
			{
				break;
			}
		}

		switch (encoding)
		{
		case 0:
			ret = eucjis2004_string_to_wstring(buf);
			if (ret.empty())
			{
				ret = eucjp_string_to_wstring(buf);
			}
			break;
		case 1:
			ret = utf8_string_to_wstring(buf);
			break;
		default:
			break;
		}
	}

	return ret;
}
