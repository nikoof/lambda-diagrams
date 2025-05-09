# Tromp Lambda Diagrams

An attempt at generating [Tromp diagrams](https://tromp.github.io/cl/diagrams.html) for closed lambda terms. I learned about these from [What is PLUS times PLUS?](https://youtu.be/RcVA8Nj6HEo) by 2swap (great content, highly recommend).

# Building

Using [nob.h](https://github.com/tsoding/nob.h) (requires clang, but feel free to change `nob.c` to use gcc or something else).
```
cc -I./include -o nob nob.c
./nob && ./build/tromp
```

Or with nix (~~I am aware this kind of defeats the point of nob but oh well~~)
```
nix build .#
nix run .#
```

> ![note]
> Nix downloads its own `nob.h`.
