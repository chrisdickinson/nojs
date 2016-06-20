'use strict'

const nativeModuleCache = new Map()

const console = {}

function main (cppBridge) {
  const require = getNativeRequire(cppBridge)
  cppBridge.env = {}
  Stat.prototype.constants = cppBridge.constants

  console.log = function (...args) {
    for (var i = 0; i < args.length; ++i) {
      const value = (
        !args[i] ? args[i] :
        typeof args[i] === 'object' ? JSON.stringify(args[i], null, 2) :
        args[i]
      )
      cppBridge.print(value)
    }
    cppBridge.print('\n')
  }

  const module = require('module')
  const path = require('path')

  try {
    module.runMain('gloo.js')
  } catch (err) {
    console.log(err.stack)
  }
}

function getNativeRequire (bridge) {
  return require

  function require (name) {
    if (nativeModuleCache.has(name)) {
      return nativeModuleCache.get(name).exports
    }
    const module = {exports: {}}
    nativeModuleCache.set(name, module)
    bridge.run(
      '(function (module, require, cppBridge) {' +
      bridge.sources[`native ${name}.js`] +
      '})',
      `native ${name}.js`,
      0,
      0
    )(module, require, bridge)
    return module.exports
  }
}

function Stat(dev, mode, nlink, uid, gid, rdev, ino, size, blksize, blocks, flags, gen, atime, mtime, ctime, btime) {
  this.dev = dev
  this.mode = mode
  this.nlink = nlink
  this.uid = uid
  this.gid = gid
  this.rdev = rdev
  this.ino = ino
  this.size = size
  this.blksize = blksize
  this.blocks = blocks
  this.atime = atime
  this.mtime = mtime
  this.ctime = ctime
  this.btime = btime
}

Stat.prototype._checkModeProperty = function (property) {
  return ((this.mode & this.constants.S_IFMT)) === property
}

Stat.prototype.isDirectory = function() {
  return this._checkModeProperty(this.constants.S_IFDIR)
}

Stat.prototype.isFile = function() {
  return this._checkModeProperty(this.constants.S_IFREG)
}

Stat.prototype.isBlockDevice = function() {
  return this._checkModeProperty(this.constants.S_IFBLK)
}

Stat.prototype.isCharacterDevice = function() {
  return this._checkModeProperty(this.constants.S_IFCHR)
}

Stat.prototype.isSymbolicLink = function() {
  return this._checkModeProperty(this.constants.S_IFLNK)
}

Stat.prototype.isFIFO = function() {
  return this._checkModeProperty(this.constants.S_IFIFO)
}

Stat.prototype.isSocket = function() {
  return this._checkModeProperty(this.constants.S_IFSOCK)
}

main.Stat = Stat

// NB(chrisdickinson): "main" must come last here - the last value in this
// script is used as the entry point.
main
