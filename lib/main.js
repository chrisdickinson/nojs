'use strict'

const nativeModuleCache = new Map()

function main (cppBridge) {
  const require = getNativeRequire(cppBridge)
  cppBridge.env = {}

  const fs = require('fs')

  //cppBridge.print(fs.readFileSync('BUILD.gn'))
  const path = require('path')

  cppBridge.print(path.join(cppBridge.cwd(), 'DEPS'))
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

function createFS (cppBridge) {
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

function createModuleSystem (cached, resolve, load) {
  var main = null

  function Module (filename, parent) {
    var self = this
    this.id = filename
    this.parent = parent
    if (parent) {
      if (parent.children) parent.children.push(this)
    }
    this.require = this.require.bind(this)
    this.require.resolve = function (path) {
      return resolve(self, path)
    }
    this.filename = filename
    this.exports = {}
    this.loaded = false
    this.children = []
  }
  Module.prototype.load = function () {
    assert(this.loaded === false)
    load(this, this.filename)
    return this.exports
  }
  Module.prototype.require = function (modulePath) {
    var resolvedPath = resolve(this, modulePath)
    var cachedModule = cached(this, resolvedPath)
    if (cachedModule) return cachedModule.exports
    var newModule = new Module(resolvedPath, module)
    return _load(newModule, main)
  }
  Module.runMain = function (path) {
    var resolvedPath = resolve(null, path)
    var module = new Module(resolvedPath, null)
    main = module
    return _load(module, module)
  }
  return Module
}

main.Stat = Stat

// NB(chrisdickinson): "main" must come last here - the last value in this
// script is used as the entry point.
main
