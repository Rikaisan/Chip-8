---@diagnostic disable: undefined-global
workspace "Chip8Emu"
    architecture "x64"
    configurations { "Debug", "Release" }

OUTPUTDIR = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Chip8Emu"
    location "Chip8Emu"
    kind "WindowedApp"
    language "C++"
    cppdialect "c++20"

    targetdir("bin/" .. OUTPUTDIR .. "/%{prj.name}")
    objdir("bin-int/" .. OUTPUTDIR .. "/%{prj.name}")

    files {
        "%{prj.name}/src/**.h",
        "%{prj.name}/src/**.hpp",
        "%{prj.name}/src/**.cpp"
    }
    includedirs {
        "vendor/include/"
    }
    libdirs {
        "vendor/lib/**"
    }
    links {
        "winmm",
        "raylib"
    }

    filter "system:windows"
        staticruntime "On"
        systemversion "latest"
        
        filter "configurations:Release"
        entrypoint "mainCRTStartup"
        buildoptions "/MD"
        defines { "NDEBUG" }
        optimize "On"

        filter "configurations:Debug"
        defines { "_DEBUG" }
        buildoptions "/MDd"
        linkoptions "/SUBSYSTEM:CONSOLE /NODEFAULTLIB:MSVCRT"
        symbols "On"
