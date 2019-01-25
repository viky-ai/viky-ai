#!/bin/bash
set -x -e
source ./functions.sh

sleep 1

DB_POSTGRES = echo $(parse_url "$VIKYAPP_DB_HOST")
DB_REDIS = echo $(parse_url "$VIKYAPP_ACTIONCABLE_REDIS_URL")
ES = echo $(parse_url "$VIKYAPP_STATISTICS_URL")
KIBANA = echo $(parse_url "$VIKYAPP_STATISTICS_VISUALIZER_URL")

echo "DB_POSTGRES $DB_POSTGRES"
echo "DB_REDIS $DB_REDIS"
echo "ES $ES"
echo "KIBANA $KIBANA"

# wait for services
/usr/local/bin/dockerize -wait tcp://$DB_POSTGRES:5432 -wait tcp://$DB_REDIS -wait tcp://$ES -wait tcp://$KIBANA -timeout 60s

export DISABLE_SPRING=true

# Setup DB
./bin/rails db:reset

# Setup statistics
./bin/rails statistics:setup

# Run tests
./bin/rails test

# Run system tests
./bin/rails test:system
