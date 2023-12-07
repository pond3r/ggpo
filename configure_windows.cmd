@echo off
setlocal

set GGPO_SHARED_LIB=off
set STEAMWORKS_PATH=
set NO_PROMPT=

:parse_args
IF "%~1"=="" GOTO :start
IF "%1"=="--no-prompt" SET NO_PROMPT=1 & SHIFT & GOTO :parse_args
SET STEAMWORKS_PATH=%1
SHIFT
GOTO :parse_args

:start
IF "%GGPO_SHARED_LIB%" == "" (
   echo GGPO_SHARED_LIB not set.  Defaulting to off
)

echo Generating GGPO Visual Studio solution files.
echo    GGPO_SHARED_LIB=%GGPO_SHARED_LIB%
echo    Steamworks Path: %STEAMWORKS_PATH%

cmake -G "Visual Studio 16 2019" -A x64 -B build -DBUILD_SHARED_LIBS=%GGPO_SHARED_LIB% -DSTEAMWORKS_PATH="%STEAMWORKS_PATH%"

echo Finished!  Open build/GGPO.sln in Visual Studio to build.

IF NOT DEFINED NO_PROMPT pause

:done
