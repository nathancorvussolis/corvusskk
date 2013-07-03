
#ifndef CONFIGXML_H
#define CONFIGXML_H

#define NOT_S_OK not_s_ok
#define EXIT_NOT_S_OK(hr) if((hr) != S_OK) goto NOT_S_OK

typedef std::pair<std::wstring, std::wstring> APPDATAXMLATTR;
typedef std::vector<APPDATAXMLATTR> APPDATAXMLROW;
typedef std::vector<APPDATAXMLROW> APPDATAXMLLIST;

HRESULT CreateStreamReader(LPCWSTR path, IXmlReader **ppReader, IStream **ppFileStream);
void CloseStreamReader(IXmlReader *pReader, IStream *pFileStream);

HRESULT ReadList(LPCWSTR path, LPCWSTR section, APPDATAXMLLIST &list);
HRESULT ReadValue(LPCWSTR path, LPCWSTR section, LPCWSTR key, std::wstring &strxmlval, LPCWSTR defval = L"");

HRESULT CreateStreamWriter(LPCWSTR path, IXmlWriter **ppWriter, IStream **ppFileStream);
void CloseStreamWriter(IXmlWriter *pWriter, IStream *pFileStream);

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
extern LPCWSTR ValueFontName;
extern LPCWSTR ValueFontSize;
extern LPCWSTR ValueFontWeight;
extern LPCWSTR ValueFontItalic;

//section
extern LPCWSTR SectionBehavior;
//keys
extern LPCWSTR ValueMaxWidth;
extern LPCWSTR ValueColorBG;
extern LPCWSTR ValueColorFR;
extern LPCWSTR ValueColorSE;
extern LPCWSTR ValueColorCO;
extern LPCWSTR ValueColorCA;
extern LPCWSTR ValueColorSC;
extern LPCWSTR ValueColorAN;
extern LPCWSTR ValueColorNO;
extern LPCWSTR ValueUntilCandList;
extern LPCWSTR ValueDispCandNo;
extern LPCWSTR ValueAnnotation;
extern LPCWSTR ValueAnnotatLst;
extern LPCWSTR ValueNoModeMark;
extern LPCWSTR ValueNoOkuriConv;
extern LPCWSTR ValueDelOkuriCncl;
extern LPCWSTR ValueBackIncEnter;
extern LPCWSTR ValueAddCandKtkn;
extern LPCWSTR ValueShowModeImm;

//section
extern LPCWSTR SectionDictionary;

//section
extern LPCWSTR SectionServer;
//keys
extern LPCWSTR ValueServerServ;
extern LPCWSTR ValueServerHost;
extern LPCWSTR ValueServerPort;
extern LPCWSTR ValueServerEncoding;
extern LPCWSTR ValueServerTimeOut;

//section
extern LPCWSTR SectionSelKey;

//section
extern LPCWSTR SectionPreservedKey;

//section
extern LPCWSTR SectionKeyMap;
//section
extern LPCWSTR SectionVKeyMap;
//keys
extern LPCWSTR ValueKeyMapKana;
extern LPCWSTR ValueKeyMapConvChar;
extern LPCWSTR ValueKeyMapJLatin;
extern LPCWSTR ValueKeyMapAscii;
extern LPCWSTR ValueKeyMapJMode;
extern LPCWSTR ValueKeyMapAbbrev;
extern LPCWSTR ValueKeyMapAffix;
extern LPCWSTR ValueKeyMapNextCand;
extern LPCWSTR ValueKeyMapPrevCand;
extern LPCWSTR ValueKeyMapPurgeDic;
extern LPCWSTR ValueKeyMapNextComp;
extern LPCWSTR ValueKeyMapPrevComp;
extern LPCWSTR ValueKeyMapConvPoint;
extern LPCWSTR ValueKeyMapDirect;
extern LPCWSTR ValueKeyMapEnter;
extern LPCWSTR ValueKeyMapCancel;
extern LPCWSTR ValueKeyMapBack;
extern LPCWSTR ValueKeyMapDelete;
extern LPCWSTR ValueKeyMapVoid;
extern LPCWSTR ValueKeyMapLeft;
extern LPCWSTR ValueKeyMapUp;
extern LPCWSTR ValueKeyMapRight;
extern LPCWSTR ValueKeyMapDown;
extern LPCWSTR ValueKeyMapPaste;

//section
extern LPCWSTR SectionConvPoint;

//section
extern LPCWSTR SectionKana;

//section
extern LPCWSTR SectionJLatin;

#endif //CONFIGXML_H
