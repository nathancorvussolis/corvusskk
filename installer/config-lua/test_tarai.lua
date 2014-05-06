
function tarai(x, y, z)
	if (x <= y) then
		return y
	else
		return tarai(tarai(x - 1, y, z), tarai(y - 1, z, x), tarai(z - 1, x, y))
	end
end

print(tarai(18, 12, 8))
