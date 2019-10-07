#!/bin/bash
set -e

source functions.sh

sleep 1

# Parse postgres, redis and es urls from Env Variables
# .gitlab-ci -> test_webapp -> variables
DB_POSTGRES=$(parse_url "$VIKYAPP_DB_HOST")
DB_REDIS=$(parse_url "$VIKYAPP_ACTIONCABLE_REDIS_URL")
ES=$(parse_url "$VIKYAPP_STATISTICS_URL")

echo "DB_POSTGRES $DB_POSTGRES"
echo "DB_REDIS $DB_REDIS"
echo "ES $ES"

# wait for services
/usr/local/bin/dockerize -wait tcp://$DB_POSTGRES:5432 -wait tcp://$DB_REDIS -wait tcp://$ES -timeout 60s

# Setup DB
./bin/rails db:reset

# Setup statistics
./bin/rails statistics:setup

# Run tests
COVERAGE=1 DISABLE_SPRING=1 ./bin/rails test

# Run system tests
PARALLEL_WORKERS=4 COVERAGE=1 DISABLE_SPRING=1 ./bin/rails test:system
