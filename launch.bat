@echo off
cd project_animator
set /a port=%random% %%8000 +1000

pushd build\wasm_debug
start "" http://localhost:%port%
python -m http.server %port%
popd