#!/bin/bash
set -x -e

sleep 1

# Setup DB
./bin/rails db:environment:set
./bin/rails db:setup db:migrate

# Run tests
./bin/rails test

./bin/rails test:system
