
#include "eucjptable.h"
#include "eucjis2004.h"

/* EUC-JP
    0x00..    0x7F ASCII
  0xA1**..  0xFE** JIS X 0208
  0x8E**..  0x8E** JIS X 0201 HALFWIDTH KATAKANA
0x8FA1**..0x8FFE** JIS X 0212
  (** : 0xA1..0xFE)
*/

// EUC 1文字分をUnicode Code Pointへ変換

size_t EucJPToUcp(LPCSTR src, size_t srcsize, PUCSCHAR ucp1, PUCSCHAR ucp2)
{
	CONST CHAR as = 0x00;
	CONST CHAR ae = 0x7F;
	CONST CHAR ss2 = (CHAR)0x8E;
	CONST CHAR ss3 = (CHAR)0x8F;
	CONST CHAR ejd = (CHAR)0x80;
	CONST CHAR ejs = 0x21;
	CONST CHAR eje = 0x7E;
	CHAR ej[2];
	size_t srcused = 0;

	if (src == nullptr || srcsize == 0 || ucp1 == nullptr || ucp2 == nullptr)
	{
		return 0;
	}

	*ucp1 = 0;
	*ucp2 = 0;

	if (as <= src[0] && src[0] <= ae)	// ASCII
	{
		*ucp1 = src[0];
		*ucp2 = 0;
		srcused = 1;
	}
	else
	{
		switch (src[0])
		{
		case ss3:	// JIS X 0212
			if (srcsize < 3)
			{
				break;
			}

			ej[0] = 0;
			ej[1] = 0;

			if ((UCHAR)src[1] >= (UCHAR)ejd)
			{
				ej[0] = (CHAR)((UCHAR)src[1] - (UCHAR)ejd);
			}

			if ((UCHAR)src[2] >= (UCHAR)ejd)
			{
				ej[1] = (CHAR)((UCHAR)src[2] - (UCHAR)ejd);
			}

			if ((ej[0] >= ejs && ej[0] <= eje) && (ej[1] >= ejs && ej[1] <= eje))
			{
				*ucp1 = 0;
				if (euc0212i[ej[0] - ejs] != 0 && euc0212i[ej[0] - ejs] <= ROW0212NUM)
				{
					*ucp1 = euc0212[euc0212i[ej[0] - ejs] - 1][ej[1] - ejs];
				}
				*ucp2 = 0;
				srcused = 3;
			}
			break;

		case ss2:	// JIS X 0201 片仮名
			if (srcsize < 2)
			{
				break;
			}

			ej[0] = 0;

			if ((UCHAR)src[1] >= (UCHAR)ejd)
			{
				ej[0] = (CHAR)((UCHAR)src[1] - (UCHAR)ejd);
			}

			if (ej[0] >= ejs && ej[0] <= eje)
			{
				*ucp1 = eucK[ej[0] - ejs];
				*ucp2 = 0;
				srcused = 2;
			}
			break;

		default:	// JIS X 0208
			if (srcsize < 2)
			{
				break;
			}

			ej[0] = 0;
			ej[1] = 0;

			if ((UCHAR)src[0] >= (UCHAR)ejd)
			{
				ej[0] = (CHAR)((UCHAR)src[0] - (UCHAR)ejd);
			}

			if ((UCHAR)src[1] >= (UCHAR)ejd)
			{
				ej[1] = (CHAR)((UCHAR)src[1] - (UCHAR)ejd);
			}

			if ((ej[0] >= ejs && ej[0] <= eje) && (ej[1] >= ejs && ej[1] <= eje))
			{
				*ucp1 = 0;
				CHAR ku = ej[0] - ejs + 1;
				CHAR ten = ej[1] - ejs + 1;
				if ((jisx0208e[ku - 1][(ten - (ten % 16)) / 16] & (0x0001 << (ten % 16))) != 0)
				{
					*ucp1 = euc1[ej[0] - ejs][ej[1] - ejs];
				}
				*ucp2 = 0;
				srcused = 2;
			}
			break;
		}
	}

	return srcused;
}

// EUC-JPをUTF-16へ変換

BOOL EucJPToWideChar(LPCSTR src, size_t *srcsize, LPWSTR dst, size_t *dstsize)
{
	size_t si = 0, di = 0, ss = -1;
	UCSCHAR ucp[2];
	WCHAR utf16[2][2];
	size_t utf16num[2];

	if (dstsize == nullptr)
	{
		return FALSE;
	}

	if (dst == nullptr)
	{
		*dstsize = (size_t)-1;
	}

	if (src == nullptr)
	{
		*dstsize = 0;
		return FALSE;
	}

	if (srcsize != nullptr)
	{
		ss = *srcsize;
	}

	for (si = 0; ; si++)
	{
		if ((ss <= si) || (*(src + si) == '\0'))
		{
			break;
		}

		// EUC-JPからUnicode Code Pointへ変換
		size_t used = EucJPToUcp(src + si, ss - si, &ucp[0], &ucp[1]);
		if ((ucp[0] == 0) || (used == 0))
		{
			AddNullWideChar(srcsize, si, dst, dstsize, di);
			return FALSE;
		}
		si += used - 1;

		// Unicode Code PointからUTF-16へ変換
		for (int i = 0; i < 2; i++)
		{
			utf16num[i] = 0;
			if (ucp[i] != 0)
			{
				utf16num[i] = UcpToWideChar(ucp[i], &utf16[i][0], &utf16[i][1]);
			}
		}

		if (*dstsize <= di + utf16num[0] + utf16num[1])	// limit
		{
			AddNullWideChar(srcsize, si, dst, dstsize, di);
			return FALSE;
		}

		for (int i = 0; i < 2; i++)
		{
			if (dst != nullptr)
			{
				for (int j = 0; j < (int)utf16num[i] && j < 2; j++)
				{
					*(dst + di + j) = utf16[i][j];
				}
			}
			di += utf16num[i];
		}
	}

	AddNullWideChar(srcsize, si, dst, dstsize, di);
	return TRUE;
}

// UTF-16をEUC-JPへ変換

BOOL WideCharToEucJP(LPCWSTR src, size_t *srcsize, LPSTR dst, size_t *dstsize)
{
	CONST CHAR ss2 = (CHAR)0x8E;
	CONST CHAR ss3 = (CHAR)0x8F;
	CONST CHAR ejd = (CHAR)0x80;
	CONST CHAR ejs = 0x21;
	size_t si = 0, di = 0, ss = -1;
	UCSCHAR ucp;
	BOOL exist;

	if (dstsize == nullptr)
	{
		return FALSE;
	}

	if (dst == nullptr)
	{
		*dstsize = (size_t)-1;
	}

	if (src == nullptr)
	{
		*dstsize = 0;
		return FALSE;
	}

	if (srcsize != nullptr)
	{
		ss = *srcsize;
	}

	for (si = 0; ; si++)
	{
		if ((ss <= si) || (*(src + si) == L'\0'))
		{
			break;
		}

		if (*(src + si) <= L'\x7F')	// ASCII
		{
			if (*dstsize <= di + 1)	// limit
			{
				AddNullMultiByte(srcsize, si, dst, dstsize, di);
				return FALSE;
			}
			if (dst != nullptr)
			{
				*(dst + di) = (CHAR) * (src + si);
			}
			++di;
		}
		else
		{
			exist = FALSE;

			// surrogate pair なし
			ucp = *(src + si);

			// 互換性
			if (!exist)
			{
				for (int i = 0; i < CMPEUCJPNUM; i++)
				{
					if (ucp == eucjpcmp[i].ucp) 
					{
						if (*dstsize <= di + 2)	// limit
						{
							AddNullMultiByte(srcsize, si, dst, dstsize, di);
							return FALSE;
						}
						if (dst != nullptr)
						{
							*(dst + di) = (CHAR)(eucjpcmp[i].euc >> 8);
							*(dst + di + 1) = (CHAR)(eucjpcmp[i].euc & 0xFF);
						}
						di += 2;
						exist = TRUE;
						break;
					}
				}
			}

			// JIS X 0218
			if (!exist)
			{
				for (int i = 0; i < ROWNUM; i++)
				{
					for (int j = 0; j < CELLNUM; j++)
					{
						CHAR ku = i + 1;
						CHAR ten = j + 1;
						if ((jisx0208e[ku - 1][(ten - (ten % 16)) / 16] & (0x0001 << (ten % 16))) != 0 &&
							ucp == euc1[i][j])
						{
							if (*dstsize <= di + 2)	// limit
							{
								AddNullMultiByte(srcsize, si, dst, dstsize, di);
								return FALSE;
							}
							if (dst != nullptr)
							{
								*(dst + di) = (CHAR)((UCHAR)(ejs + i) + (UCHAR)ejd);
								*(dst + di + 1) = (CHAR)((UCHAR)(ejs + j) + (UCHAR)ejd);
							}
							di += 2;
							exist = TRUE;
							break;
						}
					}

					if (exist)
					{
						break;
					}
				}
			}

			// JIS X 0212
			if (!exist)
			{
				for (int i = 0; i < ROWNUM; i++)
				{
					for (int j = 0; j < CELLNUM; j++)
					{
						if (euc0212i[i] != 0 && euc0212i[i] <= ROW0212NUM &&
							ucp == euc0212[euc0212i[i] - 1][j])
						{
							if (*dstsize <= di + 3)	// limit
							{
								AddNullMultiByte(srcsize, si, dst, dstsize, di);
								return FALSE;
							}
							if (dst != nullptr)
							{
								*(dst + di) = ss3;
								*(dst + di + 1) = (CHAR)((UCHAR)(ejs + i) + (UCHAR)ejd);
								*(dst + di + 2) = (CHAR)((UCHAR)(ejs + j) + (UCHAR)ejd);
							}
							di += 3;
							exist = TRUE;
							break;
						}
					}

					if (exist)
					{
						break;
					}
				}
			}

			// JIS X 0201 片仮名
			if (!exist)
			{
				for (int i = 0; i < ANKNUM; i++)
				{
					if (ucp == eucK[i])
					{
						if (*dstsize <= di + 2)	// limit
						{
							AddNullMultiByte(srcsize, si, dst, dstsize, di);
							return FALSE;
						}
						if (dst != nullptr)
						{
							*(dst + di) = ss2;
							*(dst + di + 1) = (CHAR)((UCHAR)(ejs + i) + (UCHAR)ejd);
						}
						di += 2;
						exist = TRUE;
						break;
					}
				}
			}

			if (!exist)
			{
				AddNullMultiByte(srcsize, si, dst, dstsize, di);
				return FALSE;
			}
		}
	}

	AddNullMultiByte(srcsize, si, dst, dstsize, di);
	return TRUE;
}

std::wstring eucjp_string_to_wstring(const std::string &s)
{
	std::wstring ret;
	size_t len;

	BOOL b = EucJPToWideChar(s.c_str(), nullptr, nullptr, &len);
	if (b && len > 0)
	{
		try
		{
			LPWSTR wcs = new WCHAR[len];
			if (EucJPToWideChar(s.c_str(), nullptr, wcs, &len))
			{
				ret = wcs;
			}
			delete[] wcs;
		}
		catch (...)
		{
		}
	}

	return ret;
}

std::string wstring_to_eucjp_string(const std::wstring &s)
{
	std::string ret;
	size_t len;

	BOOL b = WideCharToEucJP(s.c_str(), nullptr, nullptr, &len);
	if (b && len > 0)
	{
		try
		{
			LPSTR euc = new CHAR[len];
			if (WideCharToEucJP(s.c_str(), nullptr, euc, &len))
			{
				ret = euc;
			}
			delete[] euc;
		}
		catch (...)
		{
		}
	}

	return ret;
}
