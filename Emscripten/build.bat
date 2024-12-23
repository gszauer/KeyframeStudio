copy manifest.json Build\\manifest.json
copy sw.js Build\\sw.js
copy icon.png Build\\icon.png

emcc build.cpp -g -o Build/index.html -fno-rtti -fno-exceptions -sUSE_WEBGL2 --shell-file=shell.html -s ALLOW_MEMORY_GROWTH=1 -s INITIAL_MEMORY=134217728 -Wno-nontrivial-memaccess -Wno-format-security -Wno-dynamic-class-memaccess --js-library PlatformWasm.js -sEXPORTED_RUNTIME_METHODS=setValue -s EXPORTED_FUNCTIONS="['_main', '_malloc', '_free', '_WASM_InvokePlatformSelectCallback']"