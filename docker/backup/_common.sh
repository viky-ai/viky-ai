#!/bin/bash

# load env for cron
if [ -f /backup/env.sh ]; then
  source /backup/env.sh
fi

function execute()
{
  local cmd=$@
  local RESULT=0

  eval "$cmd"
  RESULT=$?

  echo ""

  if [ $RESULT -gt 0 ]
  then
    echo "[ERROR] $cmd"
    exit $RESULT
  fi
}

function executeCurl()
{
  local METHOD=$1
  local URL=$2
  local DATA=$3

  local DATA_OPTION=""
  # Check if variable exists
  if [ -z ${METHOD+x} ];     then echo "METHOD must be set"; exit 1; fi
  if [ -z ${URL+x} ];     then echo "URL must be set"; exit 1; fi

  # store the whole response with the status at the and
  if [[ "${DATA}" != "" ]]; then
    DATA_OPTION="-d @-"
  fi


  local HTTP_RESPONSE=$(echo $DATA | curl --silent --write-out "HTTPSTATUS:%{http_code}" -X $METHOD "${URL}" -H 'Content-Type: application/json' $DATA_OPTION)

  # extract the body
  local HTTP_BODY=$(echo $HTTP_RESPONSE | sed -e 's/HTTPSTATUS\:.*//g')

  # extract the status
  local HTTP_STATUS=$(echo $HTTP_RESPONSE | tr -d '\n' | sed -e 's/.*HTTPSTATUS://')

  # print the body
  echo "$HTTP_BODY" | jq .

  # example using the status
  if [[ "${HTTP_STATUS}" != "200" ]]; then
    echo "Error [HTTP status: ${HTTP_STATUS}] ${METHOD} ${URL}"
    exit 1
  fi
}

function setup_s3()
{
  # setup S3 server
  if [ "${S3_ENDPOINT}" == "" ]; then
    if [ "${S3_REGION}" == "" ]; then
      export S3_REGION="eu-west-3"
    fi
    export S3_ENDPOINT="s3.amazonaws.com"
  fi

  if [ "${S3_BUCKET}" == "" ]; then
    export S3_BUCKET="viky-ai"
  fi

  if [ "${S3_ACCESS_KEY}" == "" ]; then echo "env S3_ACCESS_KEY must be set"; exit 1; fi
  if [ "${S3_SECRET_KEY}" == "" ]; then echo "env S3_SECRET_KEY must be set"; exit 1; fi
  if [ "${S3_ENDPOINT}" == "" ];   then echo "env S3_ENDPOINT must be set"; exit 1; fi

  # https://github.com/s3tools/s3cmd/wiki/How-to-configure-s3cmd-for-alternative-S3-compatible-services
  cat > ${HOME}/.s3cfg <<EOL
access_key=${S3_ACCESS_KEY}
secret_key=${S3_SECRET_KEY}
host_base=${S3_ENDPOINT}
host_bucket=${S3_BUCKET}
bucket_location=${S3_REGION}
human_readable_sizes=True
follow_symlinks=True
stats=True
EOL

  echo "Setup S3 storage  on ${S3_ENDPOINT}/${S3_BUCKET}/viky-backups/."

}


function setup_db()
{
  if [ "${VIKYAPP_DB_HOST}" == "" ];     then echo "env VIKYAPP_DB_HOST must be set"; exit 1; fi
  if [ "${VIKYAPP_DB_USERNAME}" == "" ]; then echo "env VIKYAPP_DB_USERNAME must be set"; exit 1; fi
  if [ "${VIKYAPP_DB_PASSWORD}" == "" ]; then echo "env VIKYAPP_DB_PASSWORD must be set"; exit 1; fi
  if [ "${VIKYAPP_DB_NAME}" == "" ];     then echo "env VIKYAPP_DB_NAME must be set"; exit 1; fi

  export PGPASSWORD="${VIKYAPP_DB_PASSWORD}"
}

if [ "${VIKYAPP_BACKUP_NAME}" == "" ]; then
  export VIKYAPP_BACKUP_NAME=${VIKYAPP_DEPLOY_RANCHER_STACK}
fi

if [ "${VIKYAPP_BACKUP_NAME}" == "" ];     then echo "env VIKYAPP_BACKUP_NAME must be set"; exit 1; fi
if [ "${VIKYAPP_STATISTICS_URL}" == "" ];  then echo "env VIKYAPP_STATISTICS_URL must be set"; exit 1; fi