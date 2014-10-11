print("System Code Page : " .. u8w.getacp())

function test_codepage(codepage, s)
	local f, a, a1, s1

	io.stdout:write("CP" .. codepage .. " ")

	a = u8w.utf8toacp(s, codepage)
	f = io.open(s .. ".TXT", "w")
	f:write(a)
	f:close()

	f = io.open(s .. ".TXT", "r")
	a1 = f:read()
	f:close()
	s1 = u8w.acptoutf8(a1, codepage)

	assert(s == s1)
	print("OK.")

	io.stdout:write("Push enter key...")
	io.stdin:read()
	os.remove(s .. ".TXT")
end

test_codepage(932, "CP932 Japanese 日本語 こんにちは")
test_codepage(936, "CP936 Simplified-Chinese 简体中文 您好")
test_codepage(949, "CP949 Korean 한국어 안녕하세요")
test_codepage(950, "CP950 Traditional-Chinese 繁體中文 您好")

test_codepage(1251, "CP1251 Cyrillic кириллица АБВабв")
test_codepage(1252, "CP1252 Latin-1 ÇçÆæŒœß")
test_codepage(1253, "CP1253 Greek Έλληνες ΑΒΓαβγ")
