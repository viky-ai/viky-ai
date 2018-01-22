#!/usr/bin/env bash

if [[ "${VIKYAPP_WORKER}" == "true" ]] ; then
  # TODO Implement Sideqik HealthCheck
  exit 0
else
  curl -f http://localhost:3000/database_check.json || exit 1
fi
