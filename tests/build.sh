#!/usr/bin/env sh
set -eu

cd "$(dirname "$0")/.."

CC="${CC:-cc}"

$CC -std=gnu11 -Wall -Wextra -I. \
  tests/test_obj_parser.c \
  base/arena/arena.c \
  base/collections/list.c \
  -o tests/test_obj_parser

./tests/test_obj_parser
