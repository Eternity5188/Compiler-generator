@echo off
setlocal EnableDelayedExpansion

pushd "%~dp0\..\.."

set "ANALYZER=.\target\analyzer\analyzer.exe"
if not exist "%ANALYZER%" set "ANALYZER=.\target\analyzer\Debug\analyzer.exe"
if not exist "%ANALYZER%" (
    echo analyzer executable not found.
    echo Try building first: cmake -B build ^&^& cmake --build build -j
    popd
    exit /b 1
)

set /a total=0
set /a pass=0
set /a fail=0

set "OUT_DIR=tests\output\lex_yacc"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

echo Running lex_yacc test corpus...
for %%F in ("tests\lex_yacc\lex_yacc_*.c") do (
    set /a total+=1
    set "CASE_OUT=%OUT_DIR%\%%~nF.txt"
    > "!CASE_OUT!" echo CASE_NAME:
    >> "!CASE_OUT!" echo %%~nxF
    >> "!CASE_OUT!" echo.
    >> "!CASE_OUT!" echo SOURCE_CODE:
    >> "!CASE_OUT!" echo ----------------------------------------
    type "%%~fF" >> "!CASE_OUT!"
    >> "!CASE_OUT!" echo.
    >> "!CASE_OUT!" echo ----------------------------------------
    >> "!CASE_OUT!" echo.
    >> "!CASE_OUT!" echo ANALYZER_OUTPUT:
    "%ANALYZER%" --trace "%%~fF" >> "!CASE_OUT!" 2>&1
    if errorlevel 1 (
        set /a fail+=1
        echo [FAIL] %%~nxF
    ) else (
        set /a pass+=1
        echo [PASS] %%~nxF
    )
)

echo.
echo LEX_YACC_SUMMARY: PASS=!pass! FAIL=!fail! TOTAL=!total!

popd
if !fail! gtr 0 exit /b 1
exit /b 0
