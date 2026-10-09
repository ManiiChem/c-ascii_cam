@echo off
where cl >nul 2> nul || call "C:\Program Files\Microsoft Visual Studio\18\Community\VC\Auxiliary\Build\vcvars64.bat" >nul
if not exist build mkdir build
cl /nologo /W4 /std:c17 /Zi src\*.c /Fe:build\ascii_cam.exe /Fo:build\ /Fd:build\ /link mfplat.lib mf.lib mfreadwrite.lib mfuuid.lib ole32.lib