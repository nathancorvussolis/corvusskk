
#ifndef CONFIGXML_H
#define CONFIGXML_H

#include <string>
#include <vector>
#include <map>

#include <Windows.h>
#include <xmllite.h>
#include <Shlwapi.h>

#pragma comment(lib, "xmllite.lib")
#pragma comment(lib, "shlwapi.lib")

#define NOT_S_OK not_s_ok
#define EXIT_NOT_S_OK(hr) if(S_OK != (hr)) goto NOT_S_OK

typedef std::pair<std::wstring, std::wstring> APPDATAXMLATTR;
typedef std::vector<APPDATAXMLATTR> APPDATAXMLROW;
typedef std::vector<APPDATAXMLROW> APPDATAXMLLIST;

typedef std::pair<std::wstring, APPDATAXMLLIST> APPDATAXMLDICPAIR;
typedef std::map<std::wstring, APPDATAXMLLIST> APPDATAXMLDIC;

HRESULT ReadDicList(LPCWSTR path, LPCWSTR section, APPDATAXMLDIC &list, ULONGLONG pos = 0);
HRESULT ReadList(LPCWSTR path, LPCWSTR section, APPDATAXMLLIST &list);
HRESULT ReadValue(LPCWSTR path, LPCWSTR section, LPCWSTR key, std::wstring &strxmlval, LPCWSTR defval = L"");

HRESULT WriterInit(LPCWSTR path, IXmlWriter **ppWriter, IStream **ppFileStream, BOOL indent = TRUE);
HRESULT WriterFinal(IXmlWriter **ppWriter, IStream **ppFileStream);

HRESULT WriterNewLine(IXmlWriter *pWriter);
HRESULT WriterStartElement(IXmlWriter *pWriter, LPCWSTR element);
HRESULT WriterEndElement(IXmlWriter *pWriter);
HRESULT WriterAttribute(IXmlWriter *pWriter, LPCWSTR name, LPCWSTR value);

HRESULT WriterStartSection(IXmlWriter *pWriter, LPCWSTR name);
HRESULT WriterEndSection(IXmlWriter *pWriter);
HRESULT WriterKey(IXmlWriter *pWriter, LPCWSTR key, LPCWSTR value);
HRESULT WriterRow(IXmlWriter *pWriter, const APPDATAXMLROW &row);
HRESULT WriterList(IXmlWriter *pWriter, const APPDATAXMLLIST &list, BOOL newline = FALSE);

//tag
extern LPCWSTR TagRoot;
extern LPCWSTR TagSection;
extern LPCWSTR TagKey;
extern LPCWSTR TagEntry;
extern LPCWSTR TagList;
extern LPCWSTR TagRow;

//attribute
extern LPCWSTR AttributeName;
extern LPCWSTR AttributeValue;
extern LPCWSTR AttributeCandidate;
extern LPCWSTR AttributeAnnotation;
extern LPCWSTR AttributeKey;
extern LPCWSTR AttributePath;
extern LPCWSTR AttributeVKey;
extern LPCWSTR AttributeMKey;
extern LPCWSTR AttributeCPStart;
extern LPCWSTR AttributeCPAlter;
extern LPCWSTR AttributeCPOkuri;
extern LPCWSTR AttributeRoman;
extern LPCWSTR AttributeHiragana;
extern LPCWSTR AttributeKatakana;
extern LPCWSTR AttributeKatakanaAnk;
extern LPCWSTR AttributeSpOp;
extern LPCWSTR AttributeLatin;
extern LPCWSTR AttributeJLatin;

//section
extern LPCWSTR SectionFont;
//keys
extern LPCWSTR FontName;
extern LPCWSTR FontSize;
extern LPCWSTR FontWeight;
extern LPCWSTR FontItalic;

//section
extern LPCWSTR SectionBehavior;
//keys
extern LPCWSTR MaxWidth;
extern LPCWSTR UntilCandList;
extern LPCWSTR DispCandNo;
extern LPCWSTR Annotation;
extern LPCWSTR AnnotatLst;
extern LPCWSTR NoModeMark;
extern LPCWSTR NoOkuriConv;
extern LPCWSTR DelOkuriCncl;
extern LPCWSTR BackIncEnter;
extern LPCWSTR AddCandKtkn;

//section
extern LPCWSTR SectionDictionary;

//section
extern LPCWSTR SectionServer;
//keys
extern LPCWSTR Serv;
extern LPCWSTR Host;
extern LPCWSTR Port;
extern LPCWSTR TimeOut;

//section
extern LPCWSTR SectionSelKey;

//section
extern LPCWSTR SectionPreservedKey;

//section
extern LPCWSTR SectionKeyMap;
//section
extern LPCWSTR SectionVKeyMap;
//keys
extern LPCWSTR KeyMapKana;
extern LPCWSTR KeyMapConvChar;
extern LPCWSTR KeyMapJLatin;
extern LPCWSTR KeyMapAscii;
extern LPCWSTR KeyMapJMode;
extern LPCWSTR KeyMapAbbrev;
extern LPCWSTR KeyMapAffix;
extern LPCWSTR KeyMapNextCand;
extern LPCWSTR KeyMapPrevCand;
extern LPCWSTR KeyMapPurgeDic;
extern LPCWSTR KeyMapNextComp;
extern LPCWSTR KeyMapPrevComp;
extern LPCWSTR KeyMapConvPoint;
extern LPCWSTR KeyMapDirect;
extern LPCWSTR KeyMapEnter;
extern LPCWSTR KeyMapCancel;
extern LPCWSTR KeyMapBack;
extern LPCWSTR KeyMapDelete;
extern LPCWSTR KeyMapVoid;
extern LPCWSTR KeyMapLeft;
extern LPCWSTR KeyMapUp;
extern LPCWSTR KeyMapRight;
extern LPCWSTR KeyMapDown;
extern LPCWSTR KeyMapPaste;

//section
extern LPCWSTR SectionConvPoint;

//section
extern LPCWSTR SectionKana;

//section
extern LPCWSTR SectionJLatin;

//section
extern LPCWSTR SectionComplement;

#endif //CONFIGXML_H
