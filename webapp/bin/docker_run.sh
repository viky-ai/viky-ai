#!/usr/bin/env bash
set -e

source functions.sh

sigterm_handler() {
	echo "STOP signal received, try to gracefully shutdown all services..."

	if [[ -e ./tmp/pids/server.pid ]]; then
		pkill --signal SIGTERM --pidfile ./tmp/pids/server.pid
	fi

	if [[ -e ./tmp/pids/sidekiq.pid ]]; then
		pkill --signal SIGTERM --pidfile ./tmp/pids/sidekiq.pid
	fi

	# wait for stop
	wait
}

trap "sigterm_handler; exit" SIGTERM

# remove previously started server pid
rm -f ./tmp/pids/server.pid
rm -f ./tmp/pids/sidekiq.pid

# Check data migration
if [[ "$1" == "config" ]] || [[ "$1" == "master" ]]; then

	# Parse postgres and redis urls from Env Variables
	# docker-compose.yml -> x-app -> environment
	DB_POSTGRES=$(parse_url "$VIKYAPP_DB_HOST")
	DB_REDIS=$(parse_url "$VIKYAPP_CACHE_REDIS_URL")
	ES=$(parse_url "$VIKYAPP_STATISTICS_URL")

	echo "Waiting for postgres on $DB_POSTGRES"
	echo "Waiting for redis on $DB_REDIS"
	echo "Waiting for ES on $ES"
	/usr/local/bin/dockerize -wait tcp://$DB_POSTGRES:5432 -wait tcp://$DB_REDIS -timeout 60s -wait tcp://$ES -timeout 60s

	echo "Database and statistics setup"
	./bin/rails viky:setup
	echo "Database and statistics setup completed."
fi

if [[ "$1" != "config" ]]; then

	case "$1" in
	worker)
		# Start one worker
		bundle exec sidekiq -C config/sidekiq.yml &
		;;
	stats-rollover)
		echo "Statistics rollover"
		./bin/rails statistics:rollover
		;;
	*)
		echo "viky.ai will be available on ${VIKYAPP_PUBLIC_URL}"

		# Start web server
		./bin/rails server -b 0.0.0.0 -p 3000 &
		;;
	esac

	# wait for signal
	wait
fi
