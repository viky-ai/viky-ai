#!/bin/bash
set -x -e

sleep 1

# Run tests
./bin/rails test:all
