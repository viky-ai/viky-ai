# README

## Dependencies

* Ruby (2.4.2)
* bundler (1.15.4)
* NodeJS (8.4.0)
* Yarn (1.0.1)
* PostgreSQL (9.6.5)
* Redis (4.0.1)

## Environment Variables

The application makes extensive use of environment variables to keep the configuration flexible across every development environment.
You can define your local variables in a `.env` file and place it in the root of the webapp project,
they will be loaded into the environment at the application bootstrap and available within your code as usual, via `ENV['EXAMPLE_VAR']`.

Beware of never committing or pushing the `.env` file, it is just useful for your local webapp instance to match your local configuration.

For example you could define your local PostgreSQL username, password and other variables, and also declare your favourite Redis endpoint, maybe it's a remote machine, maybe your localhost, e.g.:

```
VOQALAPP_DB_USERNAME=superman
VOQALAPP_DB_PASSWORD='sup$_3rman'
VOQALAPP_DB_HOST=localhost
VOQALAPP_DB_PORT=5432

VOQALAPP_REDIS_URL='redis://localhost:6379/1'
```
