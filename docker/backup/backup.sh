#!/bin/bash
set -e

local_dir_path=`readlink -f ${BASH_SOURCE[0]} | xargs dirname`
. "$local_dir_path/_common.sh"

# Main block to redirect log
function backup()
{
  NB_TO_KEEP=48

  export timestamp=$(date +"%Y-%m-%dt%H-%M-%Sz")
  export base_name=${VIKYAPP_BACKUP_NAME}_${timestamp}
  export target_dir=/backup_data/${VIKYAPP_BACKUP_NAME}/${timestamp}
  execute mkdir -p "${target_dir}"

  # Redis
  if [ "${VIKYAPP_REDIS_HOST}" != "" ]; then
    echo "Backup redis ${VIKYAPP_REDIS_HOST} ..."
    execute redis-cli -h "${VIKYAPP_REDIS_HOST}" --rdb "${target_dir}/${base_name}_${VIKYAPP_REDIS_HOST}-redis.rdb"
    execute gzip "${target_dir}/${base_name}_${VIKYAPP_REDIS_HOST}-redis.rdb"
    echo "Backup redis ${VIKYAPP_REDIS_HOST} DONE"
  fi

  # Directory
  echo "Backup directory ..."
  execute tar czPf ${target_dir}/${base_name}_app-uploads-data.tgz /webapp/public/uploads/store/
  echo "Backup directory DONE"

  # Postgres
  echo "Backup postgres ..."
  setup_db
  local PG_PATRONI_SCHEMA_EXCLUDE="--exclude-schema=metric_helpers --exclude-schema=user_management"
  execute pg_dump ${PG_PATRONI_SCHEMA_EXCLUDE} --no-password -h "${VIKYAPP_DB_HOST}" -p ${VIKYAPP_DB_PORT:-5432} -U "${VIKYAPP_DB_USERNAME}" -d "${VIKYAPP_DB_NAME}" | sed -e "s/ OWNER TO ${VIKYAPP_DB_USERNAME};/ OWNER TO superman;/g" | gzip > ${target_dir}/${base_name}_${VIKYAPP_DB_HOST}-postgresql.dump.gz
  echo "Backup postgres DONE"

  # Save on external storage
  setup_s3

  # ElasticSearch
  echo "Backup Elasticsearch ..."

  echo "Setup elasticsearch repository"
  executeCurl "PUT" "${VIKYAPP_STATISTICS_URL}/_snapshot/viky-es-backup_${VIKYAPP_BACKUP_NAME}?&pretty" \
    "{ \"type\": \"s3\", \"settings\": { \"region\": \"${S3_REGION}\", \"bucket\": \"${S3_BUCKET}\", \"base_path\":\"viky-backups/${VIKYAPP_BACKUP_NAME}/es-backup\" } }"

  echo "Snapshotting ${base_name} : "
  executeCurl "PUT" "${VIKYAPP_STATISTICS_URL}/_snapshot/viky-es-backup_${VIKYAPP_BACKUP_NAME}/${base_name}?wait_for_completion=true&pretty" \
    "{ \"indices\": \"stats-*\", \"ignore_unavailable\": true }"

  # Get a list of snapshots that we want to delete
  SNAPSHOTS_TO_DELETE=$(curl -s -XGET "${VIKYAPP_STATISTICS_URL}/_snapshot/viky-es-backup_${VIKYAPP_BACKUP_NAME}/_all" \
    | jq -r ".snapshots[:-${NB_TO_KEEP}][].snapshot")

  # Loop over the results and delete each snapshot
  for SNAPSHOT in $SNAPSHOTS_TO_DELETE
  do
    echo "Deleting OLD snapshot: $SNAPSHOT"
    executeCurl "DELETE" "${VIKYAPP_STATISTICS_URL}/_snapshot/viky-es-backup_${VIKYAPP_BACKUP_NAME}/$SNAPSHOT?pretty"
  done

  echo "Backup Elasticsearch DONE"

  # Push backup
  echo "push backup to S3 storage  on ${S3_ENDPOINT}/${S3_BUCKET}/viky-backups/${VIKYAPP_BACKUP_NAME} ..."

  execute s3cmd put  --stop-on-error --recursive \
    "/backup_data/${VIKYAPP_BACKUP_NAME}/${timestamp}"  "s3://${S3_BUCKET}/viky-backups/${VIKYAPP_BACKUP_NAME}/"
  execute s3cmd sync --stop-on-error --preserve --delete-removed --delete-after \
    "/backup_data/${VIKYAPP_BACKUP_NAME}/${timestamp}/" "s3://${S3_BUCKET}/viky-backups/${VIKYAPP_BACKUP_NAME}/_latest/"

  echo "push backup to S3 storage  DONE"

  # Remove local bakcup when done
  rm -rf ${target_dir}
}

backup 2>&1 | tee >(ts "[%Y-%m-%d %H:%M:%S%z  BACKUP]" >> /backup/backup.log)
