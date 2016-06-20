'use strict'

const path = require('path')

module.exports = Module

const EXTENSIONS = {
  '.js' (module, filename) {
    const contents = _read(filename)
    const fn = cppBridge.run(
      MODULE_WRAPPER.join(contents),
      filename,
      1,
      1
    )
    fn(
      module.exports,
      module.require,
      module,
      module.filename,
      module.dirname
    )
  },
  '.json' (module, filename) {
    module.exports = JSON.parse(_read(filename))
  }
}

const PATH_TO_JSON_MAIN = new Map()
const MODULE_CACHE = new Map()
const STAT_CACHE = new Map()
const MODULE_WRAPPER = [
  '(function (exports, require, module, __filename, __dirname) { ',
  '\n});'
]

// TODO(chrisdickinson): eventually replace this with a more robust
// "read file" implementation.
const FILE_SCRATCH_BUF = new Uint8Array(1 << 20)
const FILE_SCRATCH_VIEW = cppBridge.createStringView(FILE_SCRATCH_BUF)

const NODE_BUILTINS = new Set([
  'assert', 'buffer', 'child_process', 'cluster',
  'crypto', 'dgram', 'dns', 'domain', 'events', 'fs', 'http', 'https', 'net',
  'os', 'path', 'punycode', 'querystring', 'readline', 'repl', 'stream',
  'string_decoder', 'tls', 'tty', 'url', 'util', 'v8', 'vm', 'zlib'
])

function stat (relpath) {
  if (STAT_CACHE.has(relpath)) {
    return STAT_CACHE.get(relpath)
  }
  try {
    const value = cppBridge.fsLStatSync(relpath)
    STAT_CACHE.set(relpath, value)
    return value
  } catch (err) {
    STAT_CACHE.set(relpath, null)
    return null
  }
}

function getJSONMain (relpath) {
  var value = null
  try {
    value = JSON.parse(_read(relpath)).main
  } finally {
    return value
  }
}

function _read (path) {
  const fd = cppBridge.fsOpenSync(path, cppBridge.constants.O_RDONLY, 0)
  const read = cppBridge.fsReadSync(
    fd,
    FILE_SCRATCH_BUF.buffer,
    FILE_SCRATCH_BUF.length,
    0
  )
  const subset = FILE_SCRATCH_VIEW.slice(0, read)
  cppBridge.fsCloseSync(fd)
  return subset
}

function resolve (module, relpath) {
  if (NODE_BUILTINS.has(relpath)) {
    relpath = `@no.js/node-${relpath}`
  }

  switch (relpath[0]) {
    case '.':
    case '/':
      return resolveLocal(module, relpath);
    default:
      return resolveNodeModule(module, relpath);
  }
}

function resolveLocal (module, relpath) {
  const fullPath = path.resolve(module.dirname, relpath)
  const fullPathResult = LOAD_AS_FILE(fullPath)
  if (fullPathResult !== null) {
    return fullPathResult
  }
  const fullPathStat = stat(fullPath)

  if (fullPathStat === null || !fullPathStat.isDirectory()) {
    throw new Error(`Could not resolve ${relpath}`)
  }

  const asDir = LOAD_AS_DIRECTORY(relpath)
  if (asDir !== null) {
    return asDir
  }

  throw new Error(`Could not resolve ${relpath}`)
}

function resolveNodeModule (module, relpath) {
  const dirs = NODE_MODULE_PATHS(module.dirname)
  for (var i = 0; i < dirs.length; ++i) {
    const testPath = path.join(dirs[i], relpath)
    const asFile = LOAD_AS_FILE(testPath)
    if (asFile !== null) {
      return asFile
    }
    const asDir = LOAD_AS_DIRECTORY(testPath)
    if (asDir !== null) {
      return asDir
    }
  }
  return null
}

function LOAD_AS_DIRECTORY (XS) {
  const packageJSONPath = path.join(XS, 'package.json')
  const packageJSONStat = stat(packageJSONPath)
  if (packageJSONStat !== null && packageJSONStat.isFile()) {
    const packageJSONMain = getJSONMain(packageJSONPath)
    if (packageJSONMain !== null) {
      const packageJSONMainPath = path.join(
        XS,
        packageJSONMain
      )
      const packageJSONMainResult = LOAD_AS_FILE(packageJSONMainPath)
      if (packageJSONMainResult !== null) {
        return packageJSONMainResult
      }
    }
  }

  return LOAD_AS_FILE(path.join(XS, 'index'))
}

function LOAD_AS_FILE (XS) {
  const XSStat = stat(XS)

  if (XSStat && XSStat.isFile()) {
    return XS
  }

  for (var dotEXT in EXTENSIONS) {
    const file = `${XS}${dotEXT}`
    const XSStat = stat(file)
    if (XSStat && XSStat.isFile()) {
      return file
    }
  }

  return null
}

function NODE_MODULE_PATHS (XS) {
  const bits = XS.split(/[\\/]/g)
  const out = []
  for (var i = bits.length; i > 0; --i) {
    if (bits[i - 1] === 'node_modules') {
      continue
    }
    const DIR = path.join(bits[i - 1], 'node_modules')
    const DIR_STAT = stat(DIR)
    if (DIR_STAT && DIR_STAT.isDirectory()) {
      out.push(DIR)
    }
  }
  return out
}

function load (module, filename) {
  const extension = path.extname(filename) || '.js'
  const loader = EXTENSIONS[extension] || EXTENSIONS['.js']
  loader(module, filename)
  module.loaded = true
}

function Module (filename, parent) {
  var self = this
  this.id = filename
  this.parent = parent
  if (parent) {
    if (parent.children) parent.children.push(this)
  }
  this.require = req => moduleRequire(this, req)
  this.require.resolve = path => resolve(this, path)
  this.filename = filename
  this.dirname = filename && path.dirname(filename)
  this.exports = {}
  this.loaded = false
  this.children = []
}

function moduleRequire (parent, modulePath) {
  const resolvedPath = resolve(parent, modulePath)

  if (resolvedPath === null) {
    throw new Error(`could not resolve ${modulePath}`)
  }

  if (MODULE_CACHE.has(resolvedPath)) {
    return MODULE_CACHE.get(resolvedPath).exports
  }

  const newModule = new Module(resolvedPath, module)
  MODULE_CACHE.set(resolvedPath, newModule)

  load(newModule, resolvedPath)

  return newModule.exports
}

Module.runMain = function (target) {
  target = (
    target[0] === '.'
    ? target[0]
    : path.resolve(cppBridge.cwd(), target)
  )

  var resolvedPath = resolve(new Module(
    path.join(cppBridge.cwd(), 'null'),
    null
  ), target)

  if (resolvedPath === null) {
    throw new Error(`Could not resolve ${target}`)
  }

  var module = new Module(resolvedPath, null)
  return load(module, resolvedPath)
}
