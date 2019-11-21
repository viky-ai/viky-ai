#!/bin/bash
set -e

local_dir_path=`readlink -f ${BASH_SOURCE[0]} | xargs dirname`
. "$local_dir_path/_common.sh"

# Main block to redirect log
function restore()
{
  if [ "$1" == "" ]; then
    export RESTORE_FROM="viky-ai-production"
  else
    export RESTORE_FROM="$1"
  fi
  if [ "$2" == "" ]; then
    export RESTORE_POINT="_latest"
  else
    export RESTORE_POINT="$2"
  fi

  if [ "${RESTORE_FROM}" == "${VIKYAPP_BACKUP_NAME}" ]; then
    echo "You cannot restore TO ${VIKYAPP_BACKUP_NAME} FROM ${RESTORE_FROM}"
    exit 1
  fi

  export RESTORE_DIR="/restore_data"

  setup_s3
  echo "Retrive latest backup from S3 storage on ${S3_ENDPOINT}/${S3_BUCKET}/viky-backups/${RESTORE_FROM}/${RESTORE_POINT} ..."
  execute rm -rf   "${RESTORE_DIR}"
  execute mkdir -p "${RESTORE_DIR}"
  execute s3cmd sync --preserve --exclude="*.rbd"  \
    "s3://${S3_BUCKET}/viky-backups/${RESTORE_FROM}/${RESTORE_POINT}/" "${RESTORE_DIR}/"
  echo "Retrive latest backup DONE"

  # Postgres
  echo "Restore postgres on ${VIKYAPP_DB_HOST} ..."

  setup_db

  # drop preview database
  execute psql --no-password -h "${VIKYAPP_DB_HOST}" -U "${VIKYAPP_DB_USERNAME}" -p ${VIKYAPP_DB_PORT:-5432} -d "${VIKYAPP_DB_NAME}" -c "\"DROP SCHEMA public CASCADE; CREATE SCHEMA public; GRANT ALL ON SCHEMA public TO public;\""

  # restore data
  execute zcat ${RESTORE_DIR}/*-postgresql.dump.gz | sed -e "s/ OWNER TO superman;/ OWNER TO ${VIKYAPP_DB_USERNAME};/g" | psql --no-password -h "${VIKYAPP_DB_HOST}" -p ${VIKYAPP_DB_PORT:-5432} -U "${VIKYAPP_DB_USERNAME}" -d "${VIKYAPP_DB_NAME}"
  echo "Restore postgres DONE"

  # Directory
  echo "Restore directory ..."
  execute rm -rf /webapp/public/uploads/*

	mkdir -p /webapp/public/uploads/cache
	mkdir -p /webapp/public/uploads/store

  UPLOADS_ABSOLUTE_PATH=`tar tzPf ${RESTORE_DIR}/*_app-uploads-data.tgz | grep /webapp/public/uploads | wc -l`
  if [ ${UPLOADS_ABSOLUTE_PATH} -gt 0 ]; then
    # absolute path
    execute tar xzP -f ${RESTORE_DIR}/*_app-uploads-data.tgz /webapp/public/uploads
  else
    # relative path
    execute tar xzP -C /webapp/ -f ${RESTORE_DIR}/*_app-uploads-data.tgz public/ -C /webapp/
  fi

  echo "Restore directory DONE"

  # ElasticSearch
  echo "Restoring Stats in ES ..."

  echo "Setup elasticsearch repository"
  executeCurl "PUT" "${VIKYAPP_STATISTICS_URL}/_snapshot/viky-es-restore" \
    "{ \"type\": \"s3\", \"settings\": { \"region\": \"${S3_REGION}\", \"bucket\": \"${S3_BUCKET}\", \"readonly\": true, \"base_path\":\"viky-backups/${RESTORE_FROM}/es-backup\" } }"

  echo "Delete all stats existing Indexes"
  executeCurl "DELETE" "${VIKYAPP_STATISTICS_URL}/stats-*"

  # Get last snapshot name
  SNAPSHOT_TO_RESTORE=`curl -s -XGET "${VIKYAPP_STATISTICS_URL}/_snapshot/viky-es-restore/_all" \
    | jq -r ".snapshots[-1].snapshot"`

  echo "Restoring existing stats from ${RESTORE_FROM}/${SNAPSHOT_TO_RESTORE}: "
  executeCurl "POST" "${VIKYAPP_STATISTICS_URL}/_snapshot/viky-es-restore/${SNAPSHOT_TO_RESTORE}/_restore?wait_for_completion=true" \
    "{ \"indices\": \"stats-*\", \"include_aliases\": true, \"allow_no_indices\": true, \"ignore_unavailable\": true }"

  echo "Restore Stats DONE"
}

restore $@ 2>&1 | tee >(ts "[%Y-%m-%d %H:%M:%S%z RESTORE]" >> /backup/backup.log)
