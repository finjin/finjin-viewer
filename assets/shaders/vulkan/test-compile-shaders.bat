set GLSL_COMPILER_PATH=C:\VulkanSDK\1.0.46.0\Bin32\glslangvalidator
@rem set OUTPUT_PATH=..\compiled\shaders\vulkan\
set OUTPUT_PATH=..\..\..\project\windows-win32-vs\x64\Vulkan-Win32-Debug\app\shaders-vulkan

@rem Preprocess shader code
@mkdir output
@CL /P /EP /C ForwardShaders.vert /DCOMPILING_VS=1 /DMAP_DIFFUSE_ENABLED=1 /DDIRECTIONAL_LIGHT_COUNT=1 /Fioutput\ForwardShaders.vert
@C:\Python34\python fix_macros.py output\ForwardShaders.vert

@CL /P /EP /C ForwardShaders.frag /DCOMPILING_FS=1 /DMAP_DIFFUSE_ENABLED=1 /DDIRECTIONAL_LIGHT_COUNT=1 /Fioutput\ForwardShaders.frag
@C:\Python34\python fix_macros.py output\ForwardShaders.frag

@rem Compile preprocessed shader code
@rem mkdir %OUTPUT_PATH%
@rem Add -H to output (to stdout) a human readable form of SPIR-V
"%GLSL_COMPILER_PATH%" -e main -V output\ForwardShaders.vert -o "%OUTPUT_PATH%\shaders.vert.spv"
"%GLSL_COMPILER_PATH%" -e main -V output\ForwardShaders.frag -o "%OUTPUT_PATH%\shaders.frag.spv"