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

if [ "${VIKYAPP_BASEURL}" ] && [ "${VIKYAPP_REDIS_PACKAGE_NOTIFIER}" ]
then
  # wait for services
  /usr/local/bin/dockerize -wait ${VIKYAPP_REDIS_PACKAGE_NOTIFIER/redis/tcp} -wait $VIKYAPP_BASEURL -timeout 60s
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
