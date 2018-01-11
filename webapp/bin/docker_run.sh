#!/bin/bash
set -x -e

# remove previously started server pid
rm -f ./tmp/pids/server.pid

# Setup DB
./bin/rails db:create db:migrate

# Push all Package
./bin/rails packages:push_all &

# Start web server
./bin/rails server -b 0.0.0.0 -p 3000
