# README

## Dependencies

* Ruby (2.4.2)         ( https://github.com/rbenv/rbenv#installation )
* bundler (1.15.4)     ( `gem install bundler` )
* NodeJS (10+)         ( https://nodejs.org/en/download/package-manager/#debian-and-ubuntu-based-linux-distributions )
* Yarn (1.13+)         ( https://yarnpkg.com/lang/en/docs/install/ )
* PostgreSQL (9.6.5)   ( `sudo apt-get install -y postgresql postgresql-contrib libpq-dev` )
* Redis (3.2)          ( `sudo apt-get install redis-server` or `sudo apt-get install redis` )
* ElasticSearch (6.6)  ( use docker : `docker pull docker.elastic.co/elasticsearch/elasticsearch:6.6.1`)
* Cerebro              ( use docker : `docker pull yannart/cerebro` )
* Kibana               ( use docker : `docker pull docker.elastic.co/kibana/kibana:6.6.1` )
* ImageMagick          ( `sudo apt-get install -y graphicsmagick-imagemagick-compat` )
* Graphviz             ( `sudo apt-get install -y graphviz` )
* Docker 17.09.0-ce    ( [see below](#docker) )

## Install a dev environment

This section will guide you to install a dev environment on a new machine.
This is a recommended way for **Ubuntu 18.04** but feel free to customize it if you know what you are doing.
If you need more information, see sections below.

1. Install Ruby : `sudo apt install ruby ruby-dev`
2. Install Docker : `sudo apt install docker.io`
3. Configure Docker to use the Pertimm registry :
    ```bash
    $ echo "{
        "registry-mirrors": [
          "http://docker-mirror.pertimm.corp:50000"
        ]
    }" > /etc/docker/daemon.json
    ```
4. Install PostgresSQL : `sudo apt install postgresql postgresql-contrib libpq-dev`
5. Add a viky user to PostgresSQL :
    ```bash
    $ sudo -i -u postgres
    $ createuser --interactive
    Enter name of role to add : viky
    Shall the new role be a superuser? (y/n) y
    $ psql
    postgres=# ALTER USER "viky" WITH PASSWORD 'viky';
    postgres=# \q
    $ exit
    ```
6. Install Redis : `sudo apt install redis-server`
7. Install NodeJS : `sudo apt install nodejs`
8. Install Yarn : follow instructions [here](https://yarnpkg.com/lang/en/docs/install/)
9. Install Rake & Bundler : `gem install rake bundler`
10. Create a new directory to put the viky project :
    ```bash
    $ mkdir ./viky
    $ cd viky
    $ git clone git@gitlab.pertimm.net:viky.ai/platform.git
    ```
11. Install every dependencies :
    ```bash
    $ cd platform/webapp
    $ bundler install # Ruby dependencies
    $ yarn install # JavaScript/CoffeScript dependencies
    ```
12. Configure your `.env` file :
    ```bash
    $ echo "MY_CURRENT_GIT_BRANCH=:`git describe --all --abbrev=0 --always --contains | sed 's|[~^].*||' | sed 's|remotes/origin/||' | sed 's|heads/||' | sed 's|tags/||' | sed 's|/|-|g' | sed 's|_|-|g'`" >> .env
    ```
13. Create databases : `$ ./bin/rails db:setup`
14. Start statistics containers (dev + tests) :
    ```bash
    $ docker run -p 9200:9200 -v $(pwd)/tmp:/backup_data -e "path.repo=/backup_data" -e "discovery.type=single-node" -e "node.name=viky-stats01-dev"  --rm --mount 'type=volume,src=vikyapp_stats01_dev,dst=/usr/share/elasticsearch/data'  --name viky-stats01-dev  docker.elastic.co/elasticsearch/elasticsearch:6.6.1
    $ docker run -p 9222:9200                                                        -e "discovery.type=single-node" -e "node.name=viky-stats01-test" --rm --mount 'type=volume,src=vikyapp_stats01_test,dst=/usr/share/elasticsearch/data' --name viky-stats01-test docker.elastic.co/elasticsearch/elasticsearch:6.6.1
    $ docker run -p 5601:5601 --link viky-stats01-dev  -e "SERVER_BASEPATH=/kibana" -e "SERVER_REWRITEBASEPATH=true" -e "ELASTICSEARCH_URL=http://viky-stats01-dev:9200" --rm --name viky-kibana docker.elastic.co/kibana/kibana:6.6.1
    ```
15. Setup statistics : `$ ./bin/rails statistics:setup`
16. Stop both statistics containers and the kibana container : `$ docker stop viky-stats01-dev viky-stats01-test viky-kibana`
17. Start Foreman : `$ foreman start`
18. Invite you as admin : `$ ./bin/rails users:invite_admin[<your@email.com>]`
19. System tests require chromedriver.
    a. Download its binary from (https://sites.google.com/a/chromium.org/chromedriver/downloads).
    b. Unzip the file.
    c. Move the file to `/usr/lib/chromium-browser/`

_Troubleshooting_ : sometimes Foreman fails on the Cerebro service. It is because we setup a Docker link between the dev stats container and the Cerebro container. This container may start too fast before the statistics container. Thus the link cannot be established and the Cerebro container fails. You have two solutions : either you can restart Foreman and hope to have some luck with the timing or you can increase the `sleep` delay in your `Procfile`.

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
VIKYAPP_REDIS_PACKAGE_NOTIFIER='redis://localhost:6379/3'

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

If you want to disable Postmark, use `SMTP_ENABLED=true` and the following environment variables.

* `SMTP_ADDRESS`: Allows you to use a remote mail server. Just change it from its default "localhost" setting.
* `SMTP_PORT`: On the off chance that your mail server doesn't run on port 25, you can change it.
* `SMTP_USER_NAME`: If your mail server requires authentication, set the username in this setting.
* `SMTP_PASSWORD`: If your mail server requires authentication, set the password in this setting.
* `SMTP_AUTHENTICATION`: If your mail server requires authentication, you need to specify the authentication type here. `plain` (will send the password in the clear), `login` (will send password Base64 encoded) or `cram_md5` (combines a Challenge/Response mechanism to exchange information and a cryptographic Message Digest 5 algorithm to hash important information)
* `SMTP_ENABLE_STARTTLS_AUTO`: Detects if STARTTLS is enabled in your SMTP server and starts to use it. Defaults to `true`.

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
$ sudo -i -u postgres
## Become postgres user
$ createuser --interactive
Enter name of role to add : viky
Shall the new role be a superuser? (y/n) y
$ psql
postgres=# ALTER USER "<user>" WITH PASSWORD '<password>';
postgres=# \q
$ exit
```

### Create database

```
$ ./bin/rails db:setup
> Created database 'vikylapp_development'
> Created database 'vikylapp_test'
> ...
```

## Bootstrap statistics

We use ElasticSearch to store statistics data. It is highly configured for a _time-series_ load.

Every indexes (_active_ and _inactives_) are searchable (ie: they all belongs to alias `search-stats-interpret_request_log`).
Only the _active_ index can receive new documents (ie: it is the only one belonging to alias `index-stats-interpret_request_log`).
This has two consequences :
  - the _active_ index is optimized for writing
  - _inactives_ indexes are optimized for reading

### Create templates, index and Kibana configuration

- It will create _templates_ only if it does not already exists.
- It will create _index_ only if it does not already exists.
- It always configure Kibana.
```
$ ./bin/rails statistics:setup
> Environment test.
> ...
> Environment development.
> ...
> Configure Kibana.
> ...
```

### Reindex a specific index

```bash
$ ./bin/rails statistics:reindex[<index_name>]
> ...
```

### Reindex every index

```bash
$ ./bin/rails statistics:reindex:all
> ...
```

### Rollover the active index

_Do something only if there is more than 100 000 documents in the index or it is older than 7 days._

Basically it will move the active index in the inactive pool, and put a new empty active index.

```bash
$ ./bin/rails statistics:rollover
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

### Cerebro

[Cerebro](https://github.com/lmenezes/cerebro) is an open source ElasticSearch web admin tool. In a dev environment it is accessible at `http://localhost:9201`.
