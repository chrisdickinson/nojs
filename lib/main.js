'use strict'

function main (cppBridge) {
  const xs = cppBridge.fsOpenSync(
    'test-file-sync.txt',
    cppBridge.constants.O_WRONLY |
    cppBridge.constants.O_CREAT |
    cppBridge.constants.O_TRUNC,
    cppBridge.constants.S_IRUSR |
    cppBridge.constants.S_IWUSR |
    cppBridge.constants.S_IRGRP
  )

  cppBridge.fsOpen(
    'test-file.txt',
    cppBridge.constants.O_WRONLY |
    cppBridge.constants.O_CREAT |
    cppBridge.constants.O_TRUNC,
    cppBridge.constants.S_IRUSR |
    cppBridge.constants.S_IWUSR |
    cppBridge.constants.S_IRGRP
  ).then(fd => {
    cppBridge.print(`hello: ${Number(fd)}`)
  }).catch(err => {
    cppBridge.print(`<UV error: ${Number(err)}>`)
  })
}

// NB(chrisdickinson): "main" must come last here - the last value in this
// script is used as the entry point.
main
