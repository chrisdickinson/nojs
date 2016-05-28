#!/bin/bash

if [ ! -e depot_tools ]; then
  echo 'cloning depot_tools...'
  git clone https://chromium.googlesource.com/chromium/tools/depot_tools.git
else
  echo 'updating depot_tools...'
  cd depot_tools
  git pull origin master
  cd ..
fi
export PATH=./depot_tools:$PATH
DEPOT_TOOLS_UPDATE=0 gclient config https://chromium.googlesource.com/chromium/src.git --unmanaged
DEPOT_TOOLS_UPDATE=0 gclient sync
