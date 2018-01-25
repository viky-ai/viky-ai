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

if [ "${VIKYAPP_REDIS_PACKAGE_NOTIFIER}" == "" ] ; then
  # wait for services
  /usr/local/bin/dockerize -wait tcp://localhost:6379 -wait http://localhost:3000 -timeout 60s
else
  # wait for services
  /usr/local/bin/dockerize -wait tcp://db-redis:6379 -wait http://app-master:3000 -timeout 600s
fi

# Start nlp in background, not in daemon
cd /nlp_route && bundle exec ./bin/nlp-route &

# wait for signal
wait
