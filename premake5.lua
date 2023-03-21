workspace "Dusty"
    architecture "x64"

    configurations {
      "Debug",
      "Development",
      "Release"
    }
    startproject "Ash"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "Ash"
    location "Ash" -- everything will be relative to this path
    kind "ConsoleApp"
    language "C++"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

    pchheader "pch.h"
    pchsource "Ash/src/pch.cpp"

    files {
        "%{prj.name}/src/**.h", -- ** means to recursively search through subfolders
        "%{prj.name}/src/**.cpp"
    }

    includedirs {
        "%{prj.name}/src",
        "%{prj.name}/lib/spdlog/include",
        "%{prj.name}/lib/glew/include",
        "%{prj.name}/lib/sdl/include",
        "%{prj.name}/lib/glm"
    }

    filter "system:windows" -- windows specific flags
        cppdialect "C++17"
        staticruntime "On"
        systemversion "latest" -- version of the Windows SDK

        defines {
            "DUSTY_PLATFORM_WINDOWS"
        }


    filter "configurations:Debug"
        defines "DUSTY_DEBUG"
        symbols "On"

    filter "configurations:Release"
        defines "DUSTY_RELEASE"
        optimize "On"

    filter "configurations:Development"
        defines "DUSTY_DEVELOPMENT"
        optimize "On"