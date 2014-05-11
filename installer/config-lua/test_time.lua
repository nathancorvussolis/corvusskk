package.path = ".\\?.lua;" .. package.path
package.cpath = ".\\?.dll;" .. package.cpath

crvmgr = require("test_c")
require("init")



el_test_time_table = {

{"today",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 'gengo 0 0 0)))"},
{"today",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s-%s-%s(%s)\" 0 nil 0 0 nil)))"},
{"today",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 'gengo 1 0 0)))"},
{"today",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 3 'gengo 0 0 0)))"},
{"today",	"(skk-current-date)"},

{"today",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 'gengo 0 0 0 t)))"},
{"today",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s-%s-%s(%s)%s:%s:%s\" 0 nil 0 0 nil t)))"},
{"today",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 'gengo 1 0 0 t)))"},
{"today",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 3 'gengo 0 0 0 t)))"},

{"3かげつご",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年%s月\" 0 nil 0 0 0)) nil nil :mm #0)"},
{"3かげつまえ",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年%s月\" 0 nil 0 0 0)) nil nil :mm -#0)"},
{"3にちご",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 nil 0 0 0)) nil nil :dd #0)"},
{"3にちまえ",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 nil 0 0 0)) nil nil :dd -#0)"},
{"3ねんご",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年\" 0 nil 0 0 0)) nil nil :yy #0)"},
{"3ねんまえ",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年\" 0 nil 0 0 0)) nil nil :yy -#0)"},
{"おととし",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年\" 0 nil 0 0 0)) nil nil :yy -2)"},
{"きょねん",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年\" 0 nil 0 0 0)) nil nil :yy -1)"},
{"せんせんげつ",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年%s月\" 0 nil 0 0 0)) nil nil :mm -2)"},
{"せんげつ",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年%s月\" 0 nil 0 0 0)) nil nil :mm -1)"},
{"おととい",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 nil 0 0 0)) nil nil :dd -2)"},
{"きのう",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 nil 0 0 0)) nil nil :dd -1)"},
{"きょう",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 nil 0 0 0)) nil nil :dd 0)"},
{"こんげつ",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年%s月\" 0 nil 0 0 0)))"},
{"ことし",	"(skk-current-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年\" 0 nil 0 0 0)))"},
{"あす",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 nil 0 0 0)) nil nil :dd 1)"},
{"あさって",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information nil 0 nil 0 0 0)) nil nil :dd 2)"},
{"らいげつ",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年%s月\" 0 nil 0 0 0)) nil nil :mm 1)"},
{"さらいげつ",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年%s月\" 0 nil 0 0 0)) nil nil :mm 2)"},
{"らいねん",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年\" 0 nil 0 0 0)) nil nil :yy 1)"},
{"さらいねん",	"(skk-relative-date (lambda (date-information format gengo and-time) (skk-default-current-date date-information \"%s年\" 0 nil 0 0 0)) nil nil :yy 2)"},

{"あした！",	"(skk-relative-date nil nil nil :dd 1)"},


{"へいせい1ねん",	"(skk-gengo-to-ad \"西暦\" \"年\")"},
{"へいせい1ねん",	"(skk-gengo-to-ad \"\" \"年\")"},
{"しょうわ1ねん",	"(skk-gengo-to-ad \"西暦\" \"年\")"},
{"しょうわ1ねん",	"(skk-gengo-to-ad \"\" \"年\")"},
{"たいしょう1ねん",	"(skk-gengo-to-ad \"西暦\" \"年\")"},
{"たいしょう1ねん",	"(skk-gengo-to-ad \"\" \"年\")"},
{"めいじ1ねん",		"(skk-gengo-to-ad \"西暦\" \"年\")"},
{"めいじ1ねん",		"(skk-gengo-to-ad \"\" \"年\")"},
{"めいじ5ねん",		"(skk-gengo-to-ad \"西暦\" \"年\")"},
{"めいじ5ねん",		"(skk-gengo-to-ad \"\" \"年\")"},
{"めいじ6ねん",		"(skk-gengo-to-ad \"西暦\" \"年\")"},
{"めいじ6ねん",		"(skk-gengo-to-ad \"\" \"年\")"},

{"せいれき1988ねん",	"(skk-ad-to-gengo 0 nil \"年\" t)"},
{"せいれき1988ねん",	"(skk-ad-to-gengo 1 nil \"年\" t)"},
{"せいれき1988ねん",	"(skk-ad-to-gengo 0 nil \"年\")"},
{"せいれき1988ねん",	"(skk-ad-to-gengo 1 nil \"年\")"},
{"せいれき1989ねん",	"(skk-ad-to-gengo 0 nil \"年\" t)"},
{"せいれき1989ねん",	"(skk-ad-to-gengo 1 nil \"年\" t)"},
{"せいれき1989ねん",	"(skk-ad-to-gengo 0 nil \"年\")"},
{"せいれき1989ねん",	"(skk-ad-to-gengo 1 nil \"年\")"},

}



for i, v in ipairs(el_test_time_table) do
	print("<= \"" .. v[1] .. "\" \"".. v[2] .. "\"")
	local s = lua_skk_convert_candidate(v[1], v[2])

	if (s) then
		print("=> \"" .. s .. "\"\n")
	else
		print("=> nil\n")
	end
end
