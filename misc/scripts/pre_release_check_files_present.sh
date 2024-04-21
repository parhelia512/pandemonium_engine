cd ../../bin/

files=(
  # Windows
  "pandemonium.windows.opt.64.exe"
  "pandemonium.windows.opt.debug.64.exe"
  
  "pandemonium.windows.opt.32.exe"
  "pandemonium.windows.opt.debug.32.exe"

  "pandemonium.windows.opt.tools.64.exe"
  "pandemonium.windows.opt.tools.32.exe"

  # Linux
  "pandemonium.x11.opt.32"
  "pandemonium.x11.opt.64"
  "pandemonium.x11.opt.arm"
  "pandemonium.x11.opt.arm64"

  "pandemonium.x11.opt.debug.32"
  "pandemonium.x11.opt.debug.64"
  "pandemonium.x11.opt.debug.arm"
  "pandemonium.x11.opt.debug.arm64"

  "pandemonium.x11.opt.tools.32"
  "pandemonium.x11.opt.tools.64"
  "pandemonium.x11.opt.tools.arm"
  "pandemonium.x11.opt.tools.arm64"

  # Server (Linux)
  "pandemonium_server.x11.opt.32"
  "pandemonium_server.x11.opt.64"
  "pandemonium_server.x11.opt.arm"
  "pandemonium_server.x11.opt.arm64"

  "pandemonium_server.x11.opt.debug.32"
  "pandemonium_server.x11.opt.debug.64"
  "pandemonium_server.x11.opt.debug.arm"
  "pandemonium_server.x11.opt.debug.arm64"

  "pandemonium_server.x11.opt.tools.32"
  "pandemonium_server.x11.opt.tools.64"
  "pandemonium_server.x11.opt.tools.arm"
  "pandemonium_server.x11.opt.tools.arm64"

  # HTTP Server (Linux)
  "pandemonium_http_server.x11.opt.32"
  "pandemonium_http_server.x11.opt.64"
  "pandemonium_http_server.x11.opt.arm"
  "pandemonium_http_server.x11.opt.arm64"

  "pandemonium_http_server.x11.opt.debug.32"
  "pandemonium_http_server.x11.opt.debug.64"
  "pandemonium_http_server.x11.opt.debug.arm"
  "pandemonium_http_server.x11.opt.debug.arm64"

  # JS
  "pandemonium.javascript.opt.zip"
  "pandemonium.javascript.opt.debug.zip"

  "pandemonium.javascript.opt.threads.zip"
  "pandemonium.javascript.opt.debug.threads.zip"

  "pandemonium.javascript.opt.gdnative.zip"
  "pandemonium.javascript.opt.debug.gdnative.zip"

  "pandemonium.javascript.opt.tools.threads.zip"

  # Android
  "android_source.zip"
  "android_debug.apk"
  "android_release.apk"
  "android_editor.apk"
  "pandemonium-lib.release.aar"
  "pandemonium-lib.debug.aar"

  # OSX
  "pandemonium.osx.opt.universal"
  "pandemonium.osx.opt.debug.universal"
  "pandemonium.osx.opt.tools.universal"

  # iOS
  "libpandemonium.iphone.opt.arm64.a"
  "libpandemonium.iphone.opt.debug.arm64.a"

  "libpandemonium.iphone.opt.debug.x86_64.simulator.a"
  "libpandemonium.iphone.opt.x86_64.simulator.a"

  # FRT
  "pandemonium.frt.opt.arm32v6"
  "pandemonium.frt.opt.arm32v7"
  "pandemonium.frt.opt.arm64v8"

  "pandemonium.frt.opt.debug.arm32v6"
  "pandemonium.frt.opt.debug.arm32v7"
  "pandemonium.frt.opt.debug.arm64v8"

  # FRT SDL
  "pandemonium.frt_sdl.opt.arm32v6"
  "pandemonium.frt_sdl.opt.arm32v7"
  "pandemonium.frt_sdl.opt.arm64v8"

  "pandemonium.frt_sdl.opt.debug.arm32v6"
  "pandemonium.frt_sdl.opt.debug.arm32v7"
  "pandemonium.frt_sdl.opt.debug.arm64v8"

  # OSX - final editor
  "Pandemonium.app.zip"

  # OSX - final export templates
  "osx.zip"

  # IOS - final template
  "iphone.zip"
)

error=0

for f in ${files[*]} 
do
if [ ! -e $f ]; then
  error=1
  echo "$f is not present!"
fi
done

if [ $error -eq 0 ]; then
  echo "All files are present!"
fi

cd ../..
