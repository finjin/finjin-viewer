set COMPILER_PATH=C:\Program Files (x86)\Windows Kits\10\bin\x64\fxc
set OUTPUT_PATH=C:\git\nedelman\finjin-viewer\project\windows-win32-vs\x64\D3D12-Win32-Debug\app\shaders-d3d12
@rem set OUTPUT_PATH=..\compiled\shaders-d3d12
@rem set SHADER_MODEL=5_1

mkdir %OUTPUT_PATH%
"%COMPILER_PATH%" /Fx "shaders_ps_assembly.txt" /enable_unbounded_descriptor_tables /T ps_5_1 /E PSMain /D COMPILING_PS=1 /D MAP_REFLECTION_ENABLED=1 /D MAP_DIFFUSE_ENABLED=1 /D MAP_BUMP_ENABLED=1 /D POINT_LIGHT_COUNT=1 /D SPOT_LIGHT_COUNT=1 /Fo "%OUTPUT_PATH%\shaders_ps.cso" "ForwardShaders.hlsl"
"%COMPILER_PATH%" /Fx "shaders_vs_assembly.txt" /enable_unbounded_descriptor_tables /T vs_5_1 /E VSMain /D COMPILING_VS=1 /D MAP_REFLECTION_ENABLED=1 /D MAP_DIFFUSE_ENABLED=1 /D MAP_BUMP_ENABLED=1 /D POINT_LIGHT_COUNT=1 /D SPOT_LIGHT_COUNT=1 /Fo "%OUTPUT_PATH%\shaders_vs.cso" "ForwardShaders.hlsl"
