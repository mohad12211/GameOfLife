#!/bin/sh

emcc -o web/index.html src/main.c -Os -Wall -std=c99 -D_DEFAULT_SOURCE -Iweb/ web/libraylib.a  -s USE_GLFW=3 -s ASYNCIFY -s EXPORTED_RUNTIME_METHODS=ccall -DPLATFORM_WEB --shell-file web/minshell.html 
git checkout gh-pages
cp web/index* .
git commit -am "Update"
git push
