workspace "pvr"
	configurations {"Debug", "Release"}
	location "build"
	targetdir "."
	debugdir "."
	filter "language:C"
		toolset "gcc"
		buildoptions {"-std=c11 -pedantic -Wall"}
	filter "configurations:Debug"
		defines {"DEBUG"}
		symbols "On"
	filter "configurations:Release"
		defines {"NDEBUG"}
		vectorextensions "Default"
		optimize "Speed"
	project "pvr"
		language "C"
		kind "StaticLib"
		includedirs {"include"}
		files {
			"include/pvr.h",
			"src/pvr.c",
		}
	project "pvrc"
		language "C"
		kind "ConsoleApp"
		includedirs {"include"}
		files {
			"src/pvrc.c",
		}
		links {"pvr", "png", "zlib"}
