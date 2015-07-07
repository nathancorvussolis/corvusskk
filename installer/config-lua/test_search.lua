package.path = ".\\?.lua;" .. package.path
package.cpath = ".\\?.dll;" .. package.cpath

crvmgr = require("test_c")
require("init")



print("> " .. lua_skk_search("かくてい", ""))
enable_skk_ignore_dic_word = true
print("> " .. lua_skk_search("かくてい", ""))
enable_skk_ignore_dic_word = false

print("> " .. lua_skk_search("きr", "る"))
enable_skk_ignore_dic_word = true
print("> " .. lua_skk_search("きr", "る"))
enable_skk_ignore_dic_word = false

print(lua_skk_convert_key("かx", "っ") .. "\n")

print("> " .. lua_skk_search("うんどうs", "す"))
print("> " .. lua_skk_search("き", "る"))
enable_skk_search_sagyo_only = false
print("> " .. lua_skk_search("き", "る"))
enable_skk_search_sagyo_only = true
