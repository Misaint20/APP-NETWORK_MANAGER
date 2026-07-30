@echo off
cd /d "%~dp0"
echo.
echo ========================================
echo  Building APP-NETWORK_MANAGER...
echo ========================================
echo.

del /Q *.o 2>nul
del /Q *.res 2>nul
del /Q APP-NETWORK_MANAGER.exe 2>nul
echo [1/5] Compiling resources...
windres resources.rc -O coff -o resources.res
if errorlevel 1 goto error

echo [2/5] Compiling LanguageManager.cpp...
g++ -std=c++20 -c LanguageManager.cpp -o LanguageManager.o -static
if errorlevel 1 goto error

echo [3/5] Compiling ApplicationManager.cpp...
g++ -std=c++20 -c ApplicationManager.cpp -o ApplicationManager.o -static
if errorlevel 1 goto error

echo [4/6] Compiling MainWindow.cpp (this may take a moment)...
g++ -std=c++20 -c MainWindow.cpp -o MainWindow.o -static
if errorlevel 1 goto error

echo [5/6] Compiling app_ui.cpp...
g++ -std=c++20 -c app_ui.cpp -o app_ui.o -static
if errorlevel 1 goto error

echo [6/6] Linking (static build - this can take 30-60 seconds)...
echo   Please wait, linking all libraries into a portable executable...
g++ LanguageManager.o ApplicationManager.o MainWindow.o app_ui.o resources.res -o APP-NETWORK_MANAGER.exe ^
    -mwindows ^
    -static-libgcc ^
    -static-libstdc++ ^
    -static ^
    -lstdc++fs ^
    -lpthread ^
    -lcomctl32 ^
    -lole32 ^
    -luuid ^
    -loleaut32

if errorlevel 1 goto error

del /Q *.o
del /Q *.res

echo.
echo ========================================
echo  Build completed successfully!         
echo  The executable is portable.           
echo ========================================
echo.
goto end

:error
echo.
echo ************************************************
echo  BUILD ERROR - check the messages above
echo ************************************************
echo.
pause
exit /b 1

:end
pause
