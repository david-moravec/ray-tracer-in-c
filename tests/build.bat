@echo off
setlocal

cd /d "%~dp0.."

if "%CC%"=="" set CC=clang

%CC% -std=gnu11 -Wall -Wextra -I. tests\test_obj_parser.c base\arena\arena.c base\collections\list.c -o tests\test_obj_parser.exe
if errorlevel 1 exit /b 1

tests\test_obj_parser.exe
