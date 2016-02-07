#!/bin/sh

valgrind -v --tool=memcheck \
    --leak-check=full \
    --track-origins=yes \
    --num-callers=20 \
    --error-limit=no \
    --log-file=run.log \
    --track-fds=yes \
    --leak-check=full \
    --track-origins=yes \
    --show-reachable=yes \
    --show-leak-kinds=definite,possible \
    --undef-value-errors=yes \
    ./nlb_test 32 0 3

