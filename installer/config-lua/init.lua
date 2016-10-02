--[[
	 CorvusSKK Lua拡張スクリプト


	 Cから呼ばれるLuaの関数

		辞書検索
			lua_skk_search(key, okuri)
				key : 見出し語 string
				okuri : 送り仮名 string
				戻り値 : "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n" or "" string
		補完
			lua_skk_complement(key)
				key : 見出し語 string
				戻り値 : "/<K1>/<K2>/.../<Kn>/\n" or "" string
		見出し語変換
			lua_skk_convert_key(key, okuri)
				key : 見出し語 string
				okuri : 送り仮名 string
				戻り値 : 変換済み文字列 string
		候補変換
			lua_skk_convert_candidate(key, candidate, okuri)
				key : 見出し語 string
				candidate : 候補 string
				okuri : 送り仮名 string
				戻り値 : 変換済み文字列 string
		辞書追加
			lua_skk_add(okuriari, key, candidate, annotation, okuri)
				okuriari : boolean (送りあり:true/送りなし:false)
				key : 見出し語 string
				candidate : 候補 string
				annotation : 注釈 string
				okuri : 送り仮名 string
				戻り値 : なし
		辞書削除
			lua_skk_delete(okuriari, key, candidate)
				okuriari : boolean (送りあり:true/送りなし:false)
				key : 見出し語 string
				candidate : 候補 string
				戻り値 : なし
		辞書保存
			lua_skk_save()
				戻り値 : なし


	Luaから呼ばれるCの関数

		ユーザー辞書検索
			crvmgr.search_user_dictionary(key, okuri)
				key : 見出し語 string
				okuri : 送り仮名 string
				戻り値 : "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n" or "" string
		SKK辞書検索
			crvmgr.search_skk_dictionary(key, okuri)
				key : 見出し語 string
				okuri : 送り仮名 string
				戻り値 : "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n" or "" string
		SKK辞書サーバー検索
			crvmgr.search_skk_server(key)
				key : 見出し語 string
				戻り値 : "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n" or "" string
		SKK辞書サーバー情報検索
			crvmgr.search_skk_server_info()
				戻り値 : SKK Serverプロトコル"2"の結果 バージョン番号 string
						SKK Serverプロトコル"3"の結果 ホスト名 string 
		Unicodeコードポイント変換
			crvmgr.search_unicode(key)
				key : 見出し語 string
				戻り値 : "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n" or "" string
		JIS X 0213面区点番号変換
			crvmgr.search_jisx0213(key)
				key : 見出し語 string
				戻り値 : "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n" or "" string
		JIS X 0208区点番号変換
			crvmgr.search_jisx0208(key)
				key : 見出し語 string
				戻り値 : "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n" or "" string
		文字コード表記変換 (ASCII, JIS X 0201(片仮名, 8bit), JIS X 0213 / Unicode)
			crvmgr.search_character_code(key)
				key : 見出し語 string
				戻り値 : "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n" or "" string
		補完
			crvmgr.complement(key)
				key : 見出し語 string
				戻り値 : "/<K1>/<K2>/.../<Kn>/\n" or "" string
		辞書追加
			crvmgr.add(okuriari, key, candidate, annotation, okuri)
				okuriari : boolean (送りあり:true/送りなし:false)
				key : 見出し語 string
				candidate : 候補 string
				annotation : 注釈 string
				okuri : 送り仮名 string
				戻り値 : なし
		辞書削除
			crvmgr.delete(okuriari, key, candidate)
				okuriari : boolean (送りあり:true/送りなし:false)
				key : 見出し語 string
				candidate : 候補 string
				戻り値 : なし
		辞書保存
			crvmgr.save()
				戻り値 : なし


	Cから定義される変数

		バージョン (skk-version)に使用
			SKK_VERSION
				"CorvusSKK X.Y.Z" string
--]]



-- 数値変換
enable_skk_convert_num = true
-- 実行変換
enable_skk_convert_gadget = true
-- skk-ignore-dic-word
enable_skk_ignore_dic_word = false
-- skk-search-sagyo-henkaku (t:true/anything:false)
enable_skk_search_sagyo_only = true


-- 数値変換タイプ1 (全角数字)
local skk_num_type1_table = {"０", "１", "２", "３", "４", "５", "６", "７", "８", "９"}

-- 数値変換タイプ2, 3 (漢数字)
local skk_num_type3_table = {"〇", "一", "二", "三", "四", "五", "六", "七", "八", "九"}
local skk_num_type3_1k_table = {"", "十", "百", "千"}
local skk_num_type3_10k_table = {"", "万", "億", "兆", "京", "垓",
	"𥝱", "穣", "溝", "澗", "正", "載", "極", "恒河沙", "阿僧祇", "那由他", "不可思議", "無量大数"}

-- 数値変換タイプ5 (漢数字、大字)
local skk_num_type5_table = {"零", "壱", "弐", "参", "四", "五", "六", "七", "八", "九"}
local skk_num_type5_1k_table = {"", "拾", "百", "千"}
local skk_num_type5_10k_table = {"", "万", "億", "兆", "京", "垓",
	"𥝱", "穣", "溝", "澗", "正", "載", "極", "恒河沙", "阿僧祇", "那由他", "不可思議", "無量大数"}

-- 数値変換タイプ6 (独自拡張、ローマ数字)
local skk_num_type6_table_I = {"", "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX"}
local skk_num_type6_table_X = {"", "X", "XX", "XXX", "XL", "L", "LX", "LXX", "LXXX", "XC"}
local skk_num_type6_table_C = {"", "C", "CC", "CCC", "CD", "D", "DC", "DCC", "DCCC", "CM"}
local skk_num_type6_table_M = {"", "M", "MM", "MMM"}

-- 数値変換タイプ8 (桁区切り)
local skk_num_type8_sep = ","
local skk_num_type8_sepnum = 3

-- 現在時刻
local skk_gadget_time = 0

-- skk-henkan-key
local skk_henkan_key = ""

-- skk-num-list
local skk_num_list = {}

-- おみくじ吉凶テーブル
local skk_gadget_omikuji_table = {"大吉", "吉", "中吉", "小吉", "末吉", "凶", "大凶"}

-- 元号テーブル
local skk_gadget_gengo_table = {
	{{1989,  1,  8, 1}, "へいせい",   {"平成", "H"}}, -- 1989/01/08
	{{1926, 12, 25, 1}, "しょうわ",   {"昭和", "S"}}, -- 1926/12/25
	{{1912,  7, 30, 1}, "たいしょう", {"大正", "T"}}, -- 1912/07/30
	{{1873,  1,  1, 6}, "めいじ",     {"明治", "M"}}, -- 1873/01/01(グレゴリオ暦 明治6年)
}

-- 月テーブル
local skk_gadget_month_table = {
	{"Jan", "1"}, {"Feb", "2"}, {"Mar", "3"}, {"Apr", "4"}, {"May", "5"}, {"Jun", "6"},
	{"Jul", "7"}, {"Aug", "8"}, {"Sep", "9"}, {"Oct", "10"}, {"Nov", "11"}, {"Dec", "12"}
}

-- 曜日テーブル
local skk_gadget_dayofweek_table = {
	{"Sun", "日"}, {"Mon", "月"}, {"Tue", "火"}, {"Wed", "水"}, {"Thu", "木"}, {"Fri", "金"}, {"Sat", "土"}
}

-- 単位テーブル
local skk_gadget_unit_table_org = {
	{"mile", {{"yard", 1760.0}, {"feet", 5280.0}, {"m", 1609.344}, {"km", 1.609344}}},
	{"yard", {{"feet", 3.0}, {"inch", 36.0}, {"m", 0.9144}, {"cm", 91.44}, {"mm", 914.4}}},
	{"feet", {{"inch", 12.0}, {"yard",  (1.0 / 3.0)}, {"m", 0.3048}, {"cm", 30.48}, {"mm", 304.8}}},
	{"inch", {{"feet", (1.0 / 12.0)}, {"yard", (1.0 / 36.0)}, {"m", 0.0254}, {"cm", 2.54}, {"mm", 25.4}}},
	{"pound", {{"g", 453.59237}, {"ounce", 16.0}, {"grain", 7000.0}}},
	{"ounce", {{"g", 28.349523125}, {"pound", (1.0 / 16.0)}, {"grain", (7000.0 / 16.0)}}},
	{"grain", {{"mg", 64.79891}, {"g", 0.06479891}, {"pound", (1.0 / 7000.0)}, {"ounce", (16.0 / 7000.0)}}},
	{"寸", {{"mm", (1000 / 33)}, {"cm", (100 / 33)}}},
	{"尺", {{"mm", (10000 / 33)}, {"cm", (1000 / 33)}}},
	{"坪", {{"㎡", (400 / 121)}}},
	{"勺", {{"L", (2401 / 1331) / 100}, {"mL", (2401 / 1331) * 10}}},
	{"合", {{"L", (2401 / 1331) / 10}, {"mL", (2401 / 1331) * 100}}},
	{"升", {{"L", (2401 / 1331)}}},
	{"斗", {{"L", (2401 / 1331) * 10}}},
}
local skk_gadget_unit_table = {}
for i, v in ipairs(skk_gadget_unit_table_org) do
	local unit_to_table = {}
	for j, vv in ipairs(v[2]) do
		unit_to_table[vv[1]] = vv[2]
	end
	skk_gadget_unit_table[v[1]] = unit_to_table
end

-- 変数テーブル
local skk_gadget_variable_table_org = {
	{"skk-henkan-key", function() return skk_henkan_key end},
	{"skk-num-list", function() return skk_num_list end},
	{"fill-column", "70"},
	{"comment-start", "/*"},
	{"comment-end", "*/"},
}
local skk_gadget_variable_table = {}
for i, v in ipairs(skk_gadget_variable_table_org) do
	skk_gadget_variable_table[v[1]] = v[2]
end

-- (window-width)
local window_width_value = "80"

-- (window-height)
local window_height_value = "23"

-- 文字コード表記変換プレフィックス
local charcode_conv_prefix = "?"



-- 数字を漢数字に変換
local function skk_num_to_kanji(num, type_table, type_1k_table, type_10k_table)
	local ret = ""

	-- 0を付加して4の倍数の桁数にする
	num = string.rep("0", 4 - string.len(num) % 4) .. num
	local m = string.len(num) / 4

	for i = 1, m do
		for j = 1, 4 do
			local sj = string.sub(num, (i - 1) * 4 + j, (i - 1) * 4 + j)
			if (sj ~= "0") then
				-- 十の位と百の位の「一」は表記しない
				if((sj ~= "1") or (j == 1) or (j == 4)) then
					ret = ret .. type_table[tonumber(sj) + 1]
				end
				ret = ret .. type_1k_table[(4 - j) % 4 + 1]
			end
		end
		if (string.sub(num, (i - 1) * 4 + 1, (i - 1) * 4 + 4) ~= "0000") then
			ret = ret .. type_10k_table[m - i + 1]
		end
	end

	-- 0のとき
	if (ret == "") then
		ret = type_table[1]
	end

	return ret
end

-- 数値変換タイプ未定義
local function skk_num_type_n(num, len)
	return num
end

-- 数値変換タイプ1 (全角数字)
local function skk_num_type_1(num, len)
	local ret = ""

	for i = 1, len do
		ret = ret .. skk_num_type1_table[tonumber(string.sub(num, i, i)) + 1]
	end

	return ret
end

-- 数値変換タイプ2 (漢数字、位取りあり)
local function skk_num_type_2(num, len)
	local ret = ""

	for i = 1, len do
		ret = ret .. skk_num_type3_table[tonumber(string.sub(num, i, i)) + 1]
	end

	return ret
end

-- 数値変換タイプ3 (漢数字、位取りなし)
local function skk_num_type_3(num, len)
	local ret = ""

	if (len > (#skk_num_type3_10k_table * 4)) then
		ret = num
	else
		ret = skk_num_to_kanji(num, skk_num_type3_table, skk_num_type3_1k_table, skk_num_type3_10k_table)
	end

	return ret
end

-- 数値変換タイプ5 (漢数字、大字)
local function skk_num_type_5(num, len)
	local ret = ""

	if (len > (#skk_num_type5_10k_table * 4)) then
		ret = num
	else
		ret = skk_num_to_kanji(num, skk_num_type5_table, skk_num_type5_1k_table, skk_num_type5_10k_table)
	end

	return ret
end

-- 数値変換タイプ6 (独自拡張、ローマ数字)
local function skk_num_type_6(num, len)
	local ret = ""
	local n = tonumber(num)

	if (n >= 1 and n <= 3999) then
		ret = skk_num_type6_table_M[((n - (n % 1000)) / 1000) + 1] ..
			skk_num_type6_table_C[(((n - (n % 100)) / 100) % 10) + 1] ..
			skk_num_type6_table_X[(((n - (n % 10)) / 10) % 10) + 1] ..
			skk_num_type6_table_I[(n % 10) + 1]
	end

	return ret
end

-- 数値変換タイプ8 (桁区切り)
local function skk_num_type_8(num, len)
	local ret = ""

	if (len % skk_num_type8_sepnum ~= 0) then
		ret = ret .. string.sub(num, 1, len % skk_num_type8_sepnum) .. skk_num_type8_sep
	end
	for i = 0, (len - len % skk_num_type8_sepnum) / skk_num_type8_sepnum do
		local sepi = len % skk_num_type8_sepnum + i * skk_num_type8_sepnum
		ret = ret .. string.sub(num, sepi + 1, sepi + skk_num_type8_sepnum) .. skk_num_type8_sep
	end
	ret = string.sub(ret, 1, string.len(ret) - string.len(skk_num_type8_sep) - 1)

	return ret
end

-- 数値変換タイプ9
local function skk_num_type_9(num, len)
	local ret = ""

	if (len == 2) then
		ret = skk_num_type1_table[tonumber(string.sub(num, 1, 1)) + 1] ..
			skk_num_type3_table[tonumber(string.sub(num, 2, 2)) + 1]
	else
		ret = num
	end

	return ret
end

-- 数値変換タイプ関数テーブル
local skk_num_type_func_table = {
	skk_num_type_1,
	skk_num_type_2,
	skk_num_type_3,
	skk_num_type_n,
	skk_num_type_5,
	skk_num_type_6,
	skk_num_type_n,
	skk_num_type_8,
	skk_num_type_9
}

-- 数値変換
local function skk_convert_num_type(num, type)
	local ret = ""
	local len = string.len(num)
	local ntype = tonumber(type)

	if ((1 <= ntype) and (ntype <= #skk_num_type_func_table)) then
		ret = skk_num_type_func_table[ntype](num, len)
	else
		ret = num
	end

	return ret
end

-- concat
local function concat(t)
	local ret = ""
	for i, v in ipairs(t) do
		ret = ret .. v
	end
	return ret
end

-- substring
local function substring(t)
	local s = t[1]
	local i = t[2]
	local j = t[3]
	return string.sub(s, i + 1, j)
end

-- make-string
local function make_string(t)
	local ret = ""
	local i = t[1]
	local c = t[2]

	if (string.sub(c, 1, 1) == "?") then
		c = string.sub(c, 2, 2)
	end

	ret = string.rep(string.sub(c, 1, 1), tonumber(i))

	return ret
end

-- string-to-number
local function string_to_number(t)
	return t[1]
end

-- string-to-char
local function string_to_char(t)
	return string.sub(t[1], 1, 1)
end

-- number-to-string
local function number_to_string(t)
	return t[1]
end

-- window-height
local function window_height(t)
	return window_height_value
end

-- window-width
local function window_width(t)
	return window_width_value
end

-- current-time-string
local function current_time_string(t)
	local d = os.date("*t")
	return string.format("%s %s %2d %02d:%02d:%02d %04d",
		skk_gadget_dayofweek_table[d.wday][1], skk_gadget_month_table[d.month][1], d.day,
		d.hour, d.min, d.sec, d.year)
end

-- car
local function car(t)
	if (#t > 0 and #t[1] > 0) then
		return t[1][1]
	end
	return ""
end

-- cdr
local function cdr(t)
	if (#t > 0 and #t[1] > 1) then
		return {table.unpack(t[1], 2)}
	end
	return ""
end

-- convert float to integer (remove suffix ".0")
local function float_to_integer(value)
	local ivalue = math.tointeger(value)
	if ivalue then
		return ivalue
	end
	return value
end

-- 1+
local function plus_1(t)
	local n1 = tonumber(t[1])

	if (not n1) then
		return ""
	end
	return float_to_integer(n1 + 1)
end

-- 1-
local function minus_1(t)
	local n1 = tonumber(t[1])

	if (not n1) then
		return ""
	end
	return float_to_integer(n1 - 1)
end

-- +
local function plus(t)
	local n1 = tonumber(t[1])
	local n2 = tonumber(t[2])

	if (not n1 or not n2) then
		return ""
	end
	return float_to_integer(n1 + n2)
end

-- -
local function minus(t)
	local n1 = tonumber(t[1])
	local n2 = tonumber(t[2])

	if (not n1 or not n2) then
		return ""
	end
	return float_to_integer(n1 - n2)
end

-- *
local function mul(t)
	local n1 = tonumber(t[1])
	local n2 = tonumber(t[2])

	if (not n1 or not n2) then
		return ""
	end
	return float_to_integer(n1 * n2)
end

-- /
local function div(t)
	local n1 = tonumber(t[1])
	local n2 = tonumber(t[2])

	if (not n1 or not n2) then
		return ""
	end
	if (n2 == 0) then
		return ""
	end
	return float_to_integer(n1 / n2)
end

-- %
local function mod(t)
	local n1 = tonumber(t[1])
	local n2 = tonumber(t[2])

	if (not n1 or not n2) then
		return ""
	end
	if (n2 == 0) then
		return ""
	end
	return float_to_integer(n1 % n2)
end

-- skk-version
local function skk_version(t)
	return SKK_VERSION
end

-- skk-server-version
local function skk_server_version(t)
	local v, h = crvmgr.search_skk_server_info()

	if (v == "" or h == "") then
		return ""
	end
	return "SKK SERVER version " .. v .. "running on HOST " .. h
end

-- 西暦元号変換
--    引数    1:西暦文字列, 2:元号表記タイプ(1:漢字/2:英字頭文字), 3:変換タイプ([0-9]),
--            4:区切り, 5:末尾, 6:NOT"元"年, 7:月, 8:日
--    戻り値  変換済み文字列
local function conv_ad_to_gengo(num, gengotype, type, div, tail, not_gannen, month, day)
	local ret = ""

	local year = tonumber(num)

	for i, v in ipairs(skk_gadget_gengo_table) do
		if ((year >= v[1][1] and month == 0 and day == 0) or
			(year > v[1][1]) or
			(year == v[1][1] and month > v[1][2]) or
			(year == v[1][1] and month == v[1][2] and day >= v[1][3])) then
			ret = v[3][tonumber(gengotype)] .. div
			local gengo_year = year - v[1][1] + v[1][4]
			if ((gengo_year == 1) and (not not_gannen)) then
				ret = ret .. "元" .. tail
			else
				ret = ret .. skk_convert_num_type(tostring(gengo_year), type) .. tail
			end
			break
		end
	end

	return ret
end

-- skk-ad-to-gengo
local function skk_ad_to_gengo(t)
	local ret = ""

	local num = skk_num_list[1]
	local gengotype = t[1] + 1
	local divider = t[2]
	local tail = t[3]
	local not_gannen = t[4]

	if (divider == "nil") then
		divider = ""
	end
	if (tail == "nil") then
		tail = ""
	end
	if (not_gannen == "nil") then
		not_gannen = nil
	end

	ret = conv_ad_to_gengo(num, gengotype, "0", divider, tail, not_gannen, 0, 0)

	return ret
end

-- skk-gengo-to-ad
local function skk_gengo_to_ad(t)
	local ret = ""

	local num = skk_num_list[1]
	local head = t[1]
	local tail = t[2]

	local year = tonumber(num)

	for i, v in ipairs(skk_gadget_gengo_table) do
		if (string.sub(skk_henkan_key, 1, string.len(v[2])) == v[2]) then
			if (year >= v[1][4]) then
				local ad_year = year + v[1][1] - v[1][4]
				ret = head .. tostring(ad_year) .. tail
				break
			end
		end
	end

	return ret
end

-- skk-default-current-date
local function skk_default_current_date(t)
	local ret = ""

	if (t == nil) then
		local d = os.date("*t", skk_gadget_time)
		ret = string.format("%s年%s月%s日(%s)",
			conv_ad_to_gengo(tostring(d.year), "1", "1", "", "", false, d.month, d.day),
			skk_convert_num_type(tostring(d.month), "1"),
			skk_convert_num_type(tostring(d.day), "1"),
			skk_gadget_dayofweek_table[d.wday][2])
	else
		local d = os.date("*t", skk_gadget_time)
		local format = t[2]
		local num_type = t[3]
		local gengo = t[4]
		local gengo_index = t[5]
		local month_index = t[6]
		local dayofweek_index = t[7]
		local and_time = t[8]

		if (format == nil or format == "nil") then
			if ((and_time == nil) or (and_time == "nil")) then
				format = "%s年%s月%s日(%s)"
			else
				format = "%s年%s月%s日(%s)%s時%s分%s秒"
			end
		end

		if (num_type == "nil") then
			num_type = "0"
		end

		if (dayofweek_index == "nil") then
			dayofweek_index = "-1"
		end

		local y = ""
		if (gengo == "nil") then
			y = tostring(d.year)
		else
			y = conv_ad_to_gengo(tostring(d.year), tostring(tonumber(gengo_index) + 1),
				num_type, "", "", false, d.month, d.day)
		end

		if ((and_time == nil) or (and_time == "nil")) then
			ret = string.format(format, y,
				skk_convert_num_type(tostring(d.month), num_type),
				skk_convert_num_type(tostring(d.day), num_type),
				skk_gadget_dayofweek_table[d.wday][tonumber(dayofweek_index) + 2])
		else
			ret = string.format(format, y,
				skk_convert_num_type(tostring(d.month), num_type),
				skk_convert_num_type(tostring(d.day), num_type),
				skk_gadget_dayofweek_table[d.wday][tonumber(dayofweek_index) + 2],
				skk_convert_num_type(string.format("%02d", d.hour), num_type),
				skk_convert_num_type(string.format("%02d", d.min), num_type),
				skk_convert_num_type(string.format("%02d", d.sec), num_type))
		end
	end

	return ret
end

-- skk-current-date
local function skk_current_date(t)
	local ret = ""

	local pp_function = t[1]
	-- local format = [2]
	-- local and_time = [3]

	if (pp_function == nil) then
		ret = skk_default_current_date(nil)
	else
		ret = eval_table(pp_function)
	end

	return ret
end

-- skk-relative-date
local function skk_relative_date(t)
	local ret = ""

	local pp_function = t[1]
	-- local format = t[2]
	-- local and_time = t[3]
	local ymd = t[4]
	local diff = t[5]

	local d = os.date("*t", skk_gadget_time)

	if (ymd == ":yy") then
		d["year"] = d["year"] + tonumber(diff)
	elseif (ymd == ":mm") then
		d["month"] = d["month"] + tonumber(diff)
	elseif (ymd == ":dd") then
		d["day"] = d["day"] + tonumber(diff)
	else
	end

	local skk_gadget_time_bak = skk_gadget_time
	skk_gadget_time = os.time(d)

	if (pp_function == "nil") then
		ret = skk_default_current_date(nil)
	else
		ret = eval_table(pp_function)
	end

	skk_gadget_time = skk_gadget_time_bak

	return ret
end

-- skk-gadget-units-conversion
local function skk_gadget_units_conversion(t)
	local ret = ""

	local unit_from = t[1]
	local number = t[2]
	local unit_to = t[3]

	local value_from = skk_gadget_unit_table[unit_from]
	if (value_from) then
		local value_to = value_from[unit_to]
		if (value_to) then
			ret = tostring(float_to_integer(tonumber(number) * value_to)) .. unit_to
		end
	end

	return ret
end

-- skk-omikuji
local function skk_omikuji(t)
	return skk_gadget_omikuji_table[math.random(1, #skk_gadget_omikuji_table)]
end

-- 関数テーブル
local skk_gadget_func_table_org = {
	{"concat", concat},
	{"substring", substring},
	{"make-string", make_string},
	{"string-to-number", string_to_number},
	{"string-to-char", string_to_char},
	{"number-to-string", number_to_string},
	{"window-width", window_width},
	{"window-height", window_height},
	{"current-time-string", current_time_string},
	{"car", car},
	{"cdr", cdr},
	{"1+", plus_1},
	{"1-", minus_1},
	{"+", plus},
	{"-", minus},
	{"*", mul},
	{"/", div},
	{"%", mod},
	{"skk-version", skk_version},
	{"skk-server-version", skk_server_version},
	{"skk-ad-to-gengo", skk_ad_to_gengo},
	{"skk-gengo-to-ad", skk_gengo_to_ad},
	{"skk-default-current-date", skk_default_current_date},
	{"skk-current-date", skk_current_date},
	{"skk-relative-date", skk_relative_date},
	{"skk-gadget-units-conversion", skk_gadget_units_conversion},
	{"skk-omikuji", skk_omikuji},
}
local skk_gadget_func_table = {}
for i, v in ipairs(skk_gadget_func_table_org) do
	skk_gadget_func_table[v[1]] = v[2]
end

-- 文字列パース
local function parse_string(s)
	local ret = ""
	local bsrep = "\u{f05c}"

	s = string.gsub(s, "^\"(.*)\"$", "%1")

	-- バックスラッシュ
	s = string.gsub(s, "\\\\", bsrep)
	-- 二重引用符
	s = string.gsub(s, "\\\"", "\"")
	-- 空白文字
	s = string.gsub(s, "\\s", "\x20")
	-- 制御文字など
	s = string.gsub(s, "\\[abtnvfred ]", "")
	-- 8進数表記の文字
	s = string.gsub(s, "\\[0-3][0-7][0-7]",
		function(n)
			local c =
				tonumber(string.sub(n, 2, 2)) * 64 +
				tonumber(string.sub(n, 3, 3)) * 8 +
				tonumber(string.sub(n, 4, 4))
			if (c >= 0x20 and c <= 0x7E) then
				return string.char(c)
			end
			return ""
		end)
	-- 意味なしエスケープ
	s = string.gsub(s, "\\", "")
	-- バックスラッシュ
	s = string.gsub(s, bsrep, "\\")

	ret = s

	return ret
end

-- S式をテーブル表記に変換
function convert_s_to_table(s)
	local ret = ""
	local e = ""
	local q = 0
	local d = 0
	local c = ""
	local r = ""

	for i = 1, string.len(s) do
		c = string.sub(s, i, i)
		r = string.sub(ret, -1)

		if (c == "\"" and q == 0) then
			q = 1
		elseif (c == "\"" and q == 1 and d == 0) then
			q = 0
		end

		if (q == 0) then
			if (c == "(") then
				if (ret ~= "") then
					ret = ret .. ","
				end
				ret = ret .. "{"
			elseif (c == ")" or c == "\x20") then
				if (e ~= "") then
					if (r ~= "{") then
						ret = ret .. ","
					end

					e = string.gsub(e, "\"", "\\\"")
					ret = ret .. "\"" .. e .. "\""
					e = ""
				end
			else
				e = e .. c
			end

			if (c == ")") then
				ret = ret .. "}"
			end
		else
			e = e .. c
			if (c == "\\") then
				e = e .. c
				d = d ~ 1
			else
				d = 0
			end
		end
	end

	return ret
end

-- テーブル評価
function eval_table(x)
	local argtype = type(x)
	if (argtype == "table" and #x > 0) then
		if (x[1] == "lambda") then
			if (#x >= 3 and x[3]) then
				return x[3]
			else
				return ""
			end
		end

		local func = skk_gadget_func_table[x[1]]
		if (func) then
			local arg = {table.unpack(x, 2)}
			for i, v in ipairs(arg) do
				local vv = skk_gadget_variable_table[v]
				if (vv) then
					v = vv
				end
				arg[i] = eval_table(v)
			end

			return func(arg)
		end
	elseif (argtype == "function") then
		return x()
	elseif (argtype == "string") then
		return parse_string(x)
	end

	return ""
end

-- skk-ignore-dic-word
local function skk_ignore_dic_word(candidates)
	local ret = ""
	local sca = ""
	local ignore_word_table = {}

	for ca in string.gmatch(candidates, "([^/]+)") do
		local c = string.gsub(ca, ";.+", "")
		local word = string.gsub(c, "^%(%s*skk%-ignore%-dic%-word%s+\"(.+)\"%s*%)$", "%1")
		if (word ~= c) then
			ignore_word_table[word] = true
		else
			sca = sca .. "/" .. ca
		end
	end

	if (sca == candidates) then
		return candidates
	end

	for ca in string.gmatch(sca, "([^/]+)") do
		local c = string.gsub(ca, ";.+", "")
		if (not ignore_word_table[c]) then
			ret = ret .. "/" .. ca
		end
	end

	return ret
end

-- 候補全体を数値変換
local function skk_convert_num(key, candidate)
	local ret = ""
	local keytemp = key

	ret = string.gsub(candidate, "#%d+",
		function(type)
			local num = string.match(keytemp, "%d+")
			keytemp = string.gsub(keytemp, "%d+", "#", 1)
			if (num) then
				return skk_convert_num_type(num, string.sub(type, 2))
			else
				return type
			end
		end)

	return ret
end

-- 実行変換
local function skk_convert_gadget(key, candidate)

	-- skk-henkan-key
	skk_henkan_key = key

	-- skk-num-list
	skk_num_list = {}
	string.gsub(key, "%d+",
		function(n)
			table.insert(skk_num_list, n)
		end)

	-- 日付時刻
	skk_gadget_time = os.time()

	-- 乱数
	math.randomseed(skk_gadget_time)

	return eval_table(load("return " .. convert_s_to_table(candidate))())
end

-- 候補変換処理
local function skk_convert_candidate(key, candidate, okuri)
	local ret = ""
	local temp = candidate

	-- xtu/xtsuで「っ」を送り仮名にしたとき送りローマ字「t」を有効にする
	if (okuri == "っ" and string.sub(key, string.len(key)) == "x") then
		return candidate
	end

	-- 数値変換
	if (enable_skk_convert_num) then
		if (string.find(key, "%d+") and string.find(temp, "#%d")) then
			temp = skk_convert_num(key, temp)
			ret = temp
		end
	end

	-- 実行変換
	if (enable_skk_convert_gadget) then
		if (string.match(temp, "^%(.+%)$")) then
			temp = skk_convert_gadget(key, temp)
			ret = temp
		end

		-- concat関数で"/"関数が\057にエスケープされる為再度実行する
		if (string.match(temp, "^%(.+%)$")) then
			temp = skk_convert_gadget(key, temp)
			-- 正常な実行結果であれば上書きする
			if (temp ~= "") then
				ret = temp
			end
		end
	end

	return ret
end

-- 見出し語変換処理
local function skk_convert_key(key, okuri)
	local ret = ""

	-- xtu/xtsuで「っ」を送り仮名にしたとき送りローマ字を「t」に変換する
	if (okuri == "っ" and string.sub(key, string.len(key)) == "x") then
		return string.sub(key, 1, string.len(key) - 1) .. "t"
	end

	-- 文字コード表記変換のとき見出し語変換しない
	local cccplen = string.len(charcode_conv_prefix)
	if (cccplen < string.len(key) and string.sub(key, 1, cccplen) == charcode_conv_prefix) then
		return ""
	end

	-- 数値変換
	if (enable_skk_convert_num) then
		if (string.find(key, "%d+")) then
			ret = string.gsub(key, "%d+", "#")
		end
	end

	return ret
end

-- 辞書検索処理
--   検索結果のフォーマットはSKK辞書の候補部分と同じ
--   "/<C1><;A1>/<C2><;A2>/.../<Cn><;An>/\n"
local function skk_search(key, okuri)
	local ret = ""

	-- ユーザー辞書検索
	ret = ret .. crvmgr.search_user_dictionary(key, okuri)

	-- SKK辞書検索
	ret = ret .. crvmgr.search_skk_dictionary(key, okuri)

	-- SKK辞書サーバー検索
	ret = ret .. crvmgr.search_skk_server(key)

	if (okuri == "") then
		-- Unicodeコードポイント変換
		ret = ret .. crvmgr.search_unicode(key)

		-- JIS X 0213面区点番号変換
		ret = ret .. crvmgr.search_jisx0213(key)

		-- JIS X 0208区点番号変換
		ret = ret .. crvmgr.search_jisx0208(key)

		local cccplen = string.len(charcode_conv_prefix)
		if (cccplen < string.len(key) and string.sub(key, 1, cccplen) == charcode_conv_prefix) then
			local subkey = string.sub(key, cccplen + 1)

			-- 文字コード表記変換
			ret = ret .. crvmgr.search_character_code(subkey)
		end
	end

	-- 余計な"/\n"を削除
	ret = string.gsub(ret, "/\n/", "/")

	return ret
end



--[[

	C側から呼ばれる関数群

--]]

-- 辞書検索
function lua_skk_search(key, okuri)

	-- skk-search-sagyo-henkaku (t:true/anything:false)
	if (enable_skk_search_sagyo_only) then
		if (okuri ~= "" and string.find("さしすせサシスセ", okuri) == nil and
			string.match(string.sub(key, -1), "[a-z]") == nil) then
			return ""
		end
	end

	local ret =  skk_search(key, okuri)

	-- skk-ignore-dic-word
	if (enable_skk_ignore_dic_word) then
		ret = skk_ignore_dic_word(ret)
	end

	return ret
end

-- 補完
function lua_skk_complement(key)
	return crvmgr.complement(key)
end

-- 見出し語変換
function lua_skk_convert_key(key, okuri)
	return skk_convert_key(key, okuri)
end

-- 候補変換
function lua_skk_convert_candidate(key, candidate, okuri)
	return skk_convert_candidate(key, candidate, okuri)
end

-- 辞書追加
function lua_skk_add(okuriari, key, candidate, annotation, okuri)

	--[[
	-- 例) 送りありのときユーザー辞書に登録しない
	if (okuriari) then
		return
	end
	--]]

	--[[
	-- 例) 送り仮名ブロックを登録しない
	if (okuriari) then
		okuri = ""
	end
	--]]

	--[[
	-- 例) Unicodeコードポイント変換のときユーザー辞書に登録しない
	if not (okuriari) then
		if (string.match(key, "^U%+[0-9A-F]+$") or string.match(key, "^u[0-9a-f]+$")) then
			if (string.match(key, "^U%+[0-9A-F][0-9A-F][0-9A-F][0-9A-F]$") or			-- U+XXXX
				string.match(key, "^U%+[0-9A-F][0-9A-F][0-9A-F][0-9A-F][0-9A-F]$") or	-- U+XXXXX
				string.match(key, "^U%+10[0-9A-F][0-9A-F][0-9A-F][0-9A-F]$") or			-- U+10XXXX
				string.match(key, "^u[0-9a-f][0-9a-f][0-9a-f][0-9a-f]$") or				-- uxxxx
				string.match(key, "^u[0-9a-f][0-9a-f][0-9a-f][0-9a-f][0-9a-f]$") or		-- uxxxxx
				string.match(key, "^u10[0-9a-f][0-9a-f][0-9a-f][0-9a-f]$")) then		-- u10xxxx
				return
			end
		end
	end
	--]]

	--[[
	-- 例) JIS X 0213面区点番号変換のときユーザー辞書に登録しない
	if not (okuriari) then
		if (string.match(key, "^[12]%-[0-9][0-9]%-[0-9][0-9]$")) then
			if (string.match(key, "^[12]%-0[1-9]%-0[1-9]$") or			-- [12]-01-01 - [12]-09-94
				string.match(key, "^[12]%-0[1-9]%-[1-8][0-9]$") or		-- 〃
				string.match(key, "^[12]%-0[1-9]%-9[0-4]$") or			-- 〃
				string.match(key, "^[12]%-[1-8][0-9]%-0[1-9]$") or		-- [12]-10-01 - [12]-89-94
				string.match(key, "^[12]%-[1-8][0-9]%-[1-8][0-9]$") or	-- 〃
				string.match(key, "^[12]%-[1-8][0-9]%-9[0-4]$") or		-- 〃
				string.match(key, "^[12]%-9[0-4]%-0[1-9]$") or			-- [12]-90-01 - [12]-94-94
				string.match(key, "^[12]%-9[0-4]%-[1-8][0-9]$") or		-- 〃
				string.match(key, "^[12]%-9[0-4]%-9[0-4]$")) then		-- 〃
				return
			end
		end
	end
	--]]

	--[[
	-- 例) JIS X 0208区点番号変換のときユーザー辞書に登録しない
	if not (okuriari) then
		if (string.match(key, "^[0-9][0-9]%-[0-9][0-9]$")) then
			if (string.match(key, "^0[1-9]%-0[1-9]$") or			-- 01-01 - 09-94
				string.match(key, "^0[1-9]%-[1-8][0-9]$") or		-- 〃
				string.match(key, "^0[1-9]%-9[0-4]$") or			-- 〃
				string.match(key, "^[1-8][0-9]%-0[1-9]$") or		-- 10-01 - 89-94
				string.match(key, "^[1-8][0-9]%-[1-8][0-9]$") or	-- 〃
				string.match(key, "^[1-8][0-9]%-9[0-4]$") or		-- 〃
				string.match(key, "^9[0-4]%-0[1-9]$") or			-- 90-01 - 94-94
				string.match(key, "^9[0-4]%-[1-8][0-9]$") or		-- 〃
				string.match(key, "^9[0-4]%-9[0-4]$")) then			-- 〃
				return
			end
		end
	end
	--]]

	--[[
	-- 例) 文字コード表記変換のときユーザー辞書に登録しない
	if not (okuriari) then
		local cccplen = string.len(charcode_conv_prefix)
		if (cccplen < string.len(key) and string.sub(key, 1, cccplen) == charcode_conv_prefix) then
			return
		end
	end
	--]]

	crvmgr.add(okuriari, key, candidate, annotation, okuri)
end

-- 辞書削除
function lua_skk_delete(okuriari, key, candidate)
	crvmgr.delete(okuriari, key, candidate)
end

-- 辞書保存
function lua_skk_save()
	crvmgr.save()
end
