package.path = ".\\?.lua;" .. package.path
package.cpath = ".\\?.dll;" .. package.cpath

crvmgr = require("test_c")
require("init")


key = "かくてい"
okuri = ""
print("key : \"" .. key .. "\" okuri : \"" .. okuri .. "\"\n")
print("   enable_skk_ignore_dic_word : false")
print("   " .. lua_skk_search(key, okuri))
enable_skk_ignore_dic_word = true
print("   enable_skk_ignore_dic_word : true")
print("   " .. lua_skk_search(key, okuri))
enable_skk_ignore_dic_word = false


key = "きr"
okuri = "る"
print("key : \"" .. key .. "\" okuri : \"" .. okuri .. "\"\n")
print("   enable_skk_ignore_dic_word : false")
print("   " .. lua_skk_search(key, okuri))
enable_skk_ignore_dic_word = true
print("   enable_skk_ignore_dic_word : true")
print("   " .. lua_skk_search(key, okuri))
enable_skk_ignore_dic_word = false


key = "かx"
okuri = "っ"
print("key : \"" .. key .. "\" okuri : \"" .. okuri .. "\"\n")
print("   > " .. lua_skk_convert_key(key, okuri) .. "\n")


key = "きr"
okuri = "る"
print("key : \"" .. key .. "\" okuri : \"" .. okuri .. "\"\n")
print("   enable_skk_search_sagyo_only : true")
print("   " .. lua_skk_search(key, okuri) .. "\n")

key = "きr"
okuri = "る"
enable_skk_search_sagyo_only = false
print("   enable_skk_search_sagyo_only : false")
print("   " .. lua_skk_search(key, okuri))
-- //送りあり変換で送りなし候補も検索する : ON
key = "き"
okuri = ""
print("   " .. lua_skk_search(key, okuri) .. "\n")
enable_skk_search_sagyo_only = true


print("check results by yourself.")
