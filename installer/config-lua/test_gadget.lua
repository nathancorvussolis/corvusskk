package.path = ".\\?.lua;" .. package.path
package.cpath = ".\\?.dll;" .. package.cpath

crvmgr = require("test_c")
require("init")



function get_unit_candidate(from, to)
	return string.format("(skk-gadget-units-conversion \"%s\" (string-to-number (car skk-num-list)) \"%s\")", from, to)
end

function test_unit_converted(n, unit)
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. unit
end



el_test_gadget_table = {

{"見出し語", "(concat skk-henkan-key)", "見出し語"},
{"1a2b3c4d5", "(car skk-num-list)", "1"},
{"1a2b3c4d5", "(car (cdr skk-num-list))", "2"},
{"1a2b3c4d5", "(car (cdr (cdr skk-num-list)))", "3"},
{"1a2b3c4d5", "(car (cdr (cdr (cdr skk-num-list))))", "4"},
{"1a2b3c4d5", "(car (cdr (cdr (cdr (cdr skk-num-list)))))", "5"},
{"!", "(concat \"&excl\\073\")", "&excl;"},
{"perl", "(concat \"#!\\057usr\\057local\\057bin\\057perl\")", "#!/usr/local/bin/perl"},
{"bar", "(make-string (- fill-column 1) ?-)", "---------------------------------------------------------------------"},
{"line", "(make-string (- (window-width) 5) ?-)", "---------------------------------------------------------------------------"},
{"line", "(make-string (- (window-width) 5) (string-to-char comment-start))", "///////////////////////////////////////////////////////////////////////////"},
{"size", "(concat \"w:\" (number-to-string (window-width)) \" h:\" (number-to-string (window-height)))", "w:80 h:23"},
{"now", "(current-time-string)", function (s)
	local dayofweek_table = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"}
	local w = string.sub(s, 1, 3)
	for i, v in ipairs(dayofweek_table) do
		if (w == v) then
			w = ""
			break
		end
	end
	if (w ~= "") then return "Error : day of week" end
	local month_table = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"}
	local m = string.sub(s, 5, 7)
	for i, v in ipairs(month_table) do
		if (m == v) then
			m = i
			break
		end
	end
	if (type(m) ~= "number") then return "Error : month" end
	local mday_table = {31,
		function(year)
			if (year % 400 == 0) then return 29
			elseif (year % 100 == 0) then return 28
			elseif (year % 4 == 0) then return 29 end
			return 28
		end,
		31, 30, 31, 30, 31, 31, 30, 31, 30, 31}
	local y = string.sub(s, 21, 24)
	if (not (string.match(y, "^[%d][%d][%d][%d]$"))) then return "Error : year" end
	local mdays = mday_table[m]
	if (type(mdays) == "function") then mdays = mdays(tonumber(y)) end
	local d = string.sub(s, 9, 10)
	if (not (string.match(d, "^[ %d][%d]$") and
		(1 <= tonumber(d) and tonumber(d) <= mdays))) then return "Error : days of month" end
	local t = string.sub(s, 12, 19)
	if (not (string.match(t, "^[0-1][0-9]:[0-5][0-9]:[0-5][0-9]$") or
		string.match(t, "^[2][0-3]:[0-5][0-9]:[0-5][0-9]$"))) then return "Error : time" end
	return s
end},
{"now", "(substring (current-time-string) 11 16)", function(s)
	if (not (string.match(s, "^[0-1][0-9]:[0-5][0-9]$") or
		string.match(s, "^[2][0-3]:[0-5][0-9]$"))) then return "" end
	return s
end},
{"now", "(substring (current-time-string) 11 19)", function(s)
	if (not (string.match(s, "^[0-1][0-9]:[0-5][0-9]:[0-5][0-9]$") or
		string.match(s, "^[2][0-3]:[0-5][0-9]:[0-5][0-9]$"))) then return "" end
	return s
end},
{"skk", "(skk-version)", "CorvusSKK X.Y.Z"},

{"1mile", get_unit_candidate("mile", "yard"), test_unit_converted(1760, "yard")},
{"1mile", get_unit_candidate("mile", "feet"), test_unit_converted(1760 * 3, "feet")},
{"1mile", get_unit_candidate("mile", "inch"), test_unit_converted(1760 * 3 * 12, "inch")},
{"1mile", get_unit_candidate("mile", "m"),    test_unit_converted(0.9144 * 1760, "m")},
{"1mile", get_unit_candidate("mile", "km"),   test_unit_converted(0.9144 * 1760 / 1000, "km")},

{"1yard", get_unit_candidate("yard", "mile"), test_unit_converted(1 / 1760, "mile")},
{"1yard", get_unit_candidate("yard", "feet"), test_unit_converted(3, "feet")},
{"1yard", get_unit_candidate("yard", "inch"), test_unit_converted(3 * 12, "inch")},
{"1yard", get_unit_candidate("yard", "m"),    test_unit_converted(0.9144, "m")},
{"1yard", get_unit_candidate("yard", "mm"),   test_unit_converted(0.9144 * 1000, "mm")},

{"1feet", get_unit_candidate("feet", "mile"), test_unit_converted(1 / (1760 * 3), "mile")},
{"1feet", get_unit_candidate("feet", "yard"), test_unit_converted(1 / 3, "yard")},
{"1feet", get_unit_candidate("feet", "inch"), test_unit_converted(12, "inch")},
{"1feet", get_unit_candidate("feet", "m"),    test_unit_converted(0.9144 / 3, "m")},
{"1feet", get_unit_candidate("feet", "mm"),   test_unit_converted(0.9144 / 3 * 1000, "mm")},
 
{"1inch", get_unit_candidate("inch", "mile"), test_unit_converted(1 / (1760 * 3 * 12), "mile")},
{"1inch", get_unit_candidate("inch", "yard"), test_unit_converted(1 / (3 * 12), "yard")},
{"1inch", get_unit_candidate("inch", "feet"), test_unit_converted(1 / 12, "feet")},
{"1inch", get_unit_candidate("inch", "m"),    test_unit_converted(0.0254, "m")},
{"1inch", get_unit_candidate("inch", "mm"),   test_unit_converted(25.4, "mm")},

{"1pound", get_unit_candidate("pound", "ounce"), test_unit_converted(16, "ounce")},
{"1pound", get_unit_candidate("pound", "grain"), test_unit_converted(7000, "grain")},
{"1pound", get_unit_candidate("pound", "kg"),    test_unit_converted(0.45359237, "kg")},
{"1pound", get_unit_candidate("pound", "g"),     test_unit_converted(0.45359237 * 1000, "g")},

{"1ounce", get_unit_candidate("ounce", "pound"), test_unit_converted(1 / 16, "pound")},
{"1ounce", get_unit_candidate("ounce", "grain"), test_unit_converted(7000 / 16, "grain")},
{"1ounce", get_unit_candidate("ounce", "g"),     test_unit_converted(28.349523125, "g")},

{"1grain", get_unit_candidate("grain", "pound"), test_unit_converted(1 / 7000, "pound")},
{"1grain", get_unit_candidate("grain", "ounce"), test_unit_converted(16 / 7000, "ounce")},
{"1grain", get_unit_candidate("grain", "g"),     test_unit_converted(0.06479891, "g")},
{"1grain", get_unit_candidate("grain", "mg"),    test_unit_converted(0.06479891 * 1000, "mg")},

{"1もう",   get_unit_candidate("毛", "m"),  test_unit_converted((10 / 33) / 10000, "m")},
{"1もう",   get_unit_candidate("毛", "mm"), test_unit_converted((10 / 33) / 10000 * 1000, "mm")},
{"1りん",   get_unit_candidate("厘", "m"),  test_unit_converted((10 / 33) / 1000, "m")},
{"1りん",   get_unit_candidate("厘", "mm"), test_unit_converted((10 / 33) / 1000 * 1000, "mm")},
{"1ぶ",     get_unit_candidate("分", "m"),  test_unit_converted((10 / 33) / 100, "m")},
{"1ぶ",     get_unit_candidate("分", "mm"), test_unit_converted((10 / 33) / 100 * 1000, "mm")},
{"1すん",   get_unit_candidate("寸", "m"),  test_unit_converted((10 / 33) / 10, "m")},
{"1すん",   get_unit_candidate("寸", "mm"), test_unit_converted((10 / 33) / 10 * 1000, "mm")},
{"1しゃく", get_unit_candidate("尺", "m"),  test_unit_converted((10 / 33), "m")},
{"1しゃく", get_unit_candidate("尺", "mm"), test_unit_converted((10 / 33) * 1000, "mm")},
{"1じょう", get_unit_candidate("丈", "m"),  test_unit_converted((10 / 33) * 10, "m")},
{"1けん",   get_unit_candidate("間", "m"),  test_unit_converted((10 / 33) * 6, "m")},
{"1ちょう", get_unit_candidate("町", "m"),  test_unit_converted((10 / 33) * 360, "m")},
{"1り",     get_unit_candidate("里", "m"),  test_unit_converted((10 / 33) * 12960, "m")},
{"1り",     get_unit_candidate("里", "km"), test_unit_converted((10 / 33) * 12960 / 1000, "km")},

{"1しゃく", get_unit_candidate("勺", "㎡"), test_unit_converted((400 / 121) / 100, "㎡")},
{"1ごう",   get_unit_candidate("合", "㎡"), test_unit_converted((400 / 121) / 10, "㎡")},
{"1ぶ",     get_unit_candidate("歩", "㎡"), test_unit_converted((400 / 121), "㎡")},
{"1つぼ",   get_unit_candidate("坪", "㎡"), test_unit_converted((400 / 121), "㎡")},
{"1せ",     get_unit_candidate("畝", "㎡"), test_unit_converted((400 / 121) * 30, "㎡")},
{"1たん",   get_unit_candidate("反", "㎡"), test_unit_converted((400 / 121) * 300, "㎡")},
{"1ちょう", get_unit_candidate("町", "㎡"), test_unit_converted((400 / 121) * 3000, "㎡")},

{"1しゃく", get_unit_candidate("勺", "L"),  test_unit_converted((2401 / 1331) / 100, "L")},
{"1しゃく", get_unit_candidate("勺", "mL"), test_unit_converted((2401 / 1331) / 100 * 1000, "mL")},
{"1ごう",   get_unit_candidate("合", "L"),  test_unit_converted((2401 / 1331) / 10, "L")},
{"1ごう",   get_unit_candidate("合", "mL"), test_unit_converted((2401 / 1331) / 10 * 1000, "mL")},
{"1しょう", get_unit_candidate("升", "L"),  test_unit_converted((2401 / 1331), "L")},
{"1しょう", get_unit_candidate("升", "mL"), test_unit_converted((2401 / 1331) * 1000, "mL")},
{"1と",     get_unit_candidate("斗", "L"),  test_unit_converted((2401 / 1331) * 10, "L")},
{"1と",     get_unit_candidate("斗", "mL"), test_unit_converted((2401 / 1331) * 10 * 1000, "mL")},
{"1こく",   get_unit_candidate("石", "L"),  test_unit_converted((2401 / 1331) * 100, "L")},
{"1こく",   get_unit_candidate("石", "mL"), test_unit_converted((2401 / 1331) * 100 * 1000, "mL")},

{"1もう",   get_unit_candidate("毛", "kg"), test_unit_converted(3.75 / 1000000, "kg")},
{"1もう",   get_unit_candidate("毛", "g"),  test_unit_converted(3.75 / 1000000 * 1000, "g")},
{"1りん",   get_unit_candidate("厘", "kg"), test_unit_converted(3.75 / 100000, "kg")},
{"1りん",   get_unit_candidate("厘", "g"),  test_unit_converted(3.75 / 100000 * 1000, "g")},
{"1ぶ",     get_unit_candidate("分", "kg"), test_unit_converted(3.75 / 10000, "kg")},
{"1ぶ",     get_unit_candidate("分", "g"),  test_unit_converted(3.75 / 10000 * 1000, "g")},
{"1もんめ", get_unit_candidate("匁", "kg"), test_unit_converted(3.75 / 1000, "kg")},
{"1もんめ", get_unit_candidate("匁", "g"),  test_unit_converted(3.75 / 1000 * 1000, "g")},
{"1きん",   get_unit_candidate("斤", "kg"), test_unit_converted(3.75 * 0.16, "kg")},
{"1きん",   get_unit_candidate("斤", "g"),  test_unit_converted(3.75 * 0.16 * 1000, "g")},
{"1かん",   get_unit_candidate("貫", "kg"), test_unit_converted(3.75, "kg")},
{"1かん",   get_unit_candidate("貫", "g"),  test_unit_converted(3.75 * 1000, "g")},

{"おみくじ", "(skk-omikuji)", function(s)
	local omikuji_table = {"大吉", "吉", "中吉", "小吉", "末吉", "凶", "大凶"}
	for i, v in ipairs(omikuji_table) do
		if (v == s) then return s end
	end
	return ""
end},
{"引用符", "(concat \"\\057\\\"\\073\")", "/\";"},
{"引用符", "(concat \"\\\"\\057\\\"\\073\\\"\")", "\"/\";\""},
{"引用符", "(concat \"\\057\\\"\\073\" (concat \" - \\\" - \")   \"\\\"\\057\\\"\\073\\\"\")", "/\"; - \" - \"/\";\""},
{"control", "(concat \"A\\C-a\\^aZ\")", "AC-a^aZ"},
{"control", "(  concat  \"_\\ \"    \"b\"    \" \"    \"g\"    \" \"    \"z\"  )", "_b g z"},
{"control", "(concat \"A\\g\\a\\b\\e\\f\\s\\ \\n\\r\\t\\v\\h\\i\\dZ\")", "Ag hiZ"},
{"octal", "(concat \"\\\\\\057\\\\\\073\\057\\073\\\\\" \"\\\\\\057\\\\\")", "\\/\\;/;\\\\/\\"},
{"加算", "(+)", "0"},
{"加算", "(+ 3)", "3"},
{"加算", "(+ 12.345 23.456 34.567)", "70.368"},
{"加算", "(+ 0 1 1 2 3 5 8 13 21 34 55 89)", "232"},
{"加算", "(1+ 24)", "25"},
{"減算", "(-)", "0"},
{"減算", "(- 3)", "-3"},
{"減算", "(- 34.567 23.456 12.345)", "-1.234"},
{"減算", "(- 89 55 34 21 13 8 5 3 2 1 1 0)", "-54"},
{"減算", "(1- 24)", "23"},
{"顔文字", "(concat \"(\\073´Д`)\")", "(;´Д`)"},
{"顔文字", "(- _ -)", "(- _ -)"},
{"顔文字", "(+ _ +)", "(+ _ +)"},

}



local result = "OK"

for i, v in ipairs(el_test_gadget_table) do
	print("<= \"" .. v[1] .. "\" \"".. v[2] .. "\"")
	local s = lua_skk_convert_candidate(v[1], v[2], "")

	if (s) then
		if (s == "") then
			s = v[2]
		end
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
