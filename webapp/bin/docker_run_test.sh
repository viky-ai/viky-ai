#!/bin/bash
set -e

source functions.sh

# Parse postgres, redis, es and kibana urls from Env Variables
# .gitlab-ci -> test_webapp -> variables
DB_POSTGRES=$(parse_url "$VIKYAPP_DB_HOST")
DB_REDIS=$(parse_url "$VIKYAPP_ACTIONCABLE_REDIS_URL")
ES=$(parse_url "$VIKYAPP_STATISTICS_URL")

echo "DB_POSTGRES $DB_POSTGRES"
echo "DB_REDIS $DB_REDIS"
echo "ES $ES"
echo "Test ${CI_NODE_INDEX}/${CI_NODE_TOTAL}"

# wait for services
/usr/local/bin/dockerize -wait tcp://$DB_POSTGRES:5432 -wait tcp://$DB_REDIS -wait tcp://$ES -timeout 120s

export DISABLE_SPRING=true

# Setup DB
./bin/rails db:reset

# Setup statistics
./bin/rails statistics:setup

if [[ "$1" == "unit" ]] || [[ "$1" == "" ]]; then

  # Run tests
  ./bin/rails test

fi

if [[ "$1" == "system" ]] || [[ "$1" == "" ]]; then

  # Run system tests
  ./bin/rails test:system

fi