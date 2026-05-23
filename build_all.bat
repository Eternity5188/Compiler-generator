@echo off
setlocal

echo ========================================
echo Build and Run All Tests
echo ========================================

cmake -B build
if errorlevel 1 (
	echo [FAIL] CMake configure failed.
	exit /b 1
)

cmake --build build -j
if errorlevel 1 (
	echo [FAIL] Build failed.
	exit /b 1
)

call .\tests\run_all_tests.bat
if errorlevel 1 (
	echo [FAIL] Test suite failed.
	exit /b 1
)

echo [PASS] Build and tests completed.
exit /b 0