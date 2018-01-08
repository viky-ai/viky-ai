#!/bin/bash
set -x -e

sleep 1

# Setup DB
./bin/rails db:reset

# Run tests
./bin/rails test

# Run system tests
./bin/rails test:system
