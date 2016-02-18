#!/bin/bash

VALGRIND_LOG=$1
VALGRIND_LOG_CLEAN="${VALGRIND_LOG}_clean.log"

sed -r -e 's|==[[:digit:]]+== ||g' \
       -e 's|--[[:digit:]]+-- ||g' \
       -e 's|blocks are definitely lost in loss record ([[:digit:]],?)+ of ([[:digit:]],?)+|blocks|g' \
       -e 's|blocks are indirectly lost in loss record ([[:digit:]],?)+ of ([[:digit:]],?)+|blocks|g' \
       -e 's|blocks are possibly lost in loss record ([[:digit:]],?)+ of ([[:digit:]],?)+|blocks|g' \
       -e 's|blocks are still reachable in loss record ([[:digit:]],?)+ of ([[:digit:]],?)+|blocks|g' \
       $VALGRIND_LOG > $VALGRIND_LOG_CLEAN

echo "Valgrind log cleaned in $VALGRIND_LOG_CLEAN"

head -n 5 $VALGRIND_LOG_CLEAN
