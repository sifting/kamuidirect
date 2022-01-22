local atoms = {
	"KM_PARAMTYPE",
    "KM_LISTTYPE",
	"KM_STRIPLENGTH",
	"KM_USERCLIPMODE",
    "KM_COLORTYPE",
    "KM_UVFORMAT",
    "KM_DEPTHMODE",
    "KM_CULLINGMODE",
    "KM_SCREENCOORDINATION",
    "KM_SHADINGMODE",
	"KM_MODIFIER",
    "KM_ZWRITEDISABLE",
    "KM_SRCBLENDINGMODE",
    "KM_DSTBLENDINGMODE",
    "KM_SRCSELECT",
    "KM_DSTSELECT",
	"KM_FOGMODE",
    "KM_USESPECULAR",
    "KM_USEALPHA",
    "KM_IGNORETEXTUREALPHA",
    "KM_CLAMPUV",
    "KM_FLIPUV",
    "KM_FILTERMODE",
    "KM_SUPERSAMPLE",
    "KM_MIPMAPDADJUST",
    "KM_TEXTURESHADINGMODE",
    "KM_COLORCLAMP",
    "KM_PALETTEBANK", 
    "KM_DCALCEXACT"
}

for i, v in ipairs (atoms) do
	print (string.format ("if (state&%s)\n{\n}", v))
end

for i, v in ipairs (atoms) do
	print (string.format ("if (state&%s) strcat (buf, \"%s \");", v, v))
end

