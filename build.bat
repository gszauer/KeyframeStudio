@echo off

cd project_animator

REM Build wasm debug

mkdir build
mkdir build\\wasm_debug
mkdir build\\wasm_debug\\assets

del build\\wasm_debug\\assets\\*.* /F /Q
del build\\wasm_debug\\*.* /F /Q
xcopy "assets" "build/wasm_debug/assets" /E /Y
xcopy "../platform/wasm" "build/wasm_debug" /E /V /Y
del build\\wasm_debug\\assets\\*.bak /F /Q
del build\\wasm_debug\\assets\\*.vfb /F /Q
del build\\wasm_debug\\assets\\*.exe /F /Q

C:/WASM/clang.exe -x c++ ^
    --target=wasm32 ^
    -nostdinc ^
    -nostdlib ^
    -O0 ^
    -gfull ^
    -fno-threadsafe-statics ^
    -Wl,--allow-undefined ^
    -Wl,--import-memory ^
    -Wl,--no-entry ^
    -Wl,--export-dynamic ^
    -Wl,-z,stack-size=4194304 ^
    -D WASM32=1 ^
    -D _WASM32=1 ^
    -D DEBUG=1 ^
    -D _DEBUG=1 ^
    -o "build/wasm_debug/application.wasm" ^
    animator.cpp

python C:/WASM/wasm-sourcemap.py ^
    "build/wasm_debug/application.wasm" -s ^
    -o "build/wasm_debug/application.wasm.map" ^
    -u "./application.wasm.map" ^
    -w "build/wasm_debug/application.debug.wasm" ^
    --dwarfdump="C:/WASM/llvm-dwarfdump.exe"



REM Build wasm release

mkdir build
mkdir build\\wasm_release
mkdir build\\wasm_release\\assets

del build\\wasm_release\\assets\\*.* /F /Q
del build\\wasm_release\\*.* /F /Q
xcopy "assets" "build/wasm_release/assets" /E /Y
xcopy "../platform/wasm" "build/wasm_release" /E /V /Y
del build\\wasm_release\\assets\\*.bak /F /Q
del build\\wasm_release\\assets\\*.vfb /F /Q
del build\\wasm_release\\assets\\*.exe /F /Q

C:/WASM/clang.exe -x c++ ^
    --target=wasm32 ^
    -nostdinc ^
    -nostdlib ^
    -O3 ^
    -flto ^
    -Wl,--allow-undefined ^
    -Wl,--import-memory ^
    -Wl,--no-entry ^
    -Wl,--export-dynamic ^
    -Wl,--lto-O3 ^
    -Wl,-z,stack-size=4194304 ^
    -D WASM32=1 ^
    -D _WASM32=1 ^
    -o "build/wasm_release/application.wasm" ^
    animator.cpp