## medium term plan

[isaac on "no"](https://vimeo.com/56402326)

1. Get a minimal project that includes v8, libuv, and the various uv bits
   **@indutny** has been putting together building everywhere.
2. At that point build in & expose fs, tcp, and tls bindings and a module
   system (via `require`) to js.
  * I might do this in a separate project using `gclient` & `gn` to pull in the
    minimal binding layer.
3. Whenever a node global (`process`) is accessed, or a node builtin module is
   required `require('fs')`, short circuit the lookup to
   `require('@nojs/node-<target>')`.
4. Long term goal is to get `npm install` working and bundle `npm` with the
   project.

## :cloud: foggy plans for the future :cloud:

My (handwave-y) plans are — and you'll probably find something you *like*
and something you *dislike* here:

* Steer closer to TC39:
  * The minimal API will use `Promise`. `async` is coming.
  * No streams at first. Possibly include streams from WHATWG's
    `ReadableStream` spec later.
  * No `EventEmitter` (except via backcompat.) Maybe someday `Observables`.
* Steer closer to (newer) Google tools:
  * Build with `gn` and `gclient`, keep deps up to date with `gclient sync`.
* Focus on FFI. (Insert **so much :wave: handwaving :wave: here**)
  * With an eye towards **@indutny**'s jit.js, heap.js, & mmap.js, explore exposing `mmap`
    in order to create callable executable code from JS (possibly only for core
    functionality, but maybe not.)
  * Binary compat with Node later.
    * **@dominictarr** had the excellent idea that the build tools should be dockerized.
* Stick with Node's decision on ES modules. If Node zigs, Nojs zigs. No zagging, never zagging.
  * Interoperability/backcompat is **key**.
