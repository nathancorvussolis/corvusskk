
SKK_VERSION="CorvusSKK X.Y.Z"

local _M = {}

function _M.search_user_dictionary(key, okuri)
	return ""
end

function _M.search_skk_dictionary(key)
	local ret = ""

	if (key == "きr") then
		ret = "/切;紙を-/着;服を-/斬;人を-/伐;木を-/剪;盆栽を-/截;布地を-/(skk-ignore-dic-word \"斬\")/\n"
	elseif (key == "かくてい") then
		ret = "/(skk-ignore-dic-word \"確定\");yyy/各停;zzz/確定;xxx/\n"
	elseif (key == "うんどうs") then
		ret = "/運動/\n"
	elseif (key == "き") then
		ret = "/気/機/器/木/\n"
	end

	return ret
end

function _M.search_skk_server(key)
	return ""
end

function _M.search_skk_server_info()
	return "", ""
end

function _M.search_unicode(key)
	return ""
end

function _M.search_jisx0213(key)
	return ""
end

function _M.search_jisx0208(key)
	return ""
end

function _M.search_character_code(key)
	return ""
end

function _M.complement(key)
	return "/きあい/きけん/きき/きかい/\n"
end

function _M.add(okuriari, key, candidate, annotation, okuri)
	return
end

function _M.delete(okuriari, key, candidate)
	return
end

function _M.save()
	return
end

return _M
