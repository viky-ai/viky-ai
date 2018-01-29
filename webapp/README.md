# README

## Dependencies

* Ruby (2.4.2)       ( https://github.com/rbenv/rbenv#installation )
* bundler (1.15.4)   ( `gem install bundler` )
* NodeJS (8.4.0)     ( https://nodejs.org/en/download/package-manager/#debian-and-ubuntu-based-linux-distributions )
* Yarn (1.0.1)       ( https://yarnpkg.com/lang/en/docs/install/ )
* PostgreSQL (9.6.5) ( `sudo apt-get install -y postgresql postgresql-contrib libpq-dev` )
* Redis (4.0.1)      ( use docker )
* ImageMagick        ( `sudo apt-get install -y graphicsmagick-imagemagick-compat` )
* Docker 17.09.0-ce  ( [see below](#docker) )
* Graphviz           ( `sudo apt-get install -y graphviz` )

## Environment Variables

The application makes extensive use of environment variables to keep the configuration flexible across every development environment.
You can define your local variables in a `.env` file and place it in the root of the webapp project,
they will be loaded into the environment at the application bootstrap and available within your code as usual, via `ENV['EXAMPLE_VAR']`.

Beware of never committing or pushing the `.env` file, it is just useful for your local webapp instance to match your local configuration.

For example you could define your local PostgreSQL username, password and other variables like your favourite Redis endpoint for ActionCable and ActiveJob, maybe it's a remote machine, maybe your localhost, e.g.:

```
MY_CURRENT_GIT_BRANCH=:`git describe --all --abbrev=0 --always --contains | sed 's|[~^].*||' | sed 's|remotes/origin/||' | sed 's|heads/||' | sed 's|tags/||' | sed 's|/|-|g' | sed 's|_|-|g'`


VIKYAPP_DB_USERNAME=superman
VIKYAPP_DB_PASSWORD='sup$_3rman'
VIKYAPP_DB_HOST=localhost
VIKYAPP_DB_PORT=5432

VIKYAPP_CACHE_REDIS_URL='redis://localhost:6379/0'
VIKYAPP_ACTIVEJOB_REDIS_URL='redis://localhost:6379/1'
VIKYAPP_ACTIONCABLE_REDIS_URL='redis://localhost:6379/2'

VIKYAPP_INTERNAL_API_TOKEN=Uq6ez5IUdd

VIKYAPP_NLP_URL='http://localhost:9345'

# restore env password
RSYNC_PASSWORD='#***REMOVED***26'
```

## Run in production environment

    RAILS_ENV=production rails db:setup
    RAILS_ENV=production rails assets:clobber
    RAILS_ENV=production rails assets:precompile
    RAILS_ENV=production SECRET_KEY_BASE=xyz POSTMARK_TOKEN=wxyz RAILS_SERVE_STATIC_FILES=true rails s

## Base url

<code>VIKYAPP_BASEURL</code> environment variable must be set.

## Mail

We use postmarkapp.com in order to send emails in `production` and `development` environment. Environment variable `POSTMARK_TOKEN` must be defined for `production` environment (default one is set for `development`).
Emails from Devise are sent with `support@viky.ai` sender.

Email address `postmarkapp@viky.ai` is used to connect to postmarkapp.com UIs.

Mail delivery is performed through a high priority queue named `webapp_mailers` (see _Background jobs_ paragraph for details).


## Admin user

Admin users can access to `/backend/users` UI. In order to create admin user, you can use the Rails tasks:

* `./bin/rails users:invite_admin[email]`
* `./bin/rails users:set_admin[email]`
* `./bin/rails users:unset_admin[email]`


## Background jobs

Asynchronous tasks, like mail delivery and other, are lazily performed making use of a job queue based on ActiveJob + Sidekiq + Redis.

The configuration file for queue definition and options is `config/sidekiq.yml`, you can therein define as many queues as you need and route them in your Sidekiq workers and ActiveJob jobs.

Default concurrency for background job management is set to 5, you can change it via environment variable `VIKYAPP_SIDEKIQ_CONCURRENCY`.


## Bootstrap development databases

The first time you start the application you need to manually create two databases `vikylapp_development` and `vikylapp_test` :

### Create postgres user
```
$ sudo -u postgres createuser --interactive -W
...
```

### Create database
```
$ ./bin/rails db:setup
> Created database 'vikylapp_development'
> Created database 'vikylapp_test'
> ...
```

## Docker

If you haven't Docker installed in your system yet, you can follow this [guide](https://docs.docker.com/engine/installation/).
Then check to have the image registry mirrors well configured.
In Ubuntu you should:
1. stop the Docker daemon `service docker stop`
2. check to have the configuration file `/etc/docker/daemon.json` containing:
```
{
    "registry-mirrors": [
      "http://docker-mirror.pertimm.corp:50000"
    ]
}
```
3. add your current user into the `docker` system group: `sudo usermod -a -G groupName userName`
4. you will need to logout and log back in for this change to take effect
5. be sure the Docker daemon is started: `service docker start`

## Start the webapp in development

To ensure having all the webapp related processes up and running during development, you can simply run:

    `foreman start`

If you obtain an authorization error from `docker`, you probably have to do:

    docker login docker-registry.pertimm.net

and give your `username` and `password` of the Pertimm services.

## Doc

Slate is used to generate a doc static web site. Souces are in ../doc. Before running <code>foreman start</code>, install doc component dependencies, run:

    bundle install
