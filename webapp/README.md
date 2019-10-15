# Welcome to viky.ai webapp

`webapp` component is a Ruby On Rails application that allows you to work collaboratively to set up viky.ai agents by offering dedicated interfaces. It also provides the interpret API in order to allow integration into a third-party system.

## Install

Please refer to the instructions given in the [general readme of viky.ai](../README.md).

## Configuring

The application makes extensive use of environment variables to keep the configuration flexible across every environments. You can define your local variables in a `.env` file and place it in the root of the webapp project, they will be loaded into the environment at the application bootstrap and available within your code as usual, via `ENV['EXAMPLE_VAR']`.

Beware of never committing or pushing the `.env` file, it is just useful for your local webapp instance to match your local configuration.

### Required environment variables

The environment variables listed below are set up by the script `./bin/setup` in `.env` file.

* `MY_CURRENT_GIT_BRANCH` allows you to define the current git working branch in order to use a correct version of the associated `nlp` component.
* `VIKYAPP_INTERNAL_URL`: allows to define the URL of the webapp, this one being used by the NLP component.

### Enable mailing

When the mailing is activated, the following authentication-related features (provided by devise gem) are activated:

* _Lockable_ locks an account after 20 failed sign-in attempts. Can unlock via email or after a specified time period.
* _Recoverable_ resets the user password and sends reset instructions.
* _Confirmable_ sends emails with confirmation instructions and verifies whether an account is already confirmed during sign in.

The application also sends mails for certain operations such as transferring agents, adding, deleting or modifying user rights on an agent for example.

To activate the mailing and therefore the features described above you have two possibilities:

* Use [Postmark](postmarkapp.com) in order to send emails, for that define the environment variable `POSTMARK_TOKEN`.
* Use SMTP directly, for that set `SMTP_ENABLED` to `true` and define the following environment variables
  - `SMTP_ADDRESS`: Allows you to use a remote mail server. Just change it from its default "localhost" setting.
  - `SMTP_PORT`: On the off chance that your mail server doesn't run on port 25, you can change it.
  - `SMTP_USER_NAME`: If your mail server requires authentication, set the username in this setting.
  - `SMTP_PASSWORD`: If your mail server requires authentication, set the password in this setting.
  - `SMTP_AUTHENTICATION`: If your mail server requires authentication, you need to specify the authentication type here. `plain` (will send the password in the clear), `login` (will send password Base64 encoded) or `cram_md5` (combines a Challenge/Response mechanism to exchange information and a cryptographic Message Digest 5 algorithm to hash important information)
  - `SMTP_ENABLE_STARTTLS_AUTO`: Detects if STARTTLS is enabled in your SMTP server and starts to use it. Defaults to `true`.

Emails are sent with `MAILER_SENDER` variable value as sender.

### Enable quota feature

The quota feature can be activated using the environment variable `VIKYAPP_QUOTA_ENABLED`. This feature is disabled by default. When activated, quota can be disabled per user in order to allow full use the platform for a well-defined list of users.

The environment variable `VIKYAPP_QUOTA_EXPRESSION` defines the maximum number of formulations and entities that a user can create for all his agents.

Two other environment variables are used to  to limit the use of the API: a limitation of the number of requests for a given user per second (`VIKYAPP_QUOTA_INTERPRET_PER_SECOND`) and another per day (`VIKYAPP_QUOTA_INTERPRET_PER_DAY`). In case of quota overrun an HTTP code 429 is returned by the API.

Here is an example of a quota configuration:

    # If you want to enabled quota
    VIKYAPP_QUOTA_ENABLED=true

    # If you want to change the interpret endpoint requests limit
    VIKYAPP_QUOTA_INTERPRET_PER_DAY=400   # Max 400 requests to interpret endpoint per day
    VIKYAPP_QUOTA_INTERPRET_PER_SECOND=3  # Max 3 requests to interpret endpoint per second

    # If you want to change the expression limitation
    VIKYAPP_QUOTA_EXPRESSION=20000        # Limit the overall formulations and entities count to 20 000


## Defines a user as administrator

Administrator users can access to "Platform overview" and "Users management" UIs. In order to set a user as administrator, you can use the following Rails tasks:

```bash
./bin/rails users:set_admin[user_email]
```
