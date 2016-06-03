# nojs

[Nothing to see here.](PLAN.md)

Oh, there's [nothing to see here either.](https://github.com/chrisdickinson/nojs/issues/3)

## Hacking

:one: [Install Google's `depot_tools`][depot-tools].

:two: In a new directory, run `gclient config https://github.com/chrisdickinson/nojs.git`.

:three: Now run `gclient sync`. Go get some coffee! :coffee::runner:

:four: Generate the project!

> **On Linux**:
>
> `mkdir -p nojs/out/Default` and open `nojs/out/Default/args.gn`.
>
> Enter:
>
> ```
> is_clang = false
> use_sysroot = false
> target_sysroot = "/"
> ```
>
> Now, run `cd nojs; gn gen out/Default`.

> **On Mac**:
>
> `cd nojs; gn gen out/Default`.
>

:five: Now run `ninja -C out/Default ':nojs'`. Take a walk outside. :walking::deciduous_tree::cloud:

:six: Once your laptop cools down, run `out/Default/nojs`. Marvel at how little it does.

[depot-tools]: https://www.chromium.org/developers/how-tos/install-depot-tools
