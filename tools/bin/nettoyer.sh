#!/usr/bin/env bash
#
# Clean Script
#
# Some details:
# - We don't look in ".svn" directories, it's useless and dangerous
# - We don't touch what's under private (esp. there are "debug" and
#   "release" directories we don't want to remove)
#

if [ -z $OG_REPO_PATH ]; then
  echo "OG_REPO_PATH not defined. Aborting."
  exit 1
fi

FIND=/usr/bin/find
SOURCES=$OG_REPO_PATH/sources

echo "Cleaning up in $SOURCES"

$FIND $SOURCES \
  -name "vc90.pdb" -print0 \
  -or -name "*.suo" -print0 \
  -or -name "*.ncb" -print0 \
  -or -name "*.RES" -print0 \
  -or -name "Debug" -print0 \
  -or -name "Release" -print0 \
  -or -name "obj" -print0 \
  -or -name "COPY" -print0 \
  -or -name ".svn" -prune \
  -or -path "sources/private" -prune \
| xargs -0 rm -rf

echo "Clean done"