package.path = ".\\?.lua;" .. package.path
package.cpath = ".\\?.dll;" .. package.cpath

crvmgr = require("test_c")
require("init")



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
{"1feet", "(skk-gadget-units-conversion \"feet\" (string-to-number (car skk-num-list)) \"cm\")", function(s)
	local n = 30.48 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "cm"
end},
{"1inch", "(skk-gadget-units-conversion \"inch\" (string-to-number (car skk-num-list)) \"cm\")", function(s)
	local n = 2.54 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "cm"
end},
{"1inch", "(skk-gadget-units-conversion \"inch\" (string-to-number (car skk-num-list)) \"feet\")", function(s)
	local n = (1.0 / 12.0) * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "feet"
end},
{"1mile", "(skk-gadget-units-conversion \"mile\" (string-to-number (car skk-num-list)) \"km\")", function(s)
	local n = 1.609344 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "km"
end},
{"1mile", "(skk-gadget-units-conversion \"mile\" (string-to-number (car skk-num-list)) \"yard\")", function(s)
	local n = 1760.0 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "yard"
end},
{"1yard", "(skk-gadget-units-conversion \"yard\" (string-to-number (car skk-num-list)) \"cm\")", function(s)
	local n = 91.44 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "cm"
end},
{"1yard", "(skk-gadget-units-conversion \"yard\" (string-to-number (car skk-num-list)) \"feet\")", function(s)
	local n = 3.0 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "feet"
end},
{"1すん", "(skk-gadget-units-conversion \"寸\" (string-to-number (car skk-num-list)) \"mm\")", function(s)
	local n = (1000 / 33) * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "mm"
end},
{"1しゃく", "(skk-gadget-units-conversion \"尺\" (string-to-number (car skk-num-list)) \"cm\")", function(s)
	local n = (1000 / 33) * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "cm"
end},
{"1しゃく", "(skk-gadget-units-conversion \"勺\" (string-to-number (car skk-num-list)) \"mL\")", function(s)
	local n = (2401 / 1331) * 10 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "mL"
end},
{"1ごう", "(skk-gadget-units-conversion \"合\" (string-to-number (car skk-num-list)) \"mL\")", function(s)
	local n = (2401 / 1331) * 100 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "mL"
end},
{"1しょう", "(skk-gadget-units-conversion \"升\" (string-to-number (car skk-num-list)) \"L\")", function(s)
	local n = (2401 / 1331) * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "L"
end},
{"1と", "(skk-gadget-units-conversion \"斗\" (string-to-number (car skk-num-list)) \"L\")", function(s)
	local n = (2401 / 1331) * 10 * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "L"
end},
{"1つぼ", "(skk-gadget-units-conversion \"坪\" (string-to-number (car skk-num-list)) \"㎡\")", function(s)
	local n = (400 / 121) * 1
	if (math.tointeger(n)) then n = math.tointeger(n) end
	return tostring(n) .. "㎡"
end},
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
{"加算", "(+ 24 6)", "30"},
{"減算", "(- 24 6)", "18"},
{"乗算", "(* 24 6)", "144"},
{"除算", "(concat \"(\\057 24 6)\")", "4"},
{"剰余", "(% 24 7)", "3"},
{"四則演算", "(concat \"(+ 9 (* (\\057 24 (- 15 (% 99 10))) 3))\")", "21"},

}



local result = "OK"

for i, v in ipairs(el_test_gadget_table) do
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
