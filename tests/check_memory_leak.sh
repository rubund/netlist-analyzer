#!/bin/sh

valgrind --leak-check=full   --show-leak-kinds=all ./test1
