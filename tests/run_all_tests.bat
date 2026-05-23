@echo off
setlocal

pushd "%~dp0"

set /a total=0
set /a pass=0
set /a fail=0

echo ========================================
echo Runner: one-click test execution
echo ========================================
echo.

set /a total+=1
echo [1/3] Running lex_yacc tests...
call "lex_yacc\run_lex_yacc_tests.bat"
if errorlevel 1 (
    set /a fail+=1
    echo [FAIL] lex_yacc
) else (
    set /a pass+=1
    echo [PASS] lex_yacc
)
echo.

set /a total+=1
echo [2/3] Running ir unit tests...
call "ir\run_ir_unit_tests.bat"
if errorlevel 1 (
    set /a fail+=1
    echo [FAIL] ir
) else (
    set /a pass+=1
    echo [PASS] ir
)
echo.

set /a total+=1
echo [3/3] Running system tests...
call "system\run_system_tests.bat"
if errorlevel 1 (
    set /a fail+=1
    echo [FAIL] system
) else (
    set /a pass+=1
    echo [PASS] system
)
echo.

echo ========================================
echo RUNNER_SUMMARY: PASS=%pass% FAIL=%fail% TOTAL=%total%
echo ========================================

popd
if %fail% gtr 0 exit /b 1
exit /b 0
