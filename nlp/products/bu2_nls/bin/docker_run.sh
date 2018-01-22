#!/usr/bin/env bash
set -e

sigterm_handler() {
  echo "STOP signal received, try to gracefully shutdown NLP ..."

  if [[ -e ./ogm_nls.pid ]] ; then
    pkill --signal SIGTERM --pidfile ./ogm_nls.pid
  fi

  # wait for stop
  wait
}

trap "sigterm_handler; exit" SIGTERM

# remove previously started server pid
rm -f ./ogm_nls.pid

# wait for services
/usr/local/bin/dockerize -wait tcp://db-redis:6379 -timeout 60s

# Start nlp in background, not in daemon
./ogm_nls &

# wait for signal
wait
