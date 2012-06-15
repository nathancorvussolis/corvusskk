
#ifndef EUCJIS2004_H
#define EUCJIS2004_H

size_t UcpToWideChar(UCSCHAR ucp, PWCHAR first, PWCHAR second);
size_t EucJis2004ToUcp(LPCSTR src, size_t srcsize, PUCSCHAR ucp1, PUCSCHAR ucp2);
BOOL EucJis2004ToWideChar(LPCSTR src, size_t *srcsize, LPWSTR dst, size_t *dstsize);

BOOL WideCharToEucJis2004(LPCWSTR src, size_t *srcsize, LPSTR dst, size_t *dstsize);

#endif //EUCJIS2004_H
