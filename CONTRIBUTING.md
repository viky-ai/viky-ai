# Contributing to viky.ai

This guide covers ways in which you can become a part of the ongoing development of viky.ai.

As mentioned in [README](README.md), everyone interacting in viky.ai codebases, issue trackers, chat rooms, and mailing lists is expected to follow this [code of conduct](CODE_OF_CONDUCT.md).

## Reporting an Issue

viky.ai uses [GitHub Issue Tracking](https://github.com/viky-ai/viky-ai/issues) to track issues. If you've found a bug, this is the place to start. You'll need to create a GitHub account in order to submit an issue, to comment on them, or to create pull requests.

If you've found a problem in viky.ai do a search on GitHub under [Issues](https://github.com/viky-ai/viky-ai/issues) in case it has already been reported. If you are unable to find any open issues addressing the problem you found, your next step will be to [open a new one](https://github.com/viky-ai/viky-ai/issues/new).

### Reporting bugs

Well-written bug reports are incredibly helpful. For that reason, your issue report should contain a title and a clear description of the issue. You should include as much relevant information as possible, including a clear description of the observed behaviour, a set of instructions for replicating it and the expected behaviour. Your objective is to facilitate the reproduction of the bug, to you and others, in order to find a solution.

### Reporting user interface bugs and features

If your bug or feature request touches on user interface, there are a few additional guidelines to follow:

* Include screenshots which are the visual equivalent of a minimal testcase.
* Make sure to include URLs, code snippets, and step-by-step instructions on how to reproduce the behavior visible in the screenshots.
* Indicate the operating system and web browser used.

### Requesting features

We're always trying to make viky.ai better, and your feature requests are a key part of that. Here are some tips on how to make a request:

* Please don't put "feature request" items into GitHub Issues, request the feature on the viky.ai-developers list. It's the proper place to discuss new ideas.
* Describe clearly and concisely what the missing feature is and how you'd like to see it implemented. Include example code (non-functional is OK) and/or UI mockups if possible.
* If it isn’t obvious, explain why you’d like the feature.

As with most open-source projects, code talks. If you are willing to write the code for the feature yourself it’s much more likely to be accepted. Fork viky.ai on GitHub, create a feature branch, and show us your work with a merge request!

## Helping to resolve existing issues

As a next step beyond reporting issues, you can help us to resolve existing ones by providing feedback about them.

* Can you reproduce the reported issue on your own computer? If so, you can add a comment to the issue saying that you're seeing the same thing.
* If an issue is vague, can you help us reduce it to something more specific? Perhaps you can provide additional information to help reproduce a bug.

## Contributing to the viky.ai documentation

You can help improve the viky.ai documentation by making it more coherent, consistent, or readable, adding missing information, correcting factual errors, fixing typos.

To do so, make changes to (viky.ai documentation source files)[https://github.com/viky-ai/doc]. Then open a pull request to apply your changes to the development branch.

## Contributing to the viky.ai webapp code

We will guide you through the contribution of a patch to viky.ai for the first time. The process is as follows:

1. Getting a copy of viky.ai core.
2. Running locally viky.ai platform.
3. Running locally viky.ai webapp tests suite.
4. Writing a test for your patch.
5. Writing the code for your patch.
6. Testing your patch.
7. Submitting a pull request.

### Getting a copy of viky.ai core

The first step to contributing to viky.ai is to get a copy of the source code. First, [fork viky.ai on GitHub](https://github.com/viky.ai/viky.ai/fork). Then, from the command line, navigate to the directory where you’ll want your local copy of viky.ai to live and use the following command:

    $ git clone https://github.com/YourGitHubName/viky-ai.git

### Install locally viky.ai platform

Install requirements describe in (REAME)[README.md].

Setup the application using the following command:

    $ cd webapp/
    $ ./bin/setup

This setup script will perform the following tasks to run the webapp component in development mode:

* Installation of ruby gems (`bundle install`).
* Installation of JavaScript yarn packages (`yarn install`).
* Initialization of tables in the PostgreSQL database (`./bin/rails db:setup`).
* Initialization of the Elastic indexes (`./bin/rails statistics:setup`).
* Initialization of an.env file to define the minimum necessary environment variables.

Once the execution is finished, you can start the platform in development mode with the following command:

    $ foreman start

The `foreman` command allows you to launch several commands defined in the `Procfile` file simultaneously. We use "docker-compose" to launch PostGres, Redis, Elastic, kibana and NLP services. Another command runs sidekiq, used to execute tasks asynchronously in the webapp. Finally, the rails application is also started.

The application is now available at the following address: http://localhost:3000/

### Running locally viky.ai webapp tests suite

When you contribute to the viky.ai webapp component, it is very important that your code changes do not introduce bugs in other parts of the application. One way to check that the viky.ai webapp component still works after making your changes is to run its test suite. If you have never run the webapp test suite before, it is a good idea to run it once before to familiarize yourself with its result.

The webapp component being a Ruby On Rails application, you must execute the following commands to run the tests:

    ./bin/rails test
    ./bin/rails test:system

### Writing a test for your patch

TODO

### Writing the code for your patch

TODO

### Testing your patch

TODO

### Submitting a pull request

TODO
