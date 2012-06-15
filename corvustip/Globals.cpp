
#include "corvustip.h"

HINSTANCE g_hInst;

const WCHAR *TextServiceDesc = TEXTSERVICE_DESC;
const WCHAR *LangbarItemDesc = LANGBAR_ITEM_DESC;
const WCHAR *LangbarFuncDesc = LANGBAR_FUNC_DESC;

OSVERSIONINFO g_ovi;

#ifndef _DEBUG

// {EAEA0E29-AA1E-48ef-B2DF-46F4E24C6265}
const GUID c_clsidTextService = 
{ 0xeaea0e29, 0xaa1e, 0x48ef, { 0xb2, 0xdf, 0x46, 0xf4, 0xe2, 0x4c, 0x62, 0x65 } };

// {956F14B3-5310-4cef-9651-26710EB72F3A}
const GUID c_guidProfile = 
{ 0x956f14b3, 0x5310, 0x4cef, { 0x96, 0x51, 0x26, 0x71, 0x0e, 0xb7, 0x2f, 0x3a } };

// {812F0064-DEEF-443b-A4E2-6B29F0D9C5C4}
const GUID c_guidPreservedKeyOnOff = 
{ 0x812f0064, 0xdeef, 0x443b, { 0xa4, 0xe2, 0x6b, 0x29, 0xf0, 0xd9, 0xc5, 0xc4 } };

//{3E45E83F-C786-485f-8B66-B7EAD509F960}
const GUID c_guidLangBarItemButton = 
{ 0x3e45e83f, 0xc786, 0x485f, { 0x8b, 0x66, 0xb7, 0xea, 0xd5, 0x09, 0xf9, 0x60 } };

// {7FE29429-CB73-43f0-B481-BC220B68F2E3}
const GUID c_guidDisplayAttributeInput = 
{ 0x7fe29429, 0xcb73, 0x43f0, { 0xb4, 0x81, 0xbc, 0x22, 0x0b, 0x68, 0xf2, 0xe3 } };

// {EBFDDF5D-E34E-409e-8C93-9FE92DB32FB5}
const GUID c_guidDisplayAttributeConverted = 
{ 0xebfddf5d, 0xe34e, 0x409e, { 0x8c, 0x93, 0x9f, 0xe9, 0x2d, 0xb3, 0x2f, 0xb5 } };

// {3F821491-1536-4bc9-B4CA-080947B00F64}
const GUID c_guidCandidateListUIElement = 
{ 0x3f821491, 0x1536, 0x4bc9, { 0xb4, 0xca, 0x08, 0x09, 0x47, 0xb0, 0x0f, 0x64 } };

#else

// {4D97960C-1D59-4466-BEFE-4C1328D2550D}
const GUID c_clsidTextService = 
{ 0x4d97960c, 0x1d59, 0x4466, { 0xbe, 0xfe, 0x4c, 0x13, 0x28, 0xd2, 0x55, 0x0d } };

// {820E9894-024B-4bd1-98AF-3942B772CFF1}
const GUID c_guidProfile = 
{ 0x820e9894, 0x024b, 0x4bd1, { 0x98, 0xaf, 0x39, 0x42, 0xb7, 0x72, 0xcf, 0xf1 } };

// {D1930150-790A-437b-88B5-EB3E9FB9165F}
const GUID c_guidPreservedKeyOnOff = 
{ 0xd1930150, 0x790a, 0x437b, { 0x88, 0xb5, 0xeb, 0x3e, 0x9f, 0xb9, 0x16, 0x5f } };

// {F4BF0D3C-D4CE-456f-837E-FE6712C6A8C3}
const GUID c_guidLangBarItemButton = 
{ 0xf4bf0d3c, 0xd4ce, 0x456f, { 0x83, 0x7e, 0xfe, 0x67, 0x12, 0xc6, 0xa8, 0xc3 } };

// {6F99E3F1-36AC-4015-B334-211CFFCB3262}
const GUID c_guidDisplayAttributeInput = 
{ 0x6f99e3f1, 0x36ac, 0x4015, { 0xb3, 0x34, 0x21, 0x1c, 0xff, 0xcb, 0x32, 0x62 } };

// {6877D302-1C51-4ba4-9329-2F80B5E3A4E7}
const GUID c_guidDisplayAttributeConverted = 
{ 0x6877d302, 0x1c51, 0x4ba4, { 0x93, 0x29, 0x2f, 0x80, 0xb5, 0xe3, 0xa4, 0xe7 } };

// {25A6388F-D3CB-4866-A2C3-94E00970BF45}
const GUID c_guidCandidateListUIElement = 
{ 0x25a6388f, 0xd3cb, 0x4866, { 0xa2, 0xc3, 0x94, 0xe0, 0x09, 0x70, 0xbf, 0x45 } };

#endif

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeInput =
{
	{TF_CT_NONE, 0},			// TF_DA_COLOR crText;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crBk;
	TF_LS_DOT,					// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{ TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_INPUT				// TF_DA_ATTR_INFO bAttr;
};

const TF_DISPLAYATTRIBUTE c_daDisplayAttributeConverted =
{
	{TF_CT_NONE, 0},			// TF_DA_COLOR crText;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crBk;
	TF_LS_SOLID,				// TF_DA_LINESTYLE lsStyle;
	FALSE,						// BOOL fBoldLine;
	{TF_CT_NONE, 0},			// TF_DA_COLOR crLine;
	TF_ATTR_TARGET_CONVERTED	// TF_DA_ATTR_INFO bAttr;
};
