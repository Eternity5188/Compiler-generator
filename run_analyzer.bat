@echo off
if exist ".\target\analyzer\analyzer.exe" (
	.\target\analyzer\analyzer.exe
	goto :eof
)

if exist ".\target\analyzer\Debug\analyzer.exe" (
	.\target\analyzer\Debug\analyzer.exe
	goto :eof
)

echo analyzer executable not found.
echo Try building first: cmake -B build ^&^& cmake --build build -j
exit /b 1