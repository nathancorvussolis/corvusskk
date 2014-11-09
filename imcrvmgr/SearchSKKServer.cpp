
#include "eucjis2004.h"
#include "utf8.h"
#include "parseskkdic.h"
#include "imcrvmgr.h"

SOCKET sock = INVALID_SOCKET;

std::wstring SearchSKKServer(const std::wstring &searchkey)
{
	std::wstring candidate;
	std::string key;
	std::string buf;
	size_t pidx;
	CHAR rbuf[RECVBUFSIZE];
	int n;

	if(!serv)
	{
		return candidate;
	}

	key.push_back(SKK_REQ);
	switch(encoding)
	{
	case 0:
		key.append(wstring_to_eucjis2004_string(searchkey));
		break;
	case 1:
		key.append(wstring_to_utf8_string(searchkey));
		break;
	default:
		return candidate;
		break;
	}
	key.push_back('\x20'/*SP*/);

	if(sock == INVALID_SOCKET)
	{
		ConnectSKKServer();
	}

	GetSKKServerInfo(SKK_VER);

	if(send(sock, key.c_str(), (int)key.size(), 0) == SOCKET_ERROR)
	{
		DisconnectSKKServer();
		goto end;
	}

	while(true)
	{
		ZeroMemory(rbuf, sizeof(rbuf));
		n = recv(sock, rbuf, sizeof(rbuf) - 1, 0);
		if(n == SOCKET_ERROR || n <= 0)
		{
			DisconnectSKKServer();
			goto end;
		}

		buf += rbuf;

		if(n <= _countof(rbuf) && rbuf[n - 1] == '\n'/*LF*/)
		{
			break;
		}
	}

end:
	if(buf.size() > 1 && buf.front() == SKK_HIT)
	{
		std::string s;
		std::smatch m;
		std::regex r;
		size_t ds;

		s = buf.substr(1);
		r.assign("/[^/]+");
		while(std::regex_search(s, m, r))
		{
			switch(encoding)
			{
			case 0:
				ds = -1;
				if(EucJis2004ToWideChar(m.str().c_str(), NULL, NULL, &ds))
				{
					candidate += eucjis2004_string_to_wstring(m.str());
				}
				break;
			case 1:
				candidate += utf8_string_to_wstring(m.str());
				break;
			default:
				break;
			}

			if(candidate.size() >= DICBUFSIZE)
			{
				candidate.erase(DICBUFSIZE);

				if((pidx = candidate.find_last_of(L'/')) != std::string::npos)
				{
					candidate.erase(pidx);
					candidate.append(L"/\n");
					break;
				}
			}

			s = m.suffix();
		}
	}

	return candidate;
}

void ConnectSKKServer()
{
	ADDRINFOW aiwHints;
	ADDRINFOW *paiwResult;
	ADDRINFOW *paiw;
	u_long mode;
	timeval tv;
	fd_set fdw, fde;

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

		mode = 1;
		ioctlsocket(sock, FIONBIO, &mode);

		if(connect(sock, paiw->ai_addr, (int)paiw->ai_addrlen) == SOCKET_ERROR)
		{
			if(WSAGetLastError() != WSAEWOULDBLOCK)
			{
				closesocket(sock);
				sock = INVALID_SOCKET;
				continue;
			}
		}

		mode = 0;
		ioctlsocket(sock, FIONBIO, &mode);

		tv.tv_sec = timeout / 1000;
		tv.tv_usec = (timeout % 1000) * 1000;

		FD_ZERO(&fdw);
		FD_ZERO(&fde);
		FD_SET(sock, &fdw);
		FD_SET(sock, &fde);

		select(0, NULL, &fdw, &fde, &tv);
		if(FD_ISSET(sock, &fdw))
		{
			break;
		}

		DisconnectSKKServer();
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

std::wstring GetSKKServerInfo(CHAR req)
{
	std::wstring ret;
	std::string sbuf;
	std::string buf;
	CHAR rbuf[RECVBUFSIZE];
	int n;

	if(!serv)
	{
		return ret;
	}

	if(send(sock, &req, 1, 0) == SOCKET_ERROR)
	{
		DisconnectSKKServer();
		ConnectSKKServer();
	}
	else
	{
		while(true)
		{
			ZeroMemory(rbuf, sizeof(rbuf));
			n = recv(sock, rbuf, sizeof(rbuf) - 1, 0);
			if(n == SOCKET_ERROR || n <= 0)
			{
				DisconnectSKKServer();
				ConnectSKKServer();
				break;
			}

			sbuf += rbuf;

			if(rbuf[n - 1] == '\x20'/*SP*/)
			{
				break;
			}
		}

		switch(encoding)
		{
		case 0:
			ret = eucjis2004_string_to_wstring(sbuf);
			break;
		case 1:
			ret = utf8_string_to_wstring(sbuf);
			break;
		default:
			break;
		}
	}

	return ret;
}
