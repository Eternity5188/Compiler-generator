@echo off

if exist ".\target\generator\lexical_generator.exe" (
	.\target\generator\lexical_generator.exe
) else if exist ".\target\generator\Debug\lexical_generator.exe" (
	.\target\generator\Debug\lexical_generator.exe
) else (
	echo lexical_generator not found, skip.
)

if exist ".\target\generator\syntax_generator.exe" (
	.\target\generator\syntax_generator.exe
	goto :eof
)

if exist ".\target\generator\Debug\syntax_generator.exe" (
	.\target\generator\Debug\syntax_generator.exe
	goto :eof
)

echo syntax_generator executable not found.
echo Try building first: cmake -B build ^&^& cmake --build build -j
exit /b 1