#pragma once

#include "eucjis2004table.h"

#define ROW0212NUM		68	// JIS X 0212 区数
#define COLNUM			6	// JIS X 0208 有効区点要素数 (区数+2)/16
#define CMPEUCJPNUM		9	// Unicode -> EUC-JP 互換性

//変換テーブル

// JIS X 0208 有効区点
extern const USHORT jisx0208e[ROWNUM][COLNUM];
// JIS X 0212 インデックス
extern const BYTE euc0212i[ROWNUM];
// JIS X 0212
extern const UCSCHAR euc0212[ROW0212NUM][CELLNUM];
// Unicode -> EUC-JP 互換性
extern const EUCCMP eucjpcmp[CMPEUCJPNUM];
