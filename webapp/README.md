# README

## Dependencies

* Ruby (2.3.1)
* bundler (1.15.4)
* NodeJS (8.4.0)
* Yarn (1.0.1)
* PostgreSQL (9.6.5)

## Environment Variables

The application makes extensive use of environment variables to keep the configuration flexible across every development environment.
You can define your local variables in a `.env` file and place it in the root of the webapp project,
they will be loaded into the environment at the application bootstrap and available within your code as usual, via `ENV['EXAMPLE_VAR']`.

Beware of never committing or pushing the `.env` file, it is just useful for your local webapp instance to match your local configuration.
For example you could define your local PostgreSQL username, password, host and port variables, maybe it's a remote machine, maybe not, e.g.:

```
VOQALAPP_DB_USERNAME=superman
VOQALAPP_DB_PASSWORD='sup$_3rman'
VOQALAPP_DB_HOST=localhost
VOQALAPP_DB_PORT=5432
```

those variables will be used by the application database connector.
