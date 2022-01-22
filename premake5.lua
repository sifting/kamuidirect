workspace "kamui"
	configurations {"Debug", "Release"}
	location "build"
	targetname "KAMUI"
	targetprefix ""
	targetextension ".DLL"
	targetdir "."
	debugdir "."
	filter "language:C"
		toolset "gcc"
		buildoptions {"-std=gnu11 -pedantic -Wall -m32"}
		--KAMUI uses stdcall, but without the decoration,
		--so... it's not... standard...
		linkoptions {"-Wl,--kill-at"}
	filter "configurations:Debug"
		defines {"DEBUG"}
		symbols "On"
	filter "configurations:Release"
		defines {"NDEBUG"}
		vectorextensions "Default"
		optimize "Speed"
	project "kamui"
		language "C"
		kind "SharedLib"
		includedirs {"pvr/include"}
		syslibdirs {"pvr"}
		files {
			"src/**.h",
			"src/**.c",
		}
		links {"pvr", "d3d9", "d3dcompiler"}
