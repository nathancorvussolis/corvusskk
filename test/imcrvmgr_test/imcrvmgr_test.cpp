
#include "common.h"
#include "parseskkdic.h"
#include "dictionary.h"

int wmain(int argc, wchar_t *argv[])
{
	_wsetlocale(LC_ALL, L"ja-JP");

	if (argc < 2)
	{
		fwprintf(stderr, L"imcrvmgr stress test\n\n");
		fwprintf(stderr, L"   usage : imcrvmgr_test <skk dictionary file>\n");
		return -1;
	}

	LPCWSTR filename = argv[1];

	_CreateIpcName();

	FILE *fp;

	//check BOM
	_wfopen_s(&fp, filename, RB);
	if (fp == nullptr)
	{
		fwprintf(stderr, L"\nERROR: File I/O.\n");
		return -1;
	}

	int encoding = 0;

	WCHAR bom = L'\0';
	fread(&bom, 2, 1, fp);
	fclose(fp);

	if (bom == BOM)
	{
		//UTF-16LE
		encoding = 16;

		HRESULT hr = CheckWideCharFile(filename);
		switch (hr)
		{
		case S_OK:
			break;
		case E_MAKESKKDIC_FILEIO:
			fwprintf(stderr, L"\nERROR: File I/O.\n");
			return -1;
			break;
		default:
			//Error
			encoding = -1;
			break;
		}
	}

	//UTF-8 ?
	if (encoding == 0)
	{
		HRESULT hr = CheckMultiByteFile(filename, 8);
		switch (hr)
		{
		case S_OK:
			encoding = 8;
			break;
		case E_MAKESKKDIC_FILEIO:
			fwprintf(stderr, L"\nERROR: File I/O.\n");
			return -1;
			break;
		default:
			break;
		}
	}

	//EUC-JIS-2004 ?
	if (encoding == 0)
	{
		HRESULT hr = CheckMultiByteFile(filename, 1);
		switch (hr)
		{
		case S_OK:
			encoding = 1;
			break;
		case E_MAKESKKDIC_FILEIO:
			fwprintf(stderr, L"\nERROR: File I/O.\n");
			return -1;
			break;
		default:
			break;
		}
	}

	switch (encoding)
	{
	case 1:
		//EUC-JIS-2004
		bom = L'\0';
		_wfopen_s(&fp, filename, RB);
		break;
	case 8:
		//UTF-8
		bom = BOM;
		_wfopen_s(&fp, filename, RccsUTF8);
		break;
	case 16:
		//UTF-16LE
		_wfopen_s(&fp, filename, RccsUTF16);
		break;
	default:
		fwprintf(stderr, L"\nERROR: invalid encoding.\n");
		return -1;
		break;
	}
	if (fp == nullptr)
	{
		fwprintf(stderr, L"\nERROR: cannot open \"%s\".\n", filename);
		return -1;
	}

	SYSTEMTIME st0;
	GetLocalTime(&st0);
	fwprintf(stderr, L"START : %04d-%02d-%02dT%02d:%02d:%02d\n",
		st0.wYear, st0.wMonth, st0.wDay, st0.wHour, st0.wMinute, st0.wSecond);

	std::wstring key, exkey;
	int okuri = -1;	//-1:header / 1:okuri-ari entries. / 0:okuri-nasi entries.
	SKKDICCANDIDATES sc;
	SKKDICOKURIBLOCKS so;

	while (true)
	{
		int rl = ReadSKKDicLine(fp, bom, okuri, key, sc, so);
		if (rl == -1)
		{
			//EOF
			fwprintf(stderr, L"\nEOF.\n");
			break;
		}
		else if (rl == 1)
		{
			//comment
			wprintf(L";\n");
			continue;
		}

		if (sc.empty())
		{
			continue;
		}

		{
			switch (okuri)
			{
			case 0:
				// ;; okuri-nasi entries.
				{
					REVERSE_ITERATION_I(sc_ritr, sc)
					{
						// add
						{
							WCHAR command = REQ_USER_ADD_N;

							wprintf(L"%c\t%s\t%s\t%s\t%s\n",
								command, key.c_str(), sc_ritr->first.c_str(), sc_ritr->second.c_str(), L"");

							BOOL ret = _AddUserDic(command, key, sc_ritr->first, sc_ritr->second, L"");
							if (ret == FALSE)
							{
								fwprintf(stderr, L"\nERROR: connection.\n");
								goto exit;
							}
						}

						// search
						{
							WCHAR command = REQ_SEARCH;
							CANDIDATES candidates;

							wprintf(L"%c\t%s\t%s\t%s\n",
								command, key.c_str(), key.c_str(), L"");

							BOOL ret = _SearchDic(command, candidates, key, key, L"");
							if (ret == FALSE)
							{
								fwprintf(stderr, L"\nERROR: connection.\n");
								goto exit;
							}

							if (!candidates.empty())
							{
								FORWARD_ITERATION_I(c_itr, candidates)
								{
									wprintf(L"/%s", c_itr->first.first.c_str());
									if (!c_itr->first.second.empty())
									{
										wprintf(L";%s", c_itr->first.second.c_str());
									}
								}
								wprintf(L"/\n");
							}
						}

						// delete
						{
							WCHAR command = REQ_USER_DEL_N;

							wprintf(L"%c\t%s\t%s\n",
								command, key.c_str(), sc_ritr->first.c_str());

							_DelUserDic(command, key, sc_ritr->first);
						}

						// add
						{
							WCHAR command = REQ_USER_ADD_N;

							wprintf(L"%c\t%s\t%s\t%s\t%s\n",
								command, key.c_str(), sc_ritr->first.c_str(), sc_ritr->second.c_str(), L"");

							BOOL ret = _AddUserDic(command, key, sc_ritr->first, sc_ritr->second, L"");
							if (ret == FALSE)
							{
								fwprintf(stderr, L"\nERROR: connection.\n");
								goto exit;
							}
						}

						// complement
						{
							WCHAR command = REQ_COMPLEMENT;
							CANDIDATES candidates;
							std::wstring ckey;

							ckey.push_back(key.front());

							wprintf(L"%c\t%s\t%s\t%s\n",
								command, ckey.c_str(), L"8", L"");

							BOOL ret = _SearchDic(command, candidates, ckey, L"8", L"");
							if (ret == FALSE)
							{
								fwprintf(stderr, L"\nERROR: connection.\n");
								goto exit;
							}

							if (!candidates.empty())
							{
								FORWARD_ITERATION_I(c_itr, candidates)
								{
									wprintf(L"/%s", c_itr->first.first.c_str());
									if (!c_itr->first.second.empty())
									{
										wprintf(L";%s", c_itr->first.second.c_str());
									}
								}
								wprintf(L"/\n");
							}
						}
					}
				}
				break;
			case 1:
				// ;; okuri-ari entries.
				{
					WCHAR r = key.back();
					REVERSE_ITERATION_I(sc_ritr, sc)
					{
						for (int i = 0; i < _countof(okuri_block); i++)
						{
							if (okuri_block[i].r == L'\0') break;

							if (r == okuri_block[i].r)
							{
								for (int j = 0; j < _countof(okuri_block[i].d); j++)
								{
									WCHAR d = okuri_block[i].d[j];

									if (d == L'\0') break;

									WCHAR okuri_key[2] = {d, L'\0'};

									// add
									{
										WCHAR command = REQ_USER_ADD_A;

										wprintf(L"%c\t%s\t%s\t%s\t%s\n",
											command, key.c_str(), sc_ritr->first.c_str(), sc_ritr->second.c_str(), okuri_key);

										BOOL ret = _AddUserDic(command, key, sc_ritr->first, sc_ritr->second, okuri_key);
										if (ret == FALSE)
										{
											fwprintf(stderr, L"\nERROR: connection.\n");
											goto exit;
										}
									}

									// search
									{
										WCHAR command = REQ_SEARCH;
										CANDIDATES candidates;

										wprintf(L"%c\t%s\t%s\t%s\n",
											command, key.c_str(), key.c_str(), okuri_key);

										BOOL ret = _SearchDic(command, candidates, key, key, okuri_key);
										if (ret == FALSE)
										{
											fwprintf(stderr, L"\nERROR: connection.\n");
											goto exit;
										}

										if (!candidates.empty())
										{
											FORWARD_ITERATION_I(c_itr, candidates)
											{
												wprintf(L"/%s", c_itr->first.first.c_str());
												if (!c_itr->first.second.empty())
												{
													wprintf(L";%s", c_itr->first.second.c_str());
												}
											}
											wprintf(L"/\n");
										}
									}

									// delete
									{
										WCHAR command = REQ_USER_DEL_N;

										wprintf(L"%c\t%s\t%s\n",
											command, key.c_str(), sc_ritr->first.c_str());

										_DelUserDic(command, key, sc_ritr->first);
									}

									// add
									{
										WCHAR command = REQ_USER_ADD_A;

										wprintf(L"%c\t%s\t%s\t%s\t%s\n",
											command, key.c_str(), sc_ritr->first.c_str(), sc_ritr->second.c_str(), okuri_key);

										BOOL ret = _AddUserDic(command, key, sc_ritr->first, sc_ritr->second, okuri_key);
										if (ret == FALSE)
										{
											fwprintf(stderr, L"\nERROR: connection.\n");
											goto exit;
										}
									}
								}
								break;
							}
						}
					}
				}
				break;
			default:
				fwprintf(stderr, L"\nERROR: reading dictionary.\n");
				goto exit;
				break;
			}
		}

		{
			if (!exkey.empty() && key.front() != exkey.front())
			{
				// save user dictionary
				{
					wprintf(L"%c\n", REQ_USER_SAVE);

					BOOL ret = _SaveUserDic();
					if (ret == FALSE)
					{
						fwprintf(stderr, L"\nERROR: connection.\n");
						goto exit;
					}
				}
			}

			exkey = key;
		}
	}

	{
		// save user dictionary
		{
			wprintf(L"%c\n", REQ_USER_SAVE);

			BOOL ret = _SaveUserDic();
			if (ret == FALSE)
			{
				fwprintf(stderr, L"\nERROR: connection.\n");
				goto exit;
			}
		}
	}

exit:

	fclose(fp);

	SYSTEMTIME st1;
	GetLocalTime(&st1);

	fwprintf(stderr, L"\n");

	fwprintf(stderr, L"START : %04d-%02d-%02dT%02d:%02d:%02d\n",
		st0.wYear, st0.wMonth, st0.wDay, st0.wHour, st0.wMinute, st0.wSecond);

	fwprintf(stderr, L"END   : %04d-%02d-%02dT%02d:%02d:%02d\n",
		st1.wYear, st1.wMonth, st1.wDay, st1.wHour, st1.wMinute, st1.wSecond);

	return 0;
}
