#pragma once

#define CMBCHARNUM	25
#define ANKNUM		94
#define ROWNUM		94
#define CELLNUM		94
#define ROW2NUM		26

typedef struct {
	USHORT euc;
	UCSCHAR ucp[2];
} EUCCMB;

//変換テーブル
extern const EUCCMB euccmb[CMBCHARNUM];			//Unicode結合文字
extern const UCSCHAR euc1[ROWNUM][CELLNUM];		//JIS X 0213 第一面
extern const UCSCHAR eucK[ANKNUM];				//JIS X 0201
extern const BYTE euc2i[ROWNUM];				//JIS X 0213 第二面インデックス
extern const UCSCHAR euc2[ROW2NUM][CELLNUM];	//JIS X 0213 第二面
