#!/bin/bash
set -x -e

sleep 1

echo "Am Starting"

# Setup DB
./bin/rails db:reset

# Setup statistics
./bin/rails statistics:setup

# Run tests
./bin/rails test

# Run system tests
./bin/rails test:system

echo "Am Done"

kill -9 1