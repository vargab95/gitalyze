#!/bin/bash

valgrind --leak-check=full \
         --show-leak-kinds=all \
         --track-origins=yes \
         --num-callers=100 \
         --verbose \
         ./gitalyze "$@"
