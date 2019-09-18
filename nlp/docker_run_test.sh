#!/usr/bin/env bash
set -e

cd /nlp_tests       && bundle exec rake test

if [ "${VIKYAPP_REDIS_PACKAGE_NOTIFIER}" != "" ]; then
  cd /nlp_route_tests && bundle exec rake test
else
  echo "Skip nlp_route_tests."
fi
