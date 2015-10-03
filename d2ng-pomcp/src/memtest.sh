#!/bin/bash

SIZE=2
NUM=2
LOG="log-$$.txt"

if [ ! -z $1 ]; then
    SIZE=$1
fi

if [ ! -z $2 ]; then
    NUM=$2
fi

valgrind -v --leak-check=full ./pomcp --size $SIZE --number $NUM --problem fieldvisionrocksample \
                                            --verbose 1 \
                                            --reusedepth 2 \
                                            --reusetree 1 \
                                            --useparticlefilter 1 \
                                            --treeknowledge 2 \
                                            --rolloutknowledge 2 \
                                            --mindoubles 10 \
                                            --maxdoubles 10 \
                                            --runs 1 \
                                            --seeding 1 \
                                            2>&1 | tee $LOG

