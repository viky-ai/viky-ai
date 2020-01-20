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

if [ "${VIKYAPP_INTERNAL_URL}" ]
then
  WAIT_VIKYAPP="-wait ${VIKYAPP_INTERNAL_URL}"
else
  WAIT_VIKYAPP="-wait http://localhost:3000"
fi

if [ "${VIKYAPP_REDIS_PACKAGE_NOTIFIER}" ]
then
  WAIT_REDIS="-wait ${VIKYAPP_REDIS_PACKAGE_NOTIFIER/redis/tcp}"
else
  WAIT_REDIS="-wait tcp://localhost:6379"
fi

if   [[ "$1" == "init" ]]; then
  /usr/local/bin/dockerize ${WAIT_REDIS} ${WAIT_VIKYAPP} -timeout 240s
else
  /usr/local/bin/dockerize ${WAIT_REDIS} ${WAIT_VIKYAPP} -timeout 60s

  # Start nlp in background, not in daemon
  cd /nlp_route
  bundle exec ./bin/nlp-route &
  NLP_ROUTE_PID=$!
  touch ${NLS_INSTALL_PATH}/log/ogm_nls.log
  tail -f --pid=${NLP_ROUTE_PID} ${NLS_INSTALL_PATH}/log/ogm_nls.log

  # wait for signal
  wait
fi
