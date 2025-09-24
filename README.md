# LiquidSSH

LiquidSSH is a modern, cross-platform SSH client built with **C++** and **Qt 6**, designed with a sleek glass-like UI inspired by macOS Finder.  
It aims to provide a lightweight, performant, and elegant alternative to existing SSH clients.

## Features

-  Secure SSH connections using OpenSSH libraries
-  Tabbed interface for managing multiple sessions
-  Glass-like UI with transparency and dark/light mode support
-  Persistent connection profiles stored in SQLite
-  Sidebar for quick access to servers and favorites
-  Built with C++23 and Qt 6 for performance and modern standards

##  Getting Started

### Prerequisites
- **CMake 3.20+**
- **Qt 6.9+** (Widgets, Sql modules)
- A C++23 compatible compiler (Clang / GCC / MSVC)

### Build Instructions

```bash
# Clone the repo
git clone https://github.com/yourusername/liquidssh.git
cd liquidssh

# Configure with CMake
cmake -B build -S . -DCMAKE_PREFIX_PATH=/path/to/Qt/6.9.2/macos

# Build
cmake --build build

