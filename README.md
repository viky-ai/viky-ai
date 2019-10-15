# Welcome to viky.ai

## What's viky.ai

viky.ai is a Natural Language Processing platform. It allows you to extract information from unstructured text contents.

The technical component *nlp* allows the extraction of structured information, this extraction is defined within agents. The agents are multilingual assistants to find relevant data. The *nlp* component takes as input a set of agents in JSON format and unstructured textual content in order to provide a JSON stream of structured data as output.

The second technical component [webapp](webapp/README.md) is a web application that allows you to work collaboratively to set up agents by offering dedicated interfaces. It also provides the interpret API in order to allow integration into a third-party system.

## Getting Started

You can run viky.ai on Linux and macOS.

### Requirements

viky.ai local install used for development relies on the following dependencies.

* Docker Engine 19 and Compose 1.24
* Ruby 2.6+ and Bundler 2.0+
* NodeJS 10+ and Yarn 1.19+
* postgresql-client 11

### Setup and run

1. Setup the application using the following command:
   ```
   $ ./webapp/bin/setup
   ```

2. Start the application using the following commands:
   ```
   $ cd webapp/
   $ foreman start
   ```

The application is now available at the following address: http://localhost:3000/

## Contributing

We encourage you to contribute to viky.ai! Please check out the [Contributing to viky.ai guide](CONTRIBUTING.md).

Everyone interacting in viky.ai and its codebases, issue trackers, chat rooms, and mailing lists is expected to follow this [code of conduct](CODE_OF_CONDUCT.md).

## License

viky.ai is released under the [MIT License](LICENCE.txt).
