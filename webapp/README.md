# README

## Dependencies

* Ruby (2.3.1)
* bundler (1.15.4)
* NodeJS (8.4.0)
* Yarn (1.0.1)

## Environment Variables

The application makes extensive use of environment variable to keep the configuration flexible across every development environment.
You can define your local variables in a `.env` file and place it in the root of the webapp project,
they will be loaded into the environment at the application bootstrap and available within your code as usual, via `ENV['EXAMPLE_VAR']`.

Beware of never committing or pushing the `.env` file, it is just useful for your local webapp instance to match your local configuration.
For example you could define your local PostgreSQL user and password variables therein, e.g.:

```
VOQALAPP_DB_USERNAME=superman
VOQALAPP_DB_PASSWORD=superman
```

those variables will be used by the application database connector.

## Preparing the Database

Once you've installed PostgreSQL in your local machine and defined a user role with enough access rights, you can do:

```
rails db:setup
```
