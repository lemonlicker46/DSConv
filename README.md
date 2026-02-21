# DSConv

DSConv is a work-in-progress C tool to convert data-structure definitions between C, FreeBasic, and FreePascal.

Currently this repository contains a CLI scaffold and interfaces for a parser/AST and generators.

Build (simple):

```sh
gcc -Iinclude DSConv.c -o dsconv
```

Example run (stub):

```sh
./dsconv -i examples/sample.c -s c -t all
```

Next steps: implement parsers for each language and code-generators that honor CLI switches.
