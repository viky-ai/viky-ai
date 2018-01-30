#!/usr/bin/env bash
set -e

cd /nlp_tests       && bundle exec rake test
cd /nlp_route_tests && bundle exec rake test
