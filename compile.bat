@echo off
"gcc.exe" -Iinclude -std=gnu11 -Wall -Wextra "src/DSConv.c" "src/lexer.c" "src/parser.c" "src/generator.c" -o "dsconv.exe"
pause