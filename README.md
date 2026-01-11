# 🛠️ Build-Anleitung

## Voraussetzungen

- C++17 kompatibler Compiler
  - Windows: Visual Studio 2019 oder höher
  - Linux: GCC, GNU make
  - macOS: Clang, GNU make
- CMake (Version 3.15 oder höher)

## Projekt klonen

```bash
git clone https://github.com/TNTCoder44/CaptureTheFlag.git
cd CaptureTheFlag
```

## Projekt konfigurieren

### Windows:
	Direkt mit Visual Studio öffnen und CMake-Projekt generieren.
	Visual Studio unterstützt CMake nativ.

### Linux/macOS:

Für Linux und MacOS wird vorzugsweise "Unix Makefiles" als Generator verwendet, da XCode nicht sehr zuverlässig mit CMake funktioniert.

```bash
mkdir build
cd build
cmake .. -G "Unix Makefiles"
make
```
