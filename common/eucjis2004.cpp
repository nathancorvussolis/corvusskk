
#include "eucjis2004table.h"

/* EUC-JIS-2004
    0x00..    0x7F ASCII
  0xA1**..  0xFE** JIS X 0213 Plane 1
  0x8E**..  0x8E** JIS X 0201 HALFWIDTH KATAKANA
0x8FA1**..0x8FFE** JIS X 0213 Plane 2
  (** : 0xA1..0xFE)
*/

/* UTF-16 surrogate pair
U  = WWWWWyyyyyyxxxxxxxxxx Unicode Code Point (U+10000..U+10FFFF)
U' =  wwwwyyyyyyxxxxxxxxxx Unicode Code Point - 0x10000 (ex. U+10FFFF -> 0xFFFFF(20bit))
W1 =      110110wwwwyyyyyy 1st : D800..DBFF (use upper 10bit)
W2 =      110111xxxxxxxxxx 2nd : DC00..DFFF (use lower 10bit)
*/

#define SURROGATEPAIR_UCPMAX	0x10FFFF
#define SURROGATEPAIR_UCPMIN	0x10000
#define SURROGATEPAIR_MASK		0xFC00
#define SURROGATEPAIR_HIGH_MASK	0xD800
#define SURROGATEPAIR_LOW_MASK	0xDC00
#define SURROGATEPAIR_SEPBIT	10
#define SURROGATEPAIR_SEPMASK	0x3FF

// Unicode Code PointをUTF-16へ変換

size_t UcpToWideChar(UCSCHAR ucp, PWCHAR first, PWCHAR second)
{
	size_t ret = 0;

	if (first == nullptr || second == nullptr)
	{
		return 0;
	}

	*first = L'\0';
	*second = L'\0';

	if (ucp < SURROGATEPAIR_UCPMIN)
	{
		*first = (WCHAR)ucp;
		*second = L'\0';
		ret = 1;
	}
	else if (ucp <= SURROGATEPAIR_UCPMAX)  	//surrogate pair
	{
		*first = (WCHAR)(SURROGATEPAIR_HIGH_MASK | ((ucp - SURROGATEPAIR_UCPMIN) >> SURROGATEPAIR_SEPBIT));
		*second = (WCHAR)(SURROGATEPAIR_LOW_MASK | ((ucp - SURROGATEPAIR_UCPMIN) & SURROGATEPAIR_SEPMASK));
		ret = 2;
	}

	return ret;
}

// EUC 1文字分をUnicode Code Pointへ変換

size_t EucJis2004ToUcp(LPCSTR src, size_t srcsize, PUCSCHAR ucp1, PUCSCHAR ucp2)
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

	if (as <= src[0] && src[0] <= ae)	//ASCII
	{
		*ucp1 = src[0];
		*ucp2 = 0;
		srcused = 1;
	}
	else
	{
		switch (src[0])
		{
		case ss3:	// JIS X 0213 Plane 2
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
				if (euc2i[ej[0] - ejs] != 0 && euc2i[ej[0] - ejs] <= ROW2NUM)
				{
					*ucp1 = euc2[euc2i[ej[0] - ejs] - 1][ej[1] - ejs];
				}
				*ucp2 = 0;
				srcused = 3;
			}
			break;

		case ss2:	//JIS X 0201 halfwidth katakana
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

		default:	// JIS X 0213 Plane 1
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
				USHORT euc = ((USHORT)ej[0] << 8) | (USHORT)ej[1] | 0x8080;

				//結合文字
				for (int i = 0; i < CMBCHARNUM; i++)
				{
					if (euccmb[i].euc == euc)
					{
						*ucp1 = euccmb[i].ucp[0];
						*ucp2 = euccmb[i].ucp[1];
						srcused = 2;
						break;
					}
				}

				if (srcused != 0)
				{
					break;
				}

				*ucp1 = euc1[ej[0] - ejs][ej[1] - ejs];
				*ucp2 = 0;
				srcused = 2;
			}
			break;
		}
	}

	return srcused;
}

// 終端NULLを付加

void AddNullWideChar(size_t *srcsize, size_t si, LPWSTR dst, size_t *dstsize, size_t di)
{
	if (srcsize != nullptr)
	{
		*srcsize = si;
	}
	*dstsize = di + 1;
	if (dst != nullptr)
	{
		*(dst + di) = L'\0';
	}
}

// EUC-JIS-2004をUTF-16へ変換

BOOL EucJis2004ToWideChar(LPCSTR src, size_t *srcsize, LPWSTR dst, size_t *dstsize)
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

		// EUC-JIS-2004からUnicode Code Pointへ変換
		size_t used = EucJis2004ToUcp(src + si, ss - si, &ucp[0], &ucp[1]);
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

		if (*dstsize <= di + utf16num[0] + utf16num[1])	//limit
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

// 終端NULLを付加

void AddNullEucJis2004(size_t *srcsize, size_t si, LPSTR dst, size_t *dstsize, size_t di)
{
	if (srcsize != nullptr)
	{
		*srcsize = si;
	}
	*dstsize = di + 1;
	if (dst != nullptr)
	{
		*(dst + di) = '\0';
	}
}

// UTF-16をEUC-JIS-2004へ変換

BOOL WideCharToEucJis2004(LPCWSTR src, size_t *srcsize, LPSTR dst, size_t *dstsize)
{
	CONST CHAR ss2 = (CHAR)0x8E;
	CONST CHAR ss3 = (CHAR)0x8F;
	CONST CHAR ejd = (CHAR)0x80;
	CONST CHAR ejs = 0x21;
	size_t si = 0, di = 0, ss = -1;
	WCHAR first, second;
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

		if (*(src + si) <= L'\x7F')	//ASCII
		{
			if (*dstsize <= di + 1)	//limit
			{
				AddNullEucJis2004(srcsize, si, dst, dstsize, di);
				return FALSE;
			}
			if (dst != nullptr)
			{
				*(dst + di) = (CHAR)*(src + si);
			}
			++di;
		}
		else
		{
			exist = FALSE;

			first = *(src + si);
			if (si + 1 < ss)
			{
				second = *(src + si + 1);
			}
			else
			{
				second = 0;
			}

			if ((first >= SURROGATEPAIR_HIGH_MASK && first <= (SURROGATEPAIR_HIGH_MASK | SURROGATEPAIR_SEPMASK)) &&
				(second >= SURROGATEPAIR_LOW_MASK && second <= (SURROGATEPAIR_LOW_MASK | SURROGATEPAIR_SEPMASK)))
			{
				ucp = SURROGATEPAIR_UCPMIN +
					((((UCSCHAR)first & SURROGATEPAIR_SEPMASK) << SURROGATEPAIR_SEPBIT) |
					((UCSCHAR)second & SURROGATEPAIR_SEPMASK));
			}
			else
			{
				ucp = first;
			}

			//結合文字
			for (int i = 0; i < CMBCHARNUM; i++)
			{
				if (first == euccmb[i].ucp[0] && second == euccmb[i].ucp[1])
				{
					if (*dstsize <= di + 2)	//limit
					{
						AddNullEucJis2004(srcsize, si, dst, dstsize, di);
						return FALSE;
					}
					if (dst != nullptr)
					{
						*(dst + di) = euccmb[i].euc >> 8;
						*(dst + di + 1) = euccmb[i].euc & 0xFF;
					}
					di += 2;
					si++;
					exist = TRUE;
					break;
				}
			}

			if (!exist)
			{
				for (int i = 0; i < ROWNUM; i++)
				{
					for (int j = 0; j < CELLNUM; j++)
					{
						if (ucp == euc1[i][j])		// JIS X 0213 Plane 1
						{
							if (*dstsize <= di + 2)	//limit
							{
								AddNullEucJis2004(srcsize, si, dst, dstsize, di);
								return FALSE;
							}
							if (dst != nullptr)
							{
								*(dst + di) = (CHAR)((UCHAR)(ejs + i) + (UCHAR)ejd);
								*(dst + di + 1) = (CHAR)((UCHAR)(ejs + j) + (UCHAR)ejd);
							}
							di += 2;
							if (ucp != first)	//surrogate pair
							{
								si++;
							}
							exist = TRUE;
							break;
						}
						else if (euc2i[i] != 0 && euc2i[i] <= ROW2NUM &&
							ucp == euc2[euc2i[i] - 1][j])	// JIS X 0213 Plane 2
						{
							if (*dstsize <= di + 3)	//limit
							{
								AddNullEucJis2004(srcsize, si, dst, dstsize, di);
								return FALSE;
							}
							if (dst != nullptr)
							{
								*(dst + di) = ss3;
								*(dst + di + 1) = (CHAR)((UCHAR)(ejs + i) + (UCHAR)ejd);
								*(dst + di + 2) = (CHAR)((UCHAR)(ejs + j) + (UCHAR)ejd);
							}
							di += 3;
							if (ucp != first)	//surrogate pair
							{
								si++;
							}
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

			if (!exist)
			{
				for (int i = 0; i < ANKNUM; i++)
				{
					if (ucp == eucK[i])	//JIS X 0201 halfwidth katakana
					{
						if (*dstsize <= di + 2)	//limit
						{
							AddNullEucJis2004(srcsize, si, dst, dstsize, di);
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
				AddNullEucJis2004(srcsize, si, dst, dstsize, di);
				return FALSE;
			}
		}
	}

	AddNullEucJis2004(srcsize, si, dst, dstsize, di);
	return TRUE;
}

std::string wstring_to_eucjis2004_string(const std::wstring &s)
{
	std::string ret;
	size_t len;

	BOOL b = WideCharToEucJis2004(s.c_str(), nullptr, nullptr, &len);
	if (b && len > 0)
	{
		try
		{
			LPSTR euc = new CHAR[len];
			WideCharToEucJis2004(s.c_str(), nullptr, euc, &len);
			ret = euc;
			delete[] euc;
		}
		catch (...)
		{
		}
	}

	return ret;
}

std::wstring eucjis2004_string_to_wstring(const std::string &s)
{
	std::wstring ret;
	size_t len;

	BOOL b = EucJis2004ToWideChar(s.c_str(), nullptr, nullptr, &len);
	if (b && len > 0)
	{
		try
		{
			LPWSTR wcs = new WCHAR[len];
			EucJis2004ToWideChar(s.c_str(), nullptr, wcs, &len);
			ret = wcs;
			delete[] wcs;
		}
		catch (...)
		{
		}
	}

	return ret;
}
