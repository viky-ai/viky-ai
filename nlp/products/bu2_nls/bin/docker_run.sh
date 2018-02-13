#!/usr/bin/env bash
set -e

NLP_ROUTE_PID=''

sigterm_handler() {
  echo "STOP signal received, try to gracefully shutdown NLP ..."

  if [[ "$NLP_ROUTE_PID" != '' ]] ; then
    kill -SIGINT $NLP_ROUTE_PID
  fi

  # wait for stop
  wait
}

trap "sigterm_handler; exit" SIGTERM

# remove previously started server pid
rm -f ./ogm_nls.pid

if [ "${DOCKER_COMPOSE_DEPLOY}" == "true" ] ; then
  # wait for services
  /usr/local/bin/dockerize -wait tcp://db-redis:6379 -wait http://lb-internal-app:3000 -timeout 600s
else
  # wait for services
  /usr/local/bin/dockerize -wait tcp://localhost:6379 -wait http://localhost:3000 -timeout 60s
fi

# Start nlp in background, not in daemon
cd /nlp_route
bundle exec ./bin/nlp-route &
NLP_ROUTE_PID=$!

# wait for signal
wait
