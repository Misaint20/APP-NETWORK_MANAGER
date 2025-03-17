@echo off
echo Compilando APP-NETWORK_MANAGER...

REM Limpiar archivos anteriores
del /Q *.o 2>nul
del /Q APP-NETWORK_MANAGER.exe 2>nul

REM Compilar recursos
windres resources.rc -O coff -o resources.o
if errorlevel 1 goto error

REM Compilar versión UI
g++ -std=c++20 -c ApplicationManager.cpp -o ApplicationManager.o
if errorlevel 1 goto error
g++ -std=c++20 -c MainWindow.cpp -o MainWindow.o
if errorlevel 1 goto error
g++ -std=c++20 -c app_ui.cpp -o app_ui.o
if errorlevel 1 goto error

REM Linkear
g++ ApplicationManager.o MainWindow.o app_ui.o resources.o -o APP-NETWORK_MANAGER.exe -mwindows -lstdc++fs -lpthread -lcomctl32 -static-libgcc -static-libstdc++
if errorlevel 1 goto error

REM Limpiar archivos objeto
del /Q *.o

echo Compilación completada exitosamente.
goto end

:error
echo Error durante la compilación
pause
exit /b 1

:end
pause
