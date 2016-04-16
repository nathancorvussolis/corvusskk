
std::string wstring_to_utf8_string(const std::wstring &s)
{
	std::string ret;

	int len = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, nullptr, 0, nullptr, nullptr);
	if(len > 0)
	{
		try
		{
			LPSTR utf8 = new CHAR[len];
			if(WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, utf8, len, nullptr, nullptr) > 0)
			{
				ret = utf8;
			}
			delete[] utf8;
		}
		catch(...)
		{
		}
	}

	return ret;
}

std::wstring utf8_string_to_wstring(const std::string &s)
{
	std::wstring ret;

	int len = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, nullptr, 0);
	if(len > 0)
	{
		try
		{
			LPWSTR wcs = new WCHAR[len];
			if(MultiByteToWideChar(CP_UTF8, 0, s.c_str(), -1, wcs, len) > 0)
			{
				ret = wcs;
			}
			delete[] wcs;
		}
		catch(...)
		{
		}
	}

	return ret;
}
