cmake -G "Visual Studio 16 2019" -A x64 -B build -DBUILD_SHARED_LIBS=off

call "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\Common7\Tools\VsDevCmd.bat"
echo on

devenv.exe "build\GGPO.sln" /build Release /out release.log
type release.log

devenv.exe "build\GGPO.sln" /build Debug /out debug.log
type debug.log

