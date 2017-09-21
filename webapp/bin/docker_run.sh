#!/bin/bash
set -x -e

# Setup DB
./bin/rails db:create db:migrate

# Start web server
./bin/rails server -b 0.0.0.0 -p 3000
