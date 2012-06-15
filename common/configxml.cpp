
#include "configxml.h"

//section
LPCWSTR SectionFont = L"font";
//keys
LPCWSTR FontName = L"name";
LPCWSTR FontSize = L"size";
LPCWSTR FontWeight = L"weight";
LPCWSTR FontItalic = L"italic";

//section
LPCWSTR SectionBehavior = L"behavior";
//keys
LPCWSTR MaxWidth = L"maxwidth";
LPCWSTR VisualStyle = L"visualstyle";
LPCWSTR UntilCandList = L"untilcandlist";
LPCWSTR DispCandNo = L"dispcandno";
LPCWSTR Annotation = L"annotation";
LPCWSTR NoModeMark = L"nomodemark";
LPCWSTR NoOkuriConv = L"nookuriconv";
LPCWSTR DelOkuriCncl = L"delokuricncl";
LPCWSTR BackIncEnter = L"backincenter";
LPCWSTR AddCandKtkn = L"addcandktkn";

//section
LPCWSTR SectionDictionary = L"dictionary";

//section
LPCWSTR SectionServer = L"server";
//keys
LPCWSTR Serv = L"serv";
LPCWSTR Host = L"host";
LPCWSTR Port = L"port";
LPCWSTR TimeOut = L"timeout";

//section
LPCWSTR SectionSelKey = L"selkey";

//section
LPCWSTR SectionKeyMap = L"keymap";
//keys
LPCWSTR KeyMapKana = L"kana";
LPCWSTR KeyMapConvChar = L"convchar";
LPCWSTR KeyMapJLatin = L"jlatin";
LPCWSTR KeyMapAscii = L"ascii";
LPCWSTR KeyMapJMode = L"jmode";
LPCWSTR KeyMapAbbrev = L"abbrev";
LPCWSTR KeyMapAffix = L"affix";
LPCWSTR KeyMapDirect = L"direct";
LPCWSTR KeyMapNextCand = L"nextcand";
LPCWSTR KeyMapPrevCand = L"prevcand";
LPCWSTR KeyMapPurgeDic = L"purgedic";
LPCWSTR KeyMapNextComp = L"nextcomp";
LPCWSTR KeyMapPrevComp = L"prevcomp";
LPCWSTR KeyMapEnter = L"enter";
LPCWSTR KeyMapCancel = L"cancel";
LPCWSTR KeyMapBack = L"back";
LPCWSTR KeyMapDelete = L"delete";
LPCWSTR KeyMapVoid = L"void";
LPCWSTR KeyMapLeft = L"left";
LPCWSTR KeyMapUp = L"up";
LPCWSTR KeyMapRight = L"right";
LPCWSTR KeyMapDown = L"down";
LPCWSTR KeyMapPaste = L"paste";

//section
LPCWSTR SectionConvPoint = L"convpoint";

//section
LPCWSTR SectionKana = L"kana";

//section
LPCWSTR SectionJLatin = L"jlatin";

//section
LPCWSTR SectionComplement = L"complement";

//attribute
LPCWSTR AttributeCandidate = L"c";
LPCWSTR AttributeAnnotation = L"a";
LPCWSTR AttributeKey = L"key";
LPCWSTR AttributePath = L"path";
LPCWSTR AttributeCPStart = L"st";
LPCWSTR AttributeCPAlter = L"al";
LPCWSTR AttributeCPOkuri = L"ok";
LPCWSTR AttributeRoman = L"ro";
LPCWSTR AttributeHiragana = L"hi";
LPCWSTR AttributeKatakana = L"ka";
LPCWSTR AttributeKatakanaAnk = L"an";
LPCWSTR AttributeSoku = L"so";
LPCWSTR AttributeLatin = L"la";
LPCWSTR AttributeJLatin = L"jl";

static LPCWSTR pcwszRoot = L"skk";
static LPCWSTR pcwszSection = L"section";
static LPCWSTR pcwszName = L"name";
static LPCWSTR pcwszEntry = L"entry";
static LPCWSTR pcwszKey = L"key";
static LPCWSTR pcwszValue = L"value";
static LPCWSTR pcwszList = L"list";
static LPCWSTR pcwszRow = L"row";

#define NOT_S_OK not_s_ok
#define EXIT_NOT_S_OK(hr) if(S_OK != (hr)) goto NOT_S_OK

static void CloseStreamReader(IXmlReader *pReader, IStream *pFileStream)
{
	if(pReader != NULL)
	{
		pReader->Release();
	}
	if(pFileStream != NULL)
	{
		pFileStream->Release();
	}
}

static HRESULT CreateStreamReader(LPCWSTR path, IXmlReader **ppReader, IStream **ppFileStream)
{
	HRESULT hr;

	hr = CreateXmlReader(IID_IXmlReader, (LPVOID *)ppReader, NULL);
	EXIT_NOT_S_OK(hr);
	hr = SHCreateStreamOnFileW(path, STGM_READ, ppFileStream);
	EXIT_NOT_S_OK(hr);
	hr = (*ppReader)->SetInput(*ppFileStream);
	EXIT_NOT_S_OK(hr);
	goto end;

NOT_S_OK:
	CloseStreamReader(*ppReader, *ppFileStream);
end:
	return hr;
}

HRESULT ReadDicList(LPCWSTR path, LPCWSTR section, APPDATAXMLDIC &list, ULONGLONG pos)
{
	HRESULT hr;
	IXmlReader *pReader = NULL;
	IStream *pFileStream = NULL;

	XmlNodeType nodeType;
	LPCWSTR pwszLocalName;
	LPCWSTR pwszAttributeName;
	LPCWSTR pwszAttributeValue;
	int sequence = 0;
	LARGE_INTEGER li0;
	ULARGE_INTEGER uli1;

	APPDATAXMLDIC::iterator d_itr;
	APPDATAXMLDICPAIR pair;
	APPDATAXMLLIST::iterator l_itr;
	APPDATAXMLROW row;
	APPDATAXMLATTR attr;

	list.clear();

	hr = CreateStreamReader(path, &pReader, &pFileStream);
	EXIT_NOT_S_OK(hr);

	while(pReader->Read(&nodeType) == S_OK)
	{
		switch(nodeType)
		{
		case XmlNodeType_Element:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 0:
				if(wcscmp(pcwszRoot, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 1:
				if(wcscmp(pcwszSection, pwszLocalName) == 0)
				{
					sequence = 2;
				}
				break;
			case 3:
				if(wcscmp(pcwszEntry, pwszLocalName) == 0)
				{
					sequence = 4;
				}
				break;
			case 5:
				if(wcscmp(pcwszList, pwszLocalName) == 0)
				{
					sequence = 6;
				}
				break;
			case 6:
				if(wcscmp(pcwszRow, pwszLocalName) == 0)
				{
					sequence = 7;
					row.clear();
				}
				break;
			default:
				break;
			}

			for(hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
			{
				hr = pReader->GetLocalName(&pwszAttributeName, NULL);
				EXIT_NOT_S_OK(hr);
				hr = pReader->GetValue(&pwszAttributeValue, NULL);
				EXIT_NOT_S_OK(hr);

				switch(sequence)
				{
				case 2:
					if(wcscmp(pcwszSection, pwszLocalName) == 0 && wcscmp(pcwszName, pwszAttributeName) == 0 &&
						wcscmp(section, pwszAttributeValue) == 0)
					{
						sequence = 3;

						if(pos != 0)
						{
							li0.QuadPart = pos;
							hr = pFileStream->Seek(li0, STREAM_SEEK_SET, &uli1);
							EXIT_NOT_S_OK(hr);
							hr = pReader->SetInput(pFileStream);
							EXIT_NOT_S_OK(hr);
						}
					}
					break;
				case 4:
					if(wcscmp(pcwszEntry, pwszLocalName) == 0 && wcscmp(pcwszKey, pwszAttributeName) == 0)
					{
						pair.first = pwszAttributeValue;
						pair.second.clear();
						sequence = 5;
						break;
					}
					break;
				case 7:
					if(wcscmp(pcwszRow, pwszLocalName) == 0)
					{
						attr.first = pwszAttributeName;
						attr.second = pwszAttributeValue;
						row.push_back(attr);
					}
					break;
				default:
					break;
				}
			}

			switch(sequence)
			{
			case 2:
				sequence = 1;
				break;
			case 4:
				sequence = 3;
				break;
			case 7:
				pair.second.push_back(row);
				row.clear();
				break;
			default:
				break;
			}
			break;

		case XmlNodeType_EndElement:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 1:
				if(wcscmp(pcwszRoot, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 2:
				if(wcscmp(pcwszSection, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 4:
				if(wcscmp(pcwszSection, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 6:
				if(wcscmp(pcwszSection, pwszLocalName) == 0 || wcscmp(pcwszList, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 7:
				if(wcscmp(pcwszList, pwszLocalName) == 0 || wcscmp(pcwszEntry, pwszLocalName) == 0)
				{
					d_itr = list.find(pair.first);
					if(d_itr == list.end())
					{
						list.insert(pair);
					}
					else
					{
						for(l_itr = pair.second.begin(); l_itr != pair.second.end(); l_itr++)
						{
							d_itr->second.push_back(*l_itr);
						}
					}

					if(pos != 0)
					{
						goto exit;
					}
					else
					{
						sequence = 3;
					}
				}
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}

NOT_S_OK:
exit:
	CloseStreamReader(pReader, pFileStream);
	return hr;
}

HRESULT ReadList(LPCWSTR path, LPCWSTR section, APPDATAXMLLIST &list)
{
	HRESULT hr;
	IXmlReader *pReader = NULL;
	IStream *pFileStream = NULL;
	XmlNodeType nodeType;
	LPCWSTR pwszLocalName;
	LPCWSTR pwszAttributeName;
	LPCWSTR pwszAttributeValue;
	int sequence = 0;

	APPDATAXMLATTR attr;
	APPDATAXMLROW row;

	list.clear();
	list.shrink_to_fit();

	hr = CreateStreamReader(path, &pReader, &pFileStream);
	EXIT_NOT_S_OK(hr);

	while(pReader->Read(&nodeType) == S_OK)
	{
		switch(nodeType)
		{
		case XmlNodeType_Element:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 0:
				if(wcscmp(pcwszRoot, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 1:
				if(wcscmp(pcwszSection, pwszLocalName) == 0)
				{
					sequence = 2;
				}
				break;
			case 3:
				if(wcscmp(pcwszList, pwszLocalName) == 0)
				{
					sequence = 4;
				}
				break;
			case 4:
				if(wcscmp(pcwszRow, pwszLocalName) == 0)
				{
					sequence = 5;
					row.clear();
				}
				break;
			default:
				break;
			}

			for(hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
			{
				hr = pReader->GetLocalName(&pwszAttributeName, NULL);
				EXIT_NOT_S_OK(hr);
				hr = pReader->GetValue(&pwszAttributeValue, NULL);
				EXIT_NOT_S_OK(hr);

				switch(sequence)
				{
				case 2:
					if(wcscmp(pcwszSection, pwszLocalName) == 0 &&
						wcscmp(pcwszName, pwszAttributeName) == 0 && wcscmp(section, pwszAttributeValue) == 0)
					{
						sequence = 3;
					}
					break;
				case 5:
					if(wcscmp(pcwszRow, pwszLocalName) == 0)
					{
						attr.first = pwszAttributeName;
						attr.second = pwszAttributeValue;
						row.push_back(attr);
					}
					break;
				default:
					break;
				}
			}

			switch(sequence)
			{
			case 2:
				sequence = 1;
				break;
			case 5:
				list.push_back(row);
				row.clear();
				break;
			default:
				break;
			}
			break;

		case XmlNodeType_EndElement:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 1:
				if(wcscmp(pcwszRoot, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 2:
				if(wcscmp(pcwszSection, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 3:
				if(wcscmp(pcwszSection, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 4:
				if(wcscmp(pcwszSection, pwszLocalName) == 0 || wcscmp(pcwszList, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 5:
				if(wcscmp(pcwszList, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}

NOT_S_OK:
exit:
	CloseStreamReader(pReader, pFileStream);
	return hr;
}

HRESULT ReadValue(LPCWSTR path, LPCWSTR section, LPCWSTR key, std::wstring &strxmlval)
{
	IXmlReader *pReader = NULL;
	IStream *pFileStream = NULL;
	HRESULT hr;
	XmlNodeType nodeType;
	LPCWSTR pwszLocalName;
	LPCWSTR pwszAttributeName;
	LPCWSTR pwszAttributeValue;
	int sequence = 0;

	strxmlval.clear();

	hr = CreateStreamReader(path, &pReader, &pFileStream);
	EXIT_NOT_S_OK(hr);

	while(pReader->Read(&nodeType) == S_OK)
	{
		switch(nodeType)
		{
		case XmlNodeType_Element:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 0:
				if(wcscmp(pcwszRoot, pwszLocalName) == 0)
				{
					sequence = 1;
				}
				break;
			case 1:
				if(wcscmp(pcwszSection, pwszLocalName) == 0)
				{
					sequence = 2;
				}
				break;
			case 3:
				if(wcscmp(pcwszKey, pwszLocalName) == 0)
				{
					sequence = 4;
				}
				break;
			default:
				break;
			}

			for(hr = pReader->MoveToFirstAttribute(); hr == S_OK; hr = pReader->MoveToNextAttribute())
			{
				hr = pReader->GetLocalName(&pwszAttributeName, NULL);
				EXIT_NOT_S_OK(hr);
				hr = pReader->GetValue(&pwszAttributeValue, NULL);
				EXIT_NOT_S_OK(hr);

				switch(sequence)
				{
				case 2:
					if(wcscmp(pcwszName, pwszAttributeName) == 0 && wcscmp(section, pwszAttributeValue) == 0)
					{
						sequence = 3;
					}
					break;
				case 4:
					if(wcscmp(pcwszName, pwszAttributeName) == 0 && wcscmp(key, pwszAttributeValue) == 0)
					{
						sequence = 5;
					}
					break;
				case 5:
					if(wcscmp(pcwszValue, pwszAttributeName) == 0)
					{
						strxmlval.assign(pwszAttributeValue);
						goto exit;
					}
					break;
				default:
					break;
				}
			}
			break;

		case XmlNodeType_EndElement:
			hr = pReader->GetLocalName(&pwszLocalName, NULL);
			EXIT_NOT_S_OK(hr);

			switch(sequence)
			{
			case 1:
				if(wcscmp(pcwszRoot, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			case 3:
				if(wcscmp(pcwszSection, pwszLocalName) == 0)
				{
					goto exit;
				}
				break;
			default:
				break;
			}
			break;

		default:
			break;
		}
	}

NOT_S_OK:
exit:
	CloseStreamReader(pReader, pFileStream);
	return hr;
}

static void CloseStreamWriter(IXmlWriter *pWriter, IStream *pFileStream)
{
	if(pWriter != NULL)
	{
		pWriter->Release();
	}
	if(pFileStream != NULL)
	{
		pFileStream->Release();
	}
}

static HRESULT CreateStreamWriter(LPCWSTR path, IXmlWriter **ppWriter, IStream **ppFileStream)
{
	HRESULT hr;

	hr = CreateXmlWriter(IID_IXmlWriter, (LPVOID *)ppWriter, NULL);
	EXIT_NOT_S_OK(hr);
	hr = SHCreateStreamOnFileW(path, STGM_WRITE | STGM_CREATE, ppFileStream);
	EXIT_NOT_S_OK(hr);
	hr = (*ppWriter)->SetOutput(*ppFileStream);
	goto end;

NOT_S_OK:
	CloseStreamWriter(*ppWriter, *ppFileStream);
end:
	return hr;
}

HRESULT WriterStartElement(IXmlWriter *pWriter, LPCWSTR element)
{
	return pWriter->WriteStartElement(NULL, element, NULL);
}

HRESULT WriterEndElement(IXmlWriter *pWriter)
{
	return pWriter->WriteEndElement();
}

HRESULT WriterAttribute(IXmlWriter *pWriter, LPCWSTR name, LPCWSTR value)
{
	return pWriter->WriteAttributeString(NULL, name, NULL, value);
}

HRESULT WriterData(IXmlWriter *pWriter, const std::wstring &data)
{
	return pWriter->WriteString(data.c_str());
}

HRESULT WriterStartSection(IXmlWriter *pWriter, LPCWSTR name)
{
	HRESULT hr;

	hr = WriterStartElement(pWriter, pcwszSection);
	EXIT_NOT_S_OK(hr);
	hr = WriterAttribute(pWriter, pcwszName, name);

NOT_S_OK:
	return hr;
}

HRESULT WriterEndSection(IXmlWriter *pWriter)
{
	return WriterEndElement(pWriter);
}

HRESULT WriterKey(IXmlWriter *pWriter, LPCWSTR key, LPCWSTR value)
{
	HRESULT hr;

	hr = WriterStartElement(pWriter, pcwszKey);
	EXIT_NOT_S_OK(hr);

	hr = WriterAttribute(pWriter, pcwszName, key);
	EXIT_NOT_S_OK(hr);
	hr = WriterAttribute(pWriter, pcwszValue, value);
	EXIT_NOT_S_OK(hr);

	hr = WriterEndElement(pWriter);	//key

NOT_S_OK:
	return hr;
}

HRESULT WriterRow(IXmlWriter *pWriter, const APPDATAXMLROW &row)
{
	HRESULT hr;
	APPDATAXMLROW::const_iterator r_itr;

	for(r_itr = row.begin(); r_itr != row.end(); r_itr++)
	{
		hr = WriterAttribute(pWriter, r_itr->first.c_str(), r_itr->second.c_str());
		EXIT_NOT_S_OK(hr);
	}

NOT_S_OK:
	return hr;
}

HRESULT WriterList(IXmlWriter *pWriter, const APPDATAXMLLIST &list)
{
	HRESULT hr;
	APPDATAXMLLIST::const_iterator l_itr;

	hr = WriterStartElement(pWriter, pcwszList);
	EXIT_NOT_S_OK(hr);

	for(l_itr = list.begin(); l_itr != list.end(); l_itr++)
	{
		hr = WriterStartElement(pWriter, pcwszRow);
		EXIT_NOT_S_OK(hr);

		hr = WriterRow(pWriter, *l_itr);
		EXIT_NOT_S_OK(hr);

		hr = WriterEndElement(pWriter);	//row
		EXIT_NOT_S_OK(hr);
	}

	hr = WriterEndElement(pWriter);	//list
	EXIT_NOT_S_OK(hr);

NOT_S_OK:
	return hr;
}

HRESULT WriterInit(LPCWSTR path, IXmlWriter **ppWriter, IStream **pFileStream)
{
	HRESULT hr;

	hr = CreateStreamWriter(path, ppWriter, pFileStream);
	EXIT_NOT_S_OK(hr);

	hr = (*ppWriter)->SetProperty(XmlWriterProperty_Indent, TRUE);
	EXIT_NOT_S_OK(hr);

	hr = (*ppWriter)->WriteStartDocument(XmlStandalone_Omit);
	EXIT_NOT_S_OK(hr);

	hr = WriterStartElement(*ppWriter, pcwszRoot);
	EXIT_NOT_S_OK(hr);

NOT_S_OK:
	return hr;
}

HRESULT WriterFinal(IXmlWriter **ppWriter, IStream **ppFileStream)
{
	HRESULT hr;

	hr = WriterEndElement(*ppWriter);	//root
	EXIT_NOT_S_OK(hr);

	hr = (*ppWriter)->WriteEndDocument();
	EXIT_NOT_S_OK(hr);

	hr = (*ppWriter)->Flush();

NOT_S_OK:
	CloseStreamWriter(*ppWriter, *ppFileStream);
	return hr;
}
