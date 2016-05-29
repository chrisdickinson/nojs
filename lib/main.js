'use strict'

function main (cppBridge) {
  cppBridge.print(
    Object.keys(cppBridge.sources).map(xs => {
      return cppBridge.sources[xs]
    })
  )
}

// NB(chrisdickinson): "main" must come last here - the last value in this
// script is used as the entry point.
main
