set PREPROCESSOR_PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\cl
set GLSL_COMPILER_PATH=C:\VulkanSDK\1.0.49.0\Bin32\glslangvalidator
@rem set OUTPUT_PATH=..\compiled\shaders\vulkan\
set OUTPUT_PATH=..\..\..\project\windows-win32-vs\x64\Vulkan-Win32-Debug\app\shaders-vulkan
set PYTHON_PATH=C:\Python34\python


@rem Preprocess shader code
@mkdir output
@"%PREPROCESSOR_PATH%" /P /EP /C ForwardShaders.vert /DCOMPILING_VS=1 /DMAP_DIFFUSE_ENABLED=1 /DDIRECTIONAL_LIGHT_COUNT=1 /Fioutput\ForwardShaders.vert
@"%PYTHON_PATH%" fix_macros.py output\ForwardShaders.vert

@"%PREPROCESSOR_PATH%" /P /EP /C ForwardShaders.frag /DCOMPILING_FS=1 /DMAP_DIFFUSE_ENABLED=1 /DDIRECTIONAL_LIGHT_COUNT=1 /Fioutput\ForwardShaders.frag
@"%PYTHON_PATH%" fix_macros.py output\ForwardShaders.frag

@"%PREPROCESSOR_PATH%" /P /EP /C CommonShaders.vert /DCOMPILING_VS=1 /DMAP_DIFFUSE_ENABLED=1 /DDIRECTIONAL_LIGHT_COUNT=1 /Fioutput\CommonShaders.vert
@"%PYTHON_PATH%" fix_macros.py output\CommonShaders.vert

@"%PREPROCESSOR_PATH%" /P /EP /C CommonShaders.frag /DCOMPILING_FS=1 /DMAP_DIFFUSE_ENABLED=1 /DDIRECTIONAL_LIGHT_COUNT=1 /Fioutput\CommonShaders.frag
@"%PYTHON_PATH%" fix_macros.py output\CommonShaders.frag


@rem Compile preprocessed shader code
@rem mkdir %OUTPUT_PATH%
@rem Add -H to output (to stdout) a human readable form of SPIR-V
@"%GLSL_COMPILER_PATH%" -e main -V output\ForwardShaders.vert -o "%OUTPUT_PATH%\shaders.vert.spv"
@"%GLSL_COMPILER_PATH%" -e main -V output\ForwardShaders.frag -o "%OUTPUT_PATH%\shaders.frag.spv"

@"%GLSL_COMPILER_PATH%" -e main -V output\CommonShaders.vert -o "%OUTPUT_PATH%\common-shaders-fullscreen-quad.vert.spv"
@"%GLSL_COMPILER_PATH%" -e main -V output\CommonShaders.frag -o "%OUTPUT_PATH%\common-shaders-fullscreen-quad.frag.spv"
