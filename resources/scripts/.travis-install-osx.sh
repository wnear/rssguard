#!/bin/sh

# Build application.
ls
mkdir rssguard-build && cd rssguard-build
lrelease -compress ../rssguard.pro
qmake .. "USE_WEBENGINE=$USE_WEBENGINE"
make
make install

# Make DMG image.
cd "src/rssguard"

# Fix .dylib linking.
install_name_tool -change "librssguard.dylib" "@executable_path/librssguard.dylib" "RSS Guard.app/Contents/MacOS/rssguard"
install_name_tool -change "librssguard.dylib" "@executable_path/librssguard.dylib" "rssguard"

otool -L "RSS Guard.app/Contents/MacOS/rssguard"
otool -L "rssguard"

make dmg

# Rename DMG.
set -- *.dmg

rc=$?; if [[ $rc != 0 ]]; then exit $rc; fi

dmgname="$1"
git_tag=$(git describe --tags `git rev-list --tags --max-count=1`)
git_revision=$(git rev-parse --short HEAD)

if [ "$USE_WEBENGINE" = true ]; then
  dmgnewname="rssguard-${git_tag}-${git_revision}-mac64.dmg"
else
  dmgnewname="rssguard-${git_tag}-${git_revision}-nowebengine-mac64.dmg"
fi

mv "$dmgname" "$dmgnewname"

ls