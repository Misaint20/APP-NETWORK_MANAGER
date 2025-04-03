@echo off
echo Building APP-NETWORK_MANAGER...

del /Q *.o 2>nul
del /Q *.res 2>nul
del /Q APP-NETWORK_MANAGER.exe 2>nul

windres resources.rc -O coff -o resources.res
if errorlevel 1 goto error

g++ -std=c++20 -c ApplicationManager.cpp -o ApplicationManager.o -static
if errorlevel 1 goto error
g++ -std=c++20 -c MainWindow.cpp -o MainWindow.o -static
if errorlevel 1 goto error
g++ -std=c++20 -c app_ui.cpp -o app_ui.o -static
if errorlevel 1 goto error

g++ ApplicationManager.o MainWindow.o app_ui.o resources.res -o APP-NETWORK_MANAGER.exe ^
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

echo Build completed successfully.
echo The generated executable is portable and contains all resources.
goto end

:error
echo Build error
pause
exit /b 1

:end
pause
