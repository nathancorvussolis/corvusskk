package.path = ".\\?.lua;" .. package.path
package.cpath = ".\\?.dll;" .. package.cpath

crvmgr = require("test_c")
require("init")



el_test_string_table = {

{"A-Z", "(concat \"A\\C-I-\\^IZ\")", "A-Z"},
{"A-Z", "(concat \"A\\C-i-\\^iZ\")", "A-Z"},

{"A-Z", "(concat \"A\\C-@-\\^@Z\")", "A-Z"},
{"A-Z", "(concat \"A\\C-[-\\^[Z\")", "A-Z"},
{"A-Z", "(concat \"A\\C-\\-\\^^Z\")", "A-Z"},
{"A-Z", "(concat \"A\\C-]-\\^]Z\")", "A-Z"},
{"A-Z", "(concat \"A\\C-^-\\^^Z\")", "A-Z"},
{"A-Z", "(concat \"A\\C-_-\\^_Z\")", "A-Z"},
{"A-Z", "(concat \"A\\C-?-\\^?\\^?Z\")", "A-Z"},

{"octal", "(concat \"(\\057\\\\ \\073_\\073 \\057\\\\)\")", "(/\\ ;_; /\\)"},

{"AghZ", "(concat \"A\\g\\a\\b\\e\\f\\s\\n\\r\\t\\v\\h\\dZ\")", "Ag hZ"},

{"backslash", "(concat \"a\\\\\\057\\z\")", "a\\/z"},
{"backslash", "(concat \"a\\\\057\\z\")", "a\\057z"},

}



local result = "OK"

for i, v in ipairs(el_test_string_table) do
	print("<= \"" .. v[1] .. "\" \"".. v[2] .. "\"")
	local s = lua_skk_convert_candidate(v[1], v[2], "")

	if (s) then
		print("=> \"" .. s .. "\"")
	else
		print("=> nil")
	end

	if (v[3]) then
		if (type(v[3]) == "function") then
			v[3] = v[3](s)
		end

		if (tostring(s) == v[3]) then
			print "OK\n"
		else
			result = "NG"
			print("== " .. v[3] .. "\n" .. result)
			break
		end
	else
		print("\n")
	end
end

if (result == "OK") then
	print("\nAll OK.")
end
