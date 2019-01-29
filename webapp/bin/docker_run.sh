#!/usr/bin/env bash
set -e

source functions.sh

sigterm_handler() {
  echo "STOP signal received, try to gracefully shutdown all services..."

  if [[ -e ./tmp/pids/server.pid ]]; then
    pkill --signal SIGTERM --pidfile ./tmp/pids/server.pid
  fi

  if [[ -e ./tmp/pids/sidekiq.pid ]]; then
    pkill --signal SIGTERM --pidfile ./tmp/pids/sidekiq.pid
  fi

  # wait for stop
  wait
}

trap "sigterm_handler; exit" SIGTERM

# remove previously started server pid
rm -f ./tmp/pids/server.pid
rm -f ./tmp/pids/sidekiq.pid

# Check data migration
if [[ "$1" == "config" ]] || [[ "$1" == "master" ]]; then

  # Parse postgres and redis urls from Env Variables
  # docker-compose.yml -> x-app -> environment
  DB_POSTGRES=$(parse_url "$VIKYAPP_DB_HOST")
  DB_REDIS=$(parse_url "$VIKYAPP_CACHE_REDIS_URL")

  echo "Waiting for postgres on $DB_POSTGRES"
  echo "Waiting for redis on $DB_REDIS"

  # wait for services
  /usr/local/bin/dockerize -wait tcp://$DB_POSTGRES:5432 -wait tcp://$DB_REDIS -timeout 120s

  echo "Database setup"
  echo "Check if database exist ..."

  DB_TMP_VERSION=$(./bin/rails db:version | grep "Current version" 2>/dev/null)
  DB_CREATED=$?
  DB_VERSION=0
  if [ $DB_CREATED -eq 0 ]; then
    DB_VERSION=$(echo "${DB_TMP_VERSION}" | cut -d":" -f2)
  fi

  if [ $DB_CREATED -ne 0 -o $DB_VERSION -eq 0 ]; then
    echo "Database schema missing or empty, try to create ...\n"
    ./bin/rails db:setup
  else
    echo "Try to run db:migrate from schema version ${DB_VERSION} ..."
    ./bin/rails db:migrate
  fi

  # Parse Elastic Search url from Env Variables
  # docker-compose.yml -> x-app -> environment
  echo "Database setup completed."

  ES=$(parse_url "$VIKYAPP_STATISTICS_URL")
  echo "Waiting for ES on $ES"

  # wait for services
  /usr/local/bin/dockerize -wait tcp://$ES -timeout 60s

  echo "Statistics setup"
  ./bin/rails statistics:setup

  echo "Statistics reindexing if needed"
  ./bin/rails statistics:reindex:all

  echo "Statistics setup completed."

  echo "Rake tasks scheduler setup."
  bundle exec whenever --update-crontab
fi

if [[ "$1" != "config" ]]; then

  case "$1" in
  worker)
    # Start one worker
    bundle exec sidekiq -C config/sidekiq.yml &
    ;;
  stats-rollover)
    echo "Statistics rollover"
    ./bin/rails statistics:rollover
    ;;
  *)
    echo "viky.ai will be available on ${VIKYAPP_BASEURL}"

    # Start web server
    ./bin/rails server -b 0.0.0.0 -p 3000 &
    ;;
  esac

  # wait for signal
  wait
fi
