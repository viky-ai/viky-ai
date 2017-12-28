#!/bin/bash
set -x -e

sleep 1

# Run tests
./bin/rails test

# Run system tests
./bin/rails test:system
