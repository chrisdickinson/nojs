# nojs

Nothing to see here.

## Hacking

:one: [Install Google's `depot_tools`][depot-tools].
:two: In a new directory, run `gclient config https://github.com/chrisdickinson/nojs.git`.
:three: Now run `gclient sync`. Go get some coffee! :coffee::runner:
:four: In `./nojs/`, run `gn gen out/Default`.
:five: Now run `ninja -C out/Default ':nojs'`. Take a walk outside. :walking::deciduous_tree::cloud:
:six: Once your laptop cools down, run `out/Default/nojs`. Marvel at how little it does.

[depot-tools]: https://www.chromium.org/developers/how-tos/install-depot-tools
