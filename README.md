# NETCONTROL-APP

A Windows application to manage Internet access for installed applications through Windows Firewall rules.

## Features

- List all installed applications
- Block/Allow Internet access for specific applications
- Available in both Console and GUI versions
- Automatically finds application executables
- Administrative privileges handling
- Clean and simple interface

## Versions

- **Console Version**: Traditional command-line interface available in English and Spanish
- **GUI Version**: Modern graphical interface with responsive design

## Requirements

- Windows Operating System
- Administrative privileges
- MinGW-w64 (for compilation)
- C++20 compatible compiler

## Building

### Console Version
```bash
g++ -std=c++20 app.cpp -o NETCONTROL.exe -lstdc++fs -lpthread
```

### GUI Version
```bash
cd UI
windres resources.rc -O coff -o resources.o
g++ -std=c++20 ApplicationManager.cpp MainWindow.cpp app_ui.cpp resources.o -o APP-NETWORK_MANAGER.exe -mwindows -lstdc++fs -lpthread -lcomctl32 -static-libgcc -static-libstdc++
```

Or simply run the provided `build.bat` script.

## Usage

1. Run the application with administrative privileges
2. Select an application from the list
3. Choose to block or allow Internet access
4. The application will create/remove the corresponding firewall rules

## Contributing

Feel free to submit issues and pull requests.

## License

This project is licensed under the MIT License - see the LICENSE file for details.
