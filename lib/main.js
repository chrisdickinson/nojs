'use strict'

function main (cppBridge) {

  const ab = new Uint8Array(32 << 10)

  const xs = cppBridge.fsOpenSync(
    'BUILD.gn',
    cppBridge.constants.O_RDONLY,
    cppBridge.constants.S_IRUSR |
    cppBridge.constants.S_IWUSR |
    cppBridge.constants.S_IRGRP
  )

  const bytes = cppBridge.fsReadSync(xs, ab.buffer, ab.length, 0)


  const str = cppBridge.createStringView(ab.slice(0, bytes))
  cppBridge.print(str + 'uhhhhh')

  const v = cppBridge.run(
    `(function (a, b) { return a + b })`,
    'cool-chuck.js',
    0,
    0
  )

  cppBridge.print(cppBridge.cwd())

  cppBridge.print(v('hello ', 'world\n'))

  const _start = Date.now()
  Promise.resolve(null).then(() => {
    cppBridge.print(Date.now() - _start + 'ms since first tick')
    const start = Date.now()
    cppBridge.fsRead(xs, ab.buffer, ab.length, 0).then(bytes => {
      const delta = Date.now() - start
      for (var i = 0; i < bytes; ++i) {
        // cppBridge.print(String.fromCharCode(ab[i]))
      }
      cppBridge.print(ab[0] + ' ' + bytes + ' ' + delta + 'ms\n')
      cppBridge.fsClose(xs).then(() => {
        cppBridge.print(typeof TextDecoder + 'whatever \n')
      })
    })
  })


  const ys = cppBridge.fsOpenSync(
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
    cppBridge.print(`hello: ${Number(fd)}\n`)
    return cppBridge.fsClose(fd)
  }).then(xs => {
    cppBridge.print("ok!\n")
  }).catch(err => {
    cppBridge.print(`<UV error: ${Number(err)}>\n`)
  })
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

main.Stat = Stat

// NB(chrisdickinson): "main" must come last here - the last value in this
// script is used as the entry point.
main
