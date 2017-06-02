del project\windows-win32-vs\*.user
del project\windows-win32-vs\*.suo
del project\windows-win32-vs\*.sdf
del project\windows-win32-vs\*.db
rmdir /S /Q project\windows-win32-vs\bin
rmdir /S /Q project\windows-win32-vs\ipch
rmdir /S /Q project\windows-win32-vs\x64
rmdir /S /Q project\windows-win32-vs\Debug
rmdir /S /Q project\windows-win32-vs\Release
rmdir /S /Q project\windows-win32-vs\obj
rmdir /S /Q "project\windows-uwp-vs\Generated Files"
rmdir /S /Q project\windows-uwp-vs\x64
rmdir /S /Q project\windows-uwp-vs\x86
rmdir /S /Q project\windows-uwp-vs\arm

rm -r assets\shaders\d3d12\output
