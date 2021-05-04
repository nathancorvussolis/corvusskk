#pragma once

#define CMBCHARNUM	25	// Unicode結合文字
#define ROWNUM		94	// 区
#define CELLNUM		94	// 点
#define ANKNUM		94	// JIS X 0201 片仮名
#define ROW2NUM		26	// JIS X 0213 第二面 区数
#define CMPEUCNUM	2	// Unicode -> EUC-JIS-2004 互換性

// Unicode結合文字
typedef struct {
	USHORT euc;
	UCSCHAR ucp[2];
} EUCCMB;

// Unicode互換性
typedef struct {
	USHORT euc;
	UCSCHAR ucp;
} EUCCMP;

// 変換テーブル

// Unicode結合文字
extern const EUCCMB euccmb[CMBCHARNUM];
// JIS X 0213 第一面
extern const UCSCHAR euc1[ROWNUM][CELLNUM];
// JIS X 0201 片仮名
extern const UCSCHAR eucK[ANKNUM];
// JIS X 0213 第二面
extern const BYTE euc2i[ROWNUM];
// JIS X 0213 第二面
extern const UCSCHAR euc2[ROW2NUM][CELLNUM];
// Unicode -> EUC-JIS-2004 互換性
extern const EUCCMP euccmp[CMPEUCNUM];
