
#ifndef RESOURCE_H
#define RESOURCE_H

#ifndef IDC_STATIC
#define IDC_STATIC (-1)
#endif

#define IDD_DIALOG_DICTIONARY                   110
#define IDD_DIALOG_SKK_DIC_ADD_URL				111
#define IDD_DIALOG_BEHAVIOR1                    112
#define IDD_DIALOG_BEHAVIOR2                    113
#define IDD_DIALOG_DISPLAY                      120
#define IDD_DIALOG_DISPLAYATTR1                 121
#define IDD_DIALOG_DISPLAYATTR2                 122
#define IDD_DIALOG_SELKEY                       131
#define IDD_DIALOG_PRSRVKEY                     132
#define IDD_DIALOG_KEYMAP1                      133
#define IDD_DIALOG_KEYMAP2                      134
#define IDD_DIALOG_CONVPOINT                    141
#define IDD_DIALOG_KANATBL                      142
#define IDD_DIALOG_JLATTBL                      143

#define IDC_LIST_SKK_DIC                        1001
#define IDC_BUTTON_SKK_DIC_ADD_FILE             1002
#define IDC_BUTTON_SKK_DIC_ADD_URL              1003
#define IDC_BUTTON_SKK_DIC_DEL                  1004
#define IDC_BUTTON_SKK_DIC_UP                   1005
#define IDC_BUTTON_SKK_DIC_DOWN                 1006
#define IDC_BUTTON_SKK_DIC_MAKE                 1007
#define IDD_DIALOG_SKK_DIC_MAKE                 1021
#define IDC_PROGRESS_DIC_MAKE                   1022
#define IDC_BUTTON_ABORT_DIC_MAKE               1023
#define IDC_CHECKBOX_SKKSRV                     1051
#define IDC_RADIO_EUC                           1052
#define IDC_RADIO_UTF8                          1053
#define IDC_EDIT_SKKSRV_HOST                    1054
#define IDC_EDIT_SKKSRV_PORT                    1055
#define IDC_EDIT_SKKSRV_TIMEOUT                 1056

#define IDC_EDIT_SKK_DIC_URL					1101

#define IDC_CHECKBOX_DEFAULTMODE                1201
#define IDC_RADIO_DEFMODEHIRA                   1202
#define IDC_RADIO_DEFMODEASCII                  1203
#define IDC_CHECKBOX_BEGINCVOKURI               1204
#define IDC_CHECKBOX_PRECEDEOKURI               1205
#define IDC_CHECKBOX_SHIFTNNOKURI               1206
#define IDC_CHECKBOX_SRCHALLOKURI               1207
#define IDC_CHECKBOX_DELCVPOSCNCL               1208
#define IDC_CHECKBOX_DELOKURICNCL               1209
#define IDC_CHECKBOX_BACKINCENTER               1210
#define IDC_CHECKBOX_ADDCANDKTKN                1211

#define IDC_COMBO_COMPMULTINUM                  1301
#define IDC_CHECKBOX_STACOMPMULTI               1302
#define IDC_CHECKBOX_DYNAMINCOMP                1303
#define IDC_CHECKBOX_DYNCOMPMULTI               1304
#define IDC_CHECKBOX_COMPUSERDIC                1305

#define IDC_EDIT_FONTNAME                       2001
#define IDC_EDIT_FONTPOINT                      2002
#define IDC_BUTTON_CHOOSEFONT                   2003
#define IDC_EDIT_MAXWIDTH                       2004
#define IDC_COL_BG                              2011
#define IDC_COL_FR                              2012
#define IDC_COL_SE                              2013
#define IDC_COL_CO                              2014
#define IDC_COL_CA                              2015
#define IDC_COL_SC                              2016
#define IDC_COL_AN                              2017
#define IDC_COL_NO                              2018
#define IDC_RADIO_API_GDI                       2021
#define IDC_RADIO_API_D2D                       2022
#define IDC_CHECKBOX_COLOR_FONT                 2023
#define IDC_COMBO_UNTILCANDLIST                 2031
#define IDC_CHECKBOX_DISPCANDNO                 2032
#define IDC_CHECKBOX_VERTICALCAND               2033
#define IDC_CHECKBOX_ANNOTATION                 2034
#define IDC_RADIO_ANNOTATALL                    2035
#define IDC_RADIO_ANNOTATLST                    2036
#define IDC_CHECKBOX_SHOWMODEINL                2037
#define IDC_EDIT_SHOWMODESEC                    2038
#define IDC_CHECKBOX_SHOWMODEMARK               2039
#define IDC_CHECKBOX_SHOWROMAN                  2040

#define IDC_CHECKBOX_SERIES_MARK                2101
#define IDC_CHECKBOX_SERIES_TEXT                2102
#define IDC_CHECKBOX_SERIES_OKURI               2103
#define IDC_CHECKBOX_SERIES_ANNOT               2104
#define IDC_RADIO_FG_STD_MARK                   2106
#define IDC_RADIO_FG_STD_TEXT                   2107
#define IDC_RADIO_FG_STD_OKURI                  2108
#define IDC_RADIO_FG_STD_ANNOT                  2109
#define IDC_RADIO_FG_SEL_MARK                   2111
#define IDC_RADIO_FG_SEL_TEXT                   2112
#define IDC_RADIO_FG_SEL_OKURI                  2113
#define IDC_RADIO_FG_SEL_ANNOT                  2114
#define IDC_COL_FG_MARK                         2116
#define IDC_COL_FG_TEXT                         2117
#define IDC_COL_FG_OKURI                        2118
#define IDC_COL_FG_ANNOT                        2119
#define IDC_RADIO_BG_STD_MARK                   2121
#define IDC_RADIO_BG_STD_TEXT                   2122
#define IDC_RADIO_BG_STD_OKURI                  2123
#define IDC_RADIO_BG_STD_ANNOT                  2124
#define IDC_RADIO_BG_SEL_MARK                   2126
#define IDC_RADIO_BG_SEL_TEXT                   2127
#define IDC_RADIO_BG_SEL_OKURI                  2128
#define IDC_RADIO_BG_SEL_ANNOT                  2129
#define IDC_COL_BG_MARK                         2131
#define IDC_COL_BG_TEXT                         2132
#define IDC_COL_BG_OKURI                        2133
#define IDC_COL_BG_ANNOT                        2134
#define IDC_COMBO_UL_ATTR_MARK                  2136
#define IDC_COMBO_UL_ATTR_TEXT                  2137
#define IDC_COMBO_UL_ATTR_OKURI                 2138
#define IDC_COMBO_UL_ATTR_ANNOT                 2139
#define IDC_CHECKBOX_UL_BOLD_MARK               2141
#define IDC_CHECKBOX_UL_BOLD_TEXT               2142
#define IDC_CHECKBOX_UL_BOLD_OKURI              2143
#define IDC_CHECKBOX_UL_BOLD_ANNOT              2144
#define IDC_RADIO_UL_STD_MARK                   2146
#define IDC_RADIO_UL_STD_TEXT                   2147
#define IDC_RADIO_UL_STD_OKURI                  2148
#define IDC_RADIO_UL_STD_ANNOT                  2149
#define IDC_RADIO_UL_SEL_MARK                   2151
#define IDC_RADIO_UL_SEL_TEXT                   2152
#define IDC_RADIO_UL_SEL_OKURI                  2153
#define IDC_RADIO_UL_SEL_ANNOT                  2154
#define IDC_COL_UL_MARK                         2156
#define IDC_COL_UL_TEXT                         2157
#define IDC_COL_UL_OKURI                        2158
#define IDC_COL_UL_ANNOT                        2159
#define IDC_COMBO_ATTR_MARK                     2161
#define IDC_COMBO_ATTR_TEXT                     2162
#define IDC_COMBO_ATTR_OKURI                    2163
#define IDC_COMBO_ATTR_ANNOT                    2164

#define IDC_LIST_SELKEY                         3101
#define IDC_EDIT_SELKEY_DISP                    3102
#define IDC_EDIT_SELKEY_SPARE                   3103
#define IDC_BUTTON_SELKEY_W                     3104

#define IDC_LIST_PRSRVKEY_ON                    3201
#define IDC_LIST_PRSRVKEY_OFF                   3202
#define IDC_EDIT_PRSRVKEY_VKEY                  3203
#define IDC_CHECKBOX_PRSRVKEY_MKEY_ALT          3204
#define IDC_CHECKBOX_PRSRVKEY_MKEY_CTRL         3205
#define IDC_CHECKBOX_PRSRVKEY_MKEY_SHIFT        3206
#define IDC_RADIO_PRSRVKEY_ON                   3207
#define IDC_RADIO_PRSRVKEY_OFF                  3208
#define IDC_BUTTON_PRSRVKEY_UP                  3209
#define IDC_BUTTON_PRSRVKEY_DOWN                3210
#define IDC_BUTTON_PRSRVKEY_W                   3211
#define IDC_BUTTON_PRSRVKEY_D                   3212
#define IDC_EDIT_DISPVKEY                       3221

#define IDC_EDIT_KANA                           3301
#define IDC_EDIT_CONV_CHAR                      3302
#define IDC_EDIT_JLATIN                         3303
#define IDC_EDIT_ASCII                          3304
#define IDC_EDIT_JMODE                          3305
#define IDC_EDIT_ABBREV                         3306
#define IDC_EDIT_AFFIX                          3307
#define IDC_EDIT_NEXT_CAND                      3308
#define IDC_EDIT_PREV_CAND                      3309
#define IDC_EDIT_PURGE_DIC                      3310
#define IDC_EDIT_NEXT_COMP                      3311
#define IDC_EDIT_PREV_COMP                      3312
#define IDC_EDIT_HINT                           3313
#define IDC_EDIT_CONV_POINT                     3321
#define IDC_EDIT_DIRECT                         3322
#define IDC_EDIT_ENTER                          3323
#define IDC_EDIT_CANCEL                         3324
#define IDC_EDIT_BACK                           3325
#define IDC_EDIT_DELETE                         3326
#define IDC_EDIT_VOID                           3327
#define IDC_EDIT_LEFT                           3328
#define IDC_EDIT_UP                             3329
#define IDC_EDIT_RIGHT                          3330
#define IDC_EDIT_DOWN                           3331
#define IDC_EDIT_PASTE                          3332

#define IDC_LIST_CONVPOINT                      4101
#define IDC_EDIT_CONVPOINT_ST                   4102
#define IDC_EDIT_CONVPOINT_AL                   4103
#define IDC_EDIT_CONVPOINT_OK                   4104
#define IDC_BUTTON_CONVPOINT_UP                 4105
#define IDC_BUTTON_CONVPOINT_DOWN               4106
#define IDC_BUTTON_CONVPOINT_W                  4107
#define IDC_BUTTON_CONVPOINT_D                  4108

#define IDC_LIST_KANATBL                        4201
#define IDC_EDIT_KANATBL_R                      4202
#define IDC_EDIT_KANATBL_H                      4203
#define IDC_EDIT_KANATBL_K                      4204
#define IDC_EDIT_KANATBL_KA                     4205
#define IDC_CHECKBOX_KANATBL_SOKU               4206
#define IDC_CHECKBOX_KANATBL_WAIT               4207
#define IDC_BUTTON_KANATBL_UP                   4208
#define IDC_BUTTON_KANATBL_DOWN                 4209
#define IDC_BUTTON_KANATBL_W                    4210
#define IDC_BUTTON_KANATBL_D                    4211
#define IDC_BUTTON_LOADKANA                     4212
#define IDC_BUTTON_SAVEKANA                     4213

#define IDC_LIST_JLATTBL                        4301
#define IDC_EDIT_JLATTBL_A                      4302
#define IDC_EDIT_JLATTBL_J                      4303
#define IDC_BUTTON_JLATTBL_UP                   4304
#define IDC_BUTTON_JLATTBL_DOWN                 4305
#define IDC_BUTTON_JLATTBL_W                    4306
#define IDC_BUTTON_JLATTBL_D                    4307

#endif //RESOURCE_H
