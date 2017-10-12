# README

## Dependencies

* Ruby (2.4.2)
* bundler (1.15.4)
* NodeJS (8.4.0)
* Yarn (1.0.1)
* PostgreSQL (9.6.5)
* Redis (4.0.1)
* ImageMagick

## Environment Variables

The application makes extensive use of environment variables to keep the configuration flexible across every development environment.
You can define your local variables in a `.env` file and place it in the root of the webapp project,
they will be loaded into the environment at the application bootstrap and available within your code as usual, via `ENV['EXAMPLE_VAR']`.

Beware of never committing or pushing the `.env` file, it is just useful for your local webapp instance to match your local configuration.

For example you could define your local PostgreSQL username, password and other variables like your favourite Redis endpoint for ActionCable and ActiveJob, maybe it's a remote machine, maybe your localhost, e.g.:

```
VOQALAPP_DB_USERNAME=superman
VOQALAPP_DB_PASSWORD='sup$_3rman'
VOQALAPP_DB_HOST=localhost
VOQALAPP_DB_PORT=5432

VOQALAPP_ACTIONCABLE_REDIS_URL='redis://localhost:6379/1'
VOQALAPP_ACTIVEJOB_REDIS_URL='redis://localhost:7372/1'
```

## Run in production environment

    RAILS_ENV=production rails db:setup
    RAILS_ENV=production rails assets:clobber
    RAILS_ENV=production rails assets:precompile
    RAILS_ENV=production SECRET_KEY_BASE=xyz POSTMARK_TOKEN=wxyz RAILS_SERVE_STATIC_FILES=true rails s

## Base url

<code>VOQALAPP_BASEURL</code> environment variable must be set.

## Mail

We use postmarkapp.com in order to send emails in `production` and `development` environment. Environment variable `POSTMARK_TOKEN` must be defined for `production` environment (default one is set for `development`).
Emails from Devise are sent with `support@voqal.ai` sender.

Email address `postmarkapp@voqal.ai` is used to connect to postmarkapp.com UIs.

Mail delivery is performed through a high priority queue named `webapp_mailers` (see _Background jobs_ paragraph for details).


## Admin user

Admin users can access to `/backend/users` UI. In order to create admin user, you can use the Rails tasks:

* `./bin/rails users:create_admin[email,password]`
* `./bin/rails users:set_admin[email]`
* `./bin/rails users:unset_admin[email]`


## Background jobs

Asynchronous tasks, like mail delivery and other, are lazily performed making use of a job queue based on ActiveJob + Sidekiq + Redis.

The configuration file for queue definition and options is `config/sidekiq.yml`, you can therein define as many queues as you need and route them in your Sidekiq workers and ActiveJob jobs.

Default concurrency for background job management is set to 5, you can change it via environment variable `VOQALAPP_SIDEKIQ_CONCURRENCY`.


## Boostrap development databases

The first time you start the application you need to manually create two databases `voqalapp_development` and `voqalapp_test` :

```
$ sudo -i -u postgres
## Become postgres user
$ createuser --interactive
...
ALTER USER "<user>" WITH PASSWORD '<password>';

$ ./bin/rails db:setup
> Created database 'voqalapp_development'
> Created database 'voqalapp_test'
> ...
```

## Start all in development

To ensure having all those processes up and running during development, you can simply run:

    foreman start

which reads the `Procfile` behind the scenes, bringing the web server and workers up:

    sidekiq_redis:  redis-server --port 7372
    workers:        bundle exec sidekiq -C config/sidekiq.yml
    webpack:        ./bin/webpack-dev-server
    web:            bundle exec rails s -p 3000
