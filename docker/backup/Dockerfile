# Using docker multi stage (docker >= 17.05)
# https://codefresh.io/blog/node_docker_multistage/

# docker build -t vikyai/tools_backup .

#################################################
# Backup image
#################################################

FROM debian:buster AS backup

# Install dependencies
RUN set -ex; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      ca-certificates \
      cron \
      curl \
      dirmngr \
      gnupg \
      jq \
      moreutils \
      openssh-client \
      postgresql-client-11 \
      python3-magic \
      python3-pip \
      libssl1.1 \
      locales \
      redis-tools \
      vim \
      wget \
    ; \
    apt-get clean; \
    rm -rf /var/lib/apt/lists/* ; \
    pip3 install s3cmd --no-cache-dir

# make the "en_US.UTF-8" locale so postgres will be utf-8 enabled by default
RUN localedef -i en_US -c -f UTF-8 -A /usr/share/locale/locale.alias en_US.UTF-8
ENV LANG en_US.utf8

# backup script
COPY _common.sh  /backup/_common.sh
COPY backup.sh  /backup/backup.sh
COPY restore.sh /backup/restore.sh
COPY cron.sh    /backup/cron.sh
RUN  touch      /backup/env.sh

VOLUME /backup_data

# Add crontab file in the cron directory
ADD crontab /etc/cron.d/backup-cron

# Give execution rights on the cron job
RUN chmod 0644 /etc/cron.d/backup-cron

# Create the log file to be able to run tail
RUN touch /var/log/cron.log

CMD ["/backup/cron.sh"]
