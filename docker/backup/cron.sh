#!/bin/bash

printenv | grep "PATH="     > /backup/env.sh
printenv | grep "VIKYAPP_" >> /backup/env.sh
printenv | grep "S3_"      >> /backup/env.sh

if [ "${VIKYAPP_AUTO_BACKUP}" == "true" ]; then
  echo "Starting backup cron ..." | (ts "[%Y-%m-%d %H:%M:%S%z  BACKUP]" >> /backup/backup.log)
  cron
else
  echo "Waitting for manual backup ..." | (ts "[%Y-%m-%d %H:%M:%S%z  BACKUP]" >> /backup/backup.log)
fi

tail -f /backup/backup.log
