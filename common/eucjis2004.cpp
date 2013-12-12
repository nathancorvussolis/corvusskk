
#include "eucjis2004table.h"

/* EUC-JIS-2004
    0x00..    0x7F ASCII
  0xA1**..  0xFE** JIS X 0213 plane1
  0x8E**..  0x8E** HALFWIDTH KATAKANA
0x8FA1**..0x8FFE** JIS X 0213 plane2 (+ JIS X 0212)
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

	if(first == NULL || second == NULL)
	{
		return 0;
	}

	*first = L'\0';
	*second = L'\0';
	
	if(ucp < SURROGATEPAIR_UCPMIN)
	{
		*first = (WCHAR)ucp;
		*second = L'\0';
		ret = 1;
	}
	else if(ucp <= SURROGATEPAIR_UCPMAX)  	//surrogate pair
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
	CONST CHAR mask = 0x7F;
	CONST CHAR ss2 = 0x0E;
	CONST CHAR ss3 = 0x0F;
	CONST CHAR ejs = 0x21;
	CONST CHAR eje = 0x7E;
	CHAR ss, ej[2];
	size_t srcused = 0;
	USHORT euc;
	int i;

	if(src == NULL || srcsize == 0 || ucp1 == NULL || ucp2 == NULL)
	{
		return 0;
	}

	*ucp1 = 0;
	*ucp2 = 0;

	if(src[0] >= 0x00 && src[0] <= 0x7F)	//ASCII
	{
		*ucp1 = src[0];
		*ucp2 = 0;
		srcused = 1;
	}
	else
	{
		ss = src[0] & mask;
		switch(ss)
		{
		case ss3:	// JIS X 0213 Plane 2
			if(srcsize < 3)
			{
				break;
			}

			ej[0] = src[1] & mask;
			ej[1] = src[2] & mask;

			if((ej[0] >= ejs && ej[0] <= eje) && (ej[1] >= ejs && ej[1] <= eje))
			{
				*ucp1 = 0;
				if(euc2i[ej[0] - ejs] != 0)
				{
					*ucp1 = euc2[euc2i[ej[0] - ejs] - 1][ej[1] - ejs];
				}
				*ucp2 = 0;
				srcused = 3;
			}
			break;

		case ss2:	//JIS X 0201 halfwidth katakana
			if(srcsize < 2)
			{
				break;
			}

			ej[0] = src[1] & mask;

			if(ej[0] >= ejs && ej[0] <= eje)
			{
				*ucp1 = eucK[ej[0] - ejs];
				*ucp2 = 0;
				srcused = 2;
			}
			break;

		default:	// JIS X 0213 Plane 1
			if(srcsize < 2)
			{
				break;
			}

			ej[0] = src[0] & mask;
			ej[1] = src[1] & mask;

			if((ej[0] >= ejs && ej[0] <= eje) && (ej[1] >= ejs && ej[1] <= eje))
			{
				euc = ((USHORT)ej[0] << 8) | (USHORT)ej[1] | 0x8080;

				//結合文字
				for(i=0; i<CMBCHARNUM; i++)
				{
					if(euccmb[i].euc == euc)
					{
						*ucp1 = euccmb[i].ucp[0];
						*ucp2 = euccmb[i].ucp[1];
						srcused = 2;
						break;
					}
				}

				if(srcused != 0)
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

// EUC-JIS-2004をUTF-16へ変換

BOOL EucJis2004ToWideChar(LPCSTR src, size_t *srcsize, LPWSTR dst, size_t *dstsize)
{
	size_t i, si, di = 0, ss = -1;
	UCSCHAR ucp[2];
	WCHAR utf16[2][2];
	size_t utf16num[2];

	if(src == NULL || dstsize == NULL)
	{
		return FALSE;
	}

	if(srcsize != NULL)
	{
		ss = *srcsize;
	}

	for(si = 0; ; si++)
	{
		if((ss <= si) || (*(src + si) == 0x00))
		{
			break;
		}

		// EUC-JIS-2004からUnicode Code Pointへ変換
		i = EucJis2004ToUcp(src + si, ss - si, &ucp[0], &ucp[1]);
		if((ucp[0] == 0) || (i == 0))
		{
			if(srcsize != NULL)
			{
				*srcsize = si;
			}
			*dstsize = di + 1;
			if(dst != NULL)
			{
				*(dst + di) = L'\0';
			}
			return FALSE;
		}
		si += i - 1;

		// Unicode Code PointからUTF-16へ変換
		for(i = 0; i < 2; i++)
		{
			utf16num[i] = 0;
			if(ucp[i] != 0)
			{
				utf16num[i] = UcpToWideChar(ucp[i], &utf16[i][0], &utf16[i][1]);
			}
		}

		if(*dstsize <= di + utf16num[0] + utf16num[1])	//limit
		{
			if(srcsize != NULL)
			{
				*srcsize = si;
			}
			*dstsize = di + 1;
			if(dst != NULL)
			{
				*(dst + di) = L'\0';
			}
			return FALSE;
		}

		for(i = 0; i < 2; i++)
		{
			if(dst != NULL)
			{
				wmemcpy_s(dst + di, 2, utf16[i], utf16num[i]);
			}
			di += utf16num[i];
		}
	}

	if(srcsize != NULL)
	{
		*srcsize = si;
	}
	*dstsize = di + 1;
	if(dst != NULL)
	{
		*(dst + di) = L'\0';
	}
	return TRUE;
}

// UTF-16をEUC-JIS-2004へ変換

BOOL WideCharToEucJis2004(LPCWSTR src, size_t *srcsize, LPSTR dst, size_t *dstsize)
{
	CONST CHAR mask = 0x7F;
	CONST CHAR ejs = 0x21;
	CONST CHAR ss2 = 0x0E;
	CONST CHAR ss3 = 0x0F;
	size_t si, di = 0, ss = -1;
	CHAR i, j;
	WCHAR first, second;
	UCSCHAR ucp;
	BOOL exist;
	
	if(src == NULL || dstsize == NULL)
	{
		return FALSE;
	}

	if(srcsize != NULL)
	{
		ss = *srcsize;
	}

	for(si = 0; ; si++)
	{
		if((ss <= si) || (*(src + si) == 0x0000))
		{
			break;
		}

		if(*(src + si) <= 0x007F)	//ASCII
		{
			if(*dstsize <= di + 1)	//limit
			{
				if(srcsize != NULL)
				{
					*srcsize = si;
				}
				*dstsize = di + 1;
				if(dst != NULL)
				{
					*(dst + di) = 0;
				}
				return FALSE;
			}
			if(dst != NULL)
			{
				*(dst + di) = (CHAR)*(src + si);
			}
			++di;
		}
		else
		{
			exist = FALSE;

			first = *(src + si);
			if(si + 1 < ss)
			{
				second = *(src + si + 1);
			}
			else
			{
				second = 0;
			}

			if((first >= SURROGATEPAIR_HIGH_MASK && first <= (SURROGATEPAIR_HIGH_MASK | SURROGATEPAIR_SEPMASK)) &&
				(second >= SURROGATEPAIR_LOW_MASK && second <= (SURROGATEPAIR_LOW_MASK | SURROGATEPAIR_SEPMASK)))
			{
				ucp = SURROGATEPAIR_UCPMIN +
					(((UCSCHAR)first & SURROGATEPAIR_SEPMASK) << SURROGATEPAIR_SEPBIT) |
					((UCSCHAR)second & SURROGATEPAIR_SEPMASK);
			}
			else
			{
				ucp = first;
			}

			//結合文字
			for(i = 0; i < CMBCHARNUM; i++)
			{
				if(first == euccmb[i].ucp[0] && second == euccmb[i].ucp[1])
				{
					if(*dstsize <= di + 2)	//limit
					{
						if(srcsize != NULL)
						{
							*srcsize = si;
						}
						*dstsize = di + 1;
						if(dst != NULL)
						{
							*(dst + di) = 0;
						}
						return FALSE;
					}
					if(dst != NULL)
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

			if(!exist)
			{
				for(i = 0; i < ROWNUM; i++)
				{
					for(j = 0; j < CELLNUM; j++)
					{
						if(ucp == euc1[i][j])		// JIS X 0213 Plane 1
						{
							if(*dstsize <= di + 2)	//limit
							{
								if(srcsize != NULL)
								{
									*srcsize = si;
								}
								*dstsize = di + 1;
								if(dst != NULL)
								{
									*(dst + di) = 0;
								}
								return FALSE;
							}
							if(dst != NULL)
							{
								*(dst + di) = (ejs + i) | ~mask;
								*(dst + di + 1) = (ejs + j) | ~mask;
							}
							di += 2;
							if(ucp != first)	//surrogate pair
							{
								si++;
							}
							exist = TRUE;
							break;
						}
						else if(euc2i[i] != 0 && ucp == euc2[euc2i[i] - 1][j])	// JIS X 0213 Plane 2
						{
							if(*dstsize <= di + 3)	//limit
							{
								if(srcsize != NULL)
								{
									*srcsize = si;
								}
								*dstsize = di + 1;
								if(dst != NULL)
								{
									*(dst + di) = 0;
								}
								return FALSE;
							}
							if(dst != NULL)
							{
								*(dst + di) = ss3 | ~mask;
								*(dst + di + 1) = (ejs + i) | ~mask;
								*(dst + di + 2) = (ejs + j) | ~mask;
							}
							di += 3;
							if(ucp != first)	//surrogate pair
							{
								si++;
							}
							exist = TRUE;
							break;
						}
					}

					if(exist)
					{
						break;
					}
				}
			}

			if(!exist)
			{
				for(i = 0; i < ANKNUM; i++)
				{
					if(ucp == eucK[i])	//JIS X 0201 halfwidth katakana
					{
						if(*dstsize <= di + 2)	//limit
						{
							if(srcsize != NULL)
							{
								*srcsize = si;
							}
							*dstsize = di + 1;
							if(dst != NULL)
							{
								*(dst + di) = 0;
							}
							return FALSE;
						}
						if(dst != NULL)
						{
							*(dst + di) = ss2 | ~mask;
							*(dst + di + 1) = (ejs + i) | ~mask;
						}
						di += 2;
						exist = TRUE;
						break;
					}
				}
			}

			if(!exist)
			{
				if(srcsize != NULL)
				{
					*srcsize = si;
				}
				*dstsize = di + 1;
				if(dst != NULL)
				{
					*(dst + di) = 0;
				}
				return FALSE;
			}
		}
	}
	
	if(srcsize != NULL)
	{
		*srcsize = si;
	}
	*dstsize = di + 1;
	if(dst != NULL)
	{
		*(dst + di) = 0;
	}
	return TRUE;
}
