
#include "configxml.h"
#include "imcrvtip.h"

HINSTANCE g_hInst;

LPCWSTR TextServiceDesc = TEXTSERVICE_DESC;
LPCWSTR LangbarItemDesc = L"ver. " TEXTSERVICE_VER;

#ifndef _DEBUG

// {EAEA0E29-AA1E-48ef-B2DF-46F4E24C6265}
const GUID c_clsidTextService =
{0xeaea0e29, 0xaa1e, 0x48ef, {0xb2, 0xdf, 0x46, 0xf4, 0xe2, 0x4c, 0x62, 0x65}};

// {956F14B3-5310-4cef-9651-26710EB72F3A}
const GUID c_guidProfile =
{0x956f14b3, 0x5310, 0x4cef, {0x96, 0x51, 0x26, 0x71, 0x0e, 0xb7, 0x2f, 0x3a}};

// {812F0064-DEEF-443b-A4E2-6B29F0D9C5C4}
const GUID c_guidPreservedKeyOnOff =
{0x812f0064, 0xdeef, 0x443b, {0xa4, 0xe2, 0x6b, 0x29, 0xf0, 0xd9, 0xc5, 0xc4}};

// {3E45E83F-C786-485f-8B66-B7EAD509F960}
const GUID c_guidLangBarItemButton =
{0x3e45e83f, 0xc786, 0x485f, {0x8b, 0x66, 0xb7, 0xea, 0xd5, 0x09, 0xf9, 0x60}};

// {3F821491-1536-4bc9-B4CA-080947B00F64}
const GUID c_guidCandidateListUIElement =
{0x3f821491, 0x1536, 0x4bc9, {0xb4, 0xca, 0x08, 0x09, 0x47, 0xb0, 0x0f, 0x64}};

// {72A02805-47AF-4133-AFA6-BF6B20FB6723}
const GUID c_guidDisplayAttributeInputMark = 
{0x72a02805, 0x47af, 0x4133, {0xaf, 0xa6, 0xbf, 0x6b, 0x20, 0xfb, 0x67, 0x23}};

// {7FE29429-CB73-43f0-B481-BC220B68F2E3}
const GUID c_guidDisplayAttributeInputText =
{0x7fe29429, 0xcb73, 0x43f0, {0xb4, 0x81, 0xbc, 0x22, 0x0b, 0x68, 0xf2, 0xe3}};

// {22CF49AD-8D24-4DEA-B420-C7D8DB4D2A12}
const GUID c_guidDisplayAttributeInputOkuri =
{0x22cf49ad, 0x8d24, 0x4dea, {0xb4, 0x20, 0xc7, 0xd8, 0xdb, 0x4d, 0x2a, 0x12}};

// {3F45C819-C7A4-47D1-BB86-2F3E00C4729F}
const GUID c_guidDisplayAttributeConvMark = 
{0x3f45c819, 0xc7a4, 0x47d1, {0xbb, 0x86, 0x2f, 0x3e, 0x00, 0xc4, 0x72, 0x9f}};

// {EBFDDF5D-E34E-409e-8C93-9FE92DB32FB5}
const GUID c_guidDisplayAttributeConvText =
{0xebfddf5d, 0xe34e, 0x409e, {0x8c, 0x93, 0x9f, 0xe9, 0x2d, 0xb3, 0x2f, 0xb5}};

// {3C1D9F80-CB33-43CF-9F00-F38F397B671A}
const GUID c_guidDisplayAttributeConvOkuri =
{0x3c1d9f80, 0xcb33, 0x43cf, {0x9f, 0x00, 0xf3, 0x8f, 0x39, 0x7b, 0x67, 0x1a}};

// {ACEDB943-CFD1-4588-89DD-B80E5720CBC7}
const GUID c_guidDisplayAttributeConvAnnot =
{0xacedb943, 0xcfd1, 0x4588, {0x89, 0xdd, 0xb8, 0x0e, 0x57, 0x20, 0xcb, 0xc7}};

#else

// {4D97960C-1D59-4466-BEFE-4C1328D2550D}
const GUID c_clsidTextService =
{0x4d97960c, 0x1d59, 0x4466, {0xbe, 0xfe, 0x4c, 0x13, 0x28, 0xd2, 0x55, 0x0d}};

// {820E9894-024B-4bd1-98AF-3942B772CFF1}
const GUID c_guidProfile =
{0x820e9894, 0x024b, 0x4bd1, {0x98, 0xaf, 0x39, 0x42, 0xb7, 0x72, 0xcf, 0xf1}};

// {D1930150-790A-437b-88B5-EB3E9FB9165F}
const GUID c_guidPreservedKeyOnOff =
{0xd1930150, 0x790a, 0x437b, {0x88, 0xb5, 0xeb, 0x3e, 0x9f, 0xb9, 0x16, 0x5f}};

// {F4BF0D3C-D4CE-456f-837E-FE6712C6A8C3}
const GUID c_guidLangBarItemButton =
{0xf4bf0d3c, 0xd4ce, 0x456f, {0x83, 0x7e, 0xfe, 0x67, 0x12, 0xc6, 0xa8, 0xc3}};

// {25A6388F-D3CB-4866-A2C3-94E00970BF45}
const GUID c_guidCandidateListUIElement =
{0x25a6388f, 0xd3cb, 0x4866, {0xa2, 0xc3, 0x94, 0xe0, 0x09, 0x70, 0xbf, 0x45}};

// {CB22C53A-AD57-485A-A6CF-20390A0D5098}
const GUID c_guidDisplayAttributeInputMark = 
{0xcb22c53a, 0xad57, 0x485a, {0xa6, 0xcf, 0x20, 0x39, 0x0a, 0x0d, 0x50, 0x98}};

// {6F99E3F1-36AC-4015-B334-211CFFCB3262}
const GUID c_guidDisplayAttributeInputText =
{0x6f99e3f1, 0x36ac, 0x4015, {0xb3, 0x34, 0x21, 0x1c, 0xff, 0xcb, 0x32, 0x62}};

// {D2176C6C-8758-40C6-9612-5832FA315879}
const GUID c_guidDisplayAttributeInputOkuri =
{0xd2176c6c, 0x8758, 0x40c6, {0x96, 0x12, 0x58, 0x32, 0xfa, 0x31, 0x58, 0x79}};

// {B564E740-166D-45B1-AF44-7CCC7F75A807}
const GUID c_guidDisplayAttributeConvMark = 
{0xb564e740, 0x166d, 0x45b1, {0xaf, 0x44, 0x7c, 0xcc, 0x7f, 0x75, 0xa8, 0x07}};

// {6877D302-1C51-4ba4-9329-2F80B5E3A4E7}
const GUID c_guidDisplayAttributeConvText =
{0x6877d302, 0x1c51, 0x4ba4, {0x93, 0x29, 0x2f, 0x80, 0xb5, 0xe3, 0xa4, 0xe7}};

// {F99304F1-9F91-439E-8446-6FE0F8A98EDC}
const GUID c_guidDisplayAttributeConvOkuri =
{0xf99304f1, 0x9f91, 0x439e, {0x84, 0x46, 0x6f, 0xe0, 0xf8, 0xa9, 0x8e, 0xdc}};

// {C6040719-6FF3-4b92-A589-36E93BFD53EC}
const GUID c_guidDisplayAttributeConvAnnot =
{0xc6040719, 0x6ff3, 0x4b92, {0xa5, 0x89, 0x36, 0xe9, 0x3b, 0xfd, 0x53, 0xec}};

#endif

LPCWSTR markNo = L":";
LPCWSTR markAnnotation = L";";
LPCWSTR markCandEnd = L"\u3000";
LPCWSTR markCursor = L"|";
LPCWSTR markReg = L"登録";
LPCWSTR markRegL = L"[";
LPCWSTR markRegR = L"]";
LPCWSTR markRegKeyEnd = L"：";
LPCWSTR markSP = L"\x20";
LPCWSTR markNBSP = L"\u00A0";
LPCWSTR markMidashi = L"▽";
LPCWSTR markHenkan = L"▼";
LPCWSTR markOkuri = L"*";

const DISPLAYATTRIBUTE_INFO c_gdDisplayAttributeInfo[DISPLAYATTRIBUTE_INFO_NUM] =
{
	{
		ValueDisplayAttrInputMark, c_guidDisplayAttributeInputMark,
		c_daDisplayAttributeSeries[0], c_daDisplayAttributeInputMark
	},
	{
		ValueDisplayAttrInputText, c_guidDisplayAttributeInputText,
		c_daDisplayAttributeSeries[1], c_daDisplayAttributeInputText
	},
	{
		ValueDisplayAttrInputOkuri, c_guidDisplayAttributeInputOkuri,
		c_daDisplayAttributeSeries[2], c_daDisplayAttributeInputOkuri
	},
	{
		ValueDisplayAttrConvMark, c_guidDisplayAttributeConvMark,
		c_daDisplayAttributeSeries[3], c_daDisplayAttributeConvMark
	},
	{
		ValueDisplayAttrConvText, c_guidDisplayAttributeConvText,
		c_daDisplayAttributeSeries[4], c_daDisplayAttributeConvText
	},
	{
		ValueDisplayAttrConvOkuri, c_guidDisplayAttributeConvOkuri,
		c_daDisplayAttributeSeries[5], c_daDisplayAttributeConvOkuri
	},
	{
		ValueDisplayAttrConvAnnot, c_guidDisplayAttributeConvAnnot,
		c_daDisplayAttributeSeries[6], c_daDisplayAttributeConvAnnot
	}
};

// added in Windows 8 SDK
#if (_WIN32_WINNT < 0x0602)

const GUID GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT =
{0x13A016DF, 0x560B, 0x46CD, {0x94, 0x7A, 0x4C, 0x3A, 0xF1, 0xE0, 0xE3, 0x5D}};

const GUID GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT =
{0x25504FB4, 0x7BAB, 0x4BC1, {0x9C, 0x69, 0xCF, 0x81, 0x89, 0x0F, 0x0E, 0xF5}};

const GUID GUID_LBI_INPUTMODE =
{0x2C77A81E, 0x41CC, 0x4178, {0xA3, 0xA7, 0x5F, 0x8A, 0x98, 0x75, 0x68, 0xE6}};

const IID IID_ITfFnGetPreferredTouchKeyboardLayout =
{0x5F309A41, 0x590A, 0x4ACC, {0xA9, 0x7F, 0xD8, 0xEF, 0xFF, 0x13, 0xFD, 0xFC}};

#endif //(_WIN32_WINNT < 0x0602)
