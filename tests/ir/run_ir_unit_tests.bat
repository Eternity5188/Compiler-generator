@echo off
setlocal

pushd "%~dp0\..\.."

if exist ".\target\analyzer\analyzer.exe" (
    .\target\analyzer\analyzer.exe --unit
    set "ret=%errorlevel%"
    popd
    exit /b %ret%
)

if exist ".\target\analyzer\Debug\analyzer.exe" (
    .\target\analyzer\Debug\analyzer.exe --unit
    set "ret=%errorlevel%"
    popd
    exit /b %ret%
)

echo analyzer executable not found.
echo Try building first: cmake -B build ^&^& cmake --build build -j
popd
exit /b 1
