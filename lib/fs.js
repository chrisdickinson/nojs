'use strict'

// reserve a megabyte.
const buf = new Uint8Array(1 << 20)
const str = cppBridge.createStringView(buf)

module.exports = {
  statSync (path) {
    return fs.lstatSync(path)
  },
  readFileSync (path) {
    const fd = cppBridge.fsOpenSync(path, cppBridge.constants.O_RDONLY, 0)
    const read = cppBridge.fsReadSync(fd, buf.buffer, buf.length, 0)
    const subset = str.slice(0, read)
    cppBridge.fsCloseSync(fd)
    return subset
  }
}
