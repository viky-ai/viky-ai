#!/usr/bin/env bash
set -ex

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

if [ "${DOCKER_COMPOSE_DEPLOY}" == "true" ] ; then
  # wait for services
  /usr/local/bin/dockerize -wait tcp://db-redis:6379 -wait http://webapp-service:3000 -timeout 600s
else
  # wait for services
  /usr/local/bin/dockerize -wait tcp://localhost:6379 -wait http://localhost:3000 -timeout 60s
fi

# Start nlp in background, not in daemon
cd /nlp_route && bundle exec ./bin/nlp-route &

# wait for signal
wait
