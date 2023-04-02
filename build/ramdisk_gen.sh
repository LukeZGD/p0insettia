#!/bin/bash

cd "$(dirname "$0")"

hfsplus="bin/macos/hfsplus"
xpwntool="bin/macos/xpwntool"
if [[ $(uname) == "Linux" ]]; then
    dir="bin/linux/"
    if [[ $(uname -m) == "a"* && $(getconf LONG_BIT) == 64 ]]; then
        dir+="arm64"
    elif [[ $(uname -m) == "a"* ]]; then
        dir+="armhf"
    else
        dir+="x86_64"
    fi
    hfsplus="$dir/hfsplus"
    xpwntool="$dir/xpwntool"
fi

rm -rf image3/rdsk

$xpwntool image3/rrdsk image3/rrdsk.dmg

$hfsplus image3/rrdsk.dmg grow 50000000

$hfsplus image3/rrdsk.dmg mv usr/local/bin/restored_external usr/local/bin/restored_external_
$hfsplus image3/rrdsk.dmg add ../iphoneos-arm/rdsk/restored_external usr/local/bin/restored_external
$hfsplus image3/rrdsk.dmg chmod 755 usr/local/bin/restored_external
$hfsplus image3/rrdsk.dmg chown 0:0 usr/local/bin/restored_external

$hfsplus image3/rrdsk.dmg mkdir haxx/
$hfsplus image3/rrdsk.dmg chmod 755 haxx/
$hfsplus image3/rrdsk.dmg chown 0:0 haxx/

$hfsplus image3/rrdsk.dmg mkdir haxx/LaunchDaemons
$hfsplus image3/rrdsk.dmg chmod 755 haxx/LaunchDaemons
$hfsplus image3/rrdsk.dmg chown 0:0 haxx/LaunchDaemons

$hfsplus image3/rrdsk.dmg add ../iphoneos-arm/launchd/launchd haxx/launchd
$hfsplus image3/rrdsk.dmg chmod 755 haxx/launchd
$hfsplus image3/rrdsk.dmg chown 0:0 haxx/launchd

$hfsplus image3/rrdsk.dmg add ../iphoneos-arm/rdsk/src/loader.dmg haxx/loader.dmg
$hfsplus image3/rrdsk.dmg chmod 644 haxx/loader.dmg
$hfsplus image3/rrdsk.dmg chown 0:0 haxx/loader.dmg

$hfsplus image3/rrdsk.dmg add ../iphoneos-arm/rdsk/src/package/haxx/xpcd.dmg haxx/xpcd.dmg
$hfsplus image3/rrdsk.dmg chmod 644 haxx/xpcd.dmg
$hfsplus image3/rrdsk.dmg chown 0:0 haxx/xpcd.dmg

$hfsplus image3/rrdsk.dmg add ../iphoneos-arm/rdsk/src/launchctl bin/launchctl
$hfsplus image3/rrdsk.dmg chmod 755 bin/launchctl
$hfsplus image3/rrdsk.dmg chown 0:0 bin/launchctl

$hfsplus image3/rrdsk.dmg add ../iphoneos-arm/rdsk/src/package/usr/libexec/dirhelper dirhelper
$hfsplus image3/rrdsk.dmg chmod 755 dirhelper
$hfsplus image3/rrdsk.dmg chown 0:0 dirhelper

$xpwntool image3/rrdsk.dmg image3/rdsk -t image3/rrdsk
rm -rf image3/rrdsk.dmg
