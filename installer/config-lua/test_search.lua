package.path = ".\\?.lua;" .. package.path
package.cpath = ".\\?.dll;" .. package.cpath

crvmgr = require("test_c")
require("init")


print(lua_skk_search("かくてい", ""))
enable_skk_ignore_dic_word = true
print(lua_skk_search("かくてい", ""))
enable_skk_ignore_dic_word = false


print(lua_skk_search("きr", "る"))
enable_skk_ignore_dic_word = true
print(lua_skk_search("きr", "る"))
enable_skk_ignore_dic_word = false
