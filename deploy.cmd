@echo off

REM get python version as number, e.g. "312" for Python 3.12.x
for /f "tokens=2" %%i in ('python -V') do for /f "tokens=1,2 delims=." %%A in ("%%i") do set PYVER=%%A%%B

if "%1" == "x64" (
	move /y %3 %1\%2\webview2.cp%PYVER%-win_amd64.pyd
	if "%2" == "Release" (
		copy /y %1\%2\webview2.cp%PYVER%-win_amd64.pyd "%PYTHONHOME%\DLLs\webview2.cp%PYVER%-win_amd64.pyd"
	)
)

if "%1" == "Win32" (
	move /y %3 %1\%2\webview2.cp%PYVER%-win32.pyd
	if "%2" == "Release" (
		copy /y %1\%2\webview2.cp%PYVER%-win32.pyd "%PYTHONHOME%\DLLs\webview2.cp%PYVER%-win32.pyd"
	)
)
