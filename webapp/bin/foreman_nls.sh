#!/usr/bin/env bash
set -e

sigterm_handler() {
  echo "STOP signal received, try to gracefully shutdown all services..."
  docker kill foreman_nls

  # wait for stop
  wait
}

trap "sigterm_handler; exit" SIGINT

PARAM_MY_CURRENT_GIT_BRANCH=$1

# Option --net=host is not avaibale on OSX you must set VIKYAPP_BASEURL and VIKYAPP_REDIS_PACKAGE_NOTIFIER
if [ "$OSX" ]; then
  NETWORK=' -p 9345:9345 '

  if [ "${VIKYAPP_BASEURL}" == "" ]; then
    echo "On OSX you must set VIKYAPP_BASEURL"
    exit 1
  fi
  if [ "${VIKYAPP_REDIS_PACKAGE_NOTIFIER}" == "" ]; then
    echo "On OSX you must set VIKYAPP_REDIS_PACKAGE_NOTIFIER"
    exit 1
  fi
else
  NETWORK=' --net=host '
fi

if [ "${VIKYAPP_BASEURL}" ]; then
  PARAM_VIKYAPP_BASEURL=" -e VIKYAPP_BASEURL=${VIKYAPP_BASEURL} "
fi

if [ "${VIKYAPP_REDIS_PACKAGE_NOTIFIER}" ]; then
  PARAM_VIKYAPP_REDIS_PACKAGE_NOTIFIER=" -e VIKYAPP_REDIS_PACKAGE_NOTIFIER=${VIKYAPP_REDIS_PACKAGE_NOTIFIER} "
fi

echo "Kill previews running NLS"
docker kill foreman_nls > /dev/null 2>&1 || true

echo "Pull NLS docker image : docker-registry.pertimm.net/viky.ai/platform/nlp${PARAM_MY_CURRENT_GIT_BRANCH}"
docker pull docker-registry.pertimm.net/viky.ai/platform/nlp${PARAM_MY_CURRENT_GIT_BRANCH}

echo "Run NLS ..."
docker run \
  ${PARAM_VIKYAPP_BASEURL} \
  ${PARAM_VIKYAPP_REDIS_PACKAGE_NOTIFIER} \
  ${NETWORK} \
  -t --rm --name foreman_nls --init \
  docker-registry.pertimm.net/viky.ai/platform/nlp${PARAM_MY_CURRENT_GIT_BRANCH}

