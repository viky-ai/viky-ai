#!/usr/bin/env bash
set -e

sigterm_handler() {
  echo "STOP signal received, try to gracefully shutdown all services..."

  if [[ -e ./tmp/pids/server.pid ]] ; then
    pkill --signal SIGTERM --pidfile ./tmp/pids/server.pid
  fi

  if [[ -e ./tmp/pids/sidekiq.pid ]] ; then
    pkill --signal SIGTERM --pidfile ./tmp/pids/sidekiq.pid
  fi

  # wait for stop
  wait
}

trap "sigterm_handler; exit" SIGTERM

# remove previously started server pid
rm -f ./tmp/pids/server.pid
rm -f ./tmp/pids/sidekiq.pid


if [[ "$1" == "config" ]] ; then

  # wait for services
  /usr/local/bin/dockerize -wait tcp://db-postgresql:5432 -wait tcp://db-redis:6379 -timeout 60s

  # Setup DB
  echo "Try to run db:migrate ..."
  ./bin/rails db:create db:migrate

elif [[ "$1" == "worker" ]] ; then

  # Start one worker
  bundle exec sidekiq -C config/sidekiq.yml &

else

  echo "viky.ai will be available on ${VIKYAPP_BASEURL}"

  # Start web server
  ./bin/rails server -b 0.0.0.0 -p 3000 &

fi

# wait for signal
wait
