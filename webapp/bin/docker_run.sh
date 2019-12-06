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

  if [[ -e ./tmp/pids/fluent-bit.pid ]]; then
    pkill --signal SIGTERM --pidfile ./tmp/pids/fluent-bit.pid
  fi

  # wait for stop
  wait
}

trap "sigterm_handler; exit" SIGTERM

start_fluentbit() {
  /fluent-bit/bin/fluent-bit -c /webapp/config/statistics/fulent-bit.conf &
  echo $! > ./tmp/pids/fluent-bit.pid
}

# remove previously started server pid
rm -f ./tmp/pids/server.pid
rm -f ./tmp/pids/sidekiq.pid
rm -f ./tmp/pids/fluent-bit.pid

# init container Check data migration
POD_INDEX="${HOSTNAME##*-}"
if [[ "$1" == "create_db" ]]; then

  if [[ "$POD_INDEX" == "0" ]] ; then

    DB_POSTGRES=$(parse_url "$VIKYAPP_DB_HOST")
    DB_PORT=${VIKYAPP_DB_PORT:-5432}
    echo "Waiting for postgres on $DB_POSTGRES:$DB_PORT"
    /usr/local/bin/dockerize -wait tcp://$DB_POSTGRES:$DB_PORT -timeout 180s

    if [ "${VIKYAPP_DB_HOST}" == "" ];     then echo "env VIKYAPP_DB_HOST must be set"; exit 1; fi
    if [ "${VIKYAPP_DB_USERNAME}" == "" ]; then echo "env VIKYAPP_DB_USERNAME must be set"; exit 1; fi
    if [ "${VIKYAPP_DB_PASSWORD}" == "" ]; then echo "env VIKYAPP_DB_PASSWORD must be set"; exit 1; fi
    if [ "${VIKYAPP_DB_NAME}" == "" ];     then echo "env VIKYAPP_DB_NAME must be set"; exit 1; fi
    if [ "${DB_SUPERUSER}" == "" ];        then echo "env DB_SUPERUSER must be set"; exit 1; fi
    if [ "${DB_SU_PASSWORD}" == "" ];      then echo "env DB_SU_PASSWORD must be set"; exit 1; fi

    export PGPASSWORD="${DB_SU_PASSWORD}"

    # create DATABASE and USER if not exist
    psql --no-password -h "${VIKYAPP_DB_HOST}" -p ${VIKYAPP_DB_PORT:-5432} -U "${DB_SUPERUSER}" -d "postgres" <<- EOM
-- create DATABASE and USER if not exist
SELECT new.sql FROM (
  SELECT 'CREATE USER "${VIKYAPP_DB_USERNAME}" WITH ENCRYPTED PASSWORD ''${VIKYAPP_DB_PASSWORD}'';' AS sql, 1 as n
  WHERE NOT EXISTS (SELECT FROM pg_user WHERE usename = '${VIKYAPP_DB_USERNAME}')
  UNION
  SELECT 'CREATE DATABASE "${VIKYAPP_DB_NAME}" OWNER "${VIKYAPP_DB_USERNAME}";' AS sql, 2 as n
  WHERE NOT EXISTS (SELECT FROM pg_database WHERE datname = '${VIKYAPP_DB_NAME}')
) AS new
ORDER BY n ASC\gexec

-- siwtch database to  "${VIKYAPP_DB_NAME}" in order to change database schema owner
\connect "${VIKYAPP_DB_NAME}"
SELECT new.sql FROM (
  SELECT 'ALTER SCHEMA public OWNER TO "${VIKYAPP_DB_USERNAME}";' AS sql, 1 as n
  WHERE NOT EXISTS (SELECT FROM information_schema.schemata WHERE schema_name = 'public' AND schema_owner = '${VIKYAPP_DB_USERNAME}')
) AS new
ORDER BY n ASC\gexec
EOM

    echo "User '${VIKYAPP_DB_USERNAME}' and database '${VIKYAPP_DB_NAME}' created on ${VIKYAPP_DB_HOST}:${VIKYAPP_DB_PORT:-5432}."

  else
    echo "Setup is only done on first container."
  fi
elif [[ "$1" == "init" ]]; then

    if [[ "$POD_INDEX" == "0" ]] ; then


      # copy public assets to external container
      echo "Sync static content"
      mkdir -p /tmp/public/
      rsync -aq --delete-after ./public/packs/  /tmp/public/packs/
      rsync -aq --delete-after ./public/assets/ /tmp/public/assets/
      # create dir if not exist
      mkdir -p /tmp/public/uploads/

      # create a readable file to check runtime access
      touch /tmp/public/uploads/.readable
      touch /tmp/public/assets/.readable
      touch /tmp/public/packs/.readable

      # Parse postgres and redis urls from Env Variables
      DB_POSTGRES=$(parse_url "$VIKYAPP_DB_HOST")
      DB_PORT=${VIKYAPP_DB_PORT:-5432}
      DB_REDIS=$(parse_url "$VIKYAPP_CACHE_REDIS_URL")
      ES=$(parse_url "$VIKYAPP_STATISTICS_URL")
      KIBANA=$(parse_url "$VIKYAPP_STATISTICS_VISUALIZER_URL")

      echo "Waiting for postgres on $DB_POSTGRES:$DB_PORT"
      echo "Waiting for redis on $DB_REDIS"
      echo "Waiting for ES on $ES"
      /usr/local/bin/dockerize -wait tcp://$DB_POSTGRES:$DB_PORT -wait tcp://$DB_REDIS -wait tcp://$ES -wait tcp://$KIBANA  -timeout 180s

      echo "Database and statistics setup"
      ./bin/rails viky:setup
      echo "Database and statistics setup completed."

    else
      echo "Init is only done on first container."
    fi
else

  case "$1" in
  worker)
    # Start one worker
    start_fluentbit
    bundle exec sidekiq -C config/sidekiq.yml &
    ;;
  stats-rollover)
    echo "Statistics rollover"
    ./bin/rails statistics:rollover
    ;;
  *)
    echo "viky.ai will be available on ${VIKYAPP_PUBLIC_URL}"

    # Start web server
    start_fluentbit
    ./bin/rails server -b 0.0.0.0 -p 3000 &
    ;;
  esac

  # wait for signal
  wait
fi
