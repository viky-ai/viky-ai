# Using docker multi stage (docker >= 17.05)
# https://codefresh.io/blog/node_docker_multistage/

# docker build --pull --target image_ruby_run   -t vikyai/app .
# docker build        --target image_ruby_test  -t vikyai/app_test .

#####################################################################
# Include fluentbit in webapp to bufferize stats collection
#####################################################################
FROM fluent/fluent-bit:1.3 AS fluent-bit

#####################################################################
# image_ruby_base : ruby + sys dependencies
#####################################################################
FROM ruby:2.6-slim-buster AS image_ruby_base

# Same has buildpack-deps-buster-curl
RUN set -ex; \
    apt-get update ; \
    apt-get install -y --no-install-recommends \
      build-essential \
      ca-certificates \
      curl \
      cron \
      dirmngr \
      gnupg2 \
      nodejs \
      vim \
      wget \
      yarnpkg \
    ; \
    rm -rf /var/lib/apt/lists/* ; \
    ln -s /usr/bin/yarnpkg  /usr/local/bin/yarn

# postgresql-client
# Temporary fix for postgresql-client install fail
# https://github.com/dalibo/temboard/commit/ff98d6740ae11345658508b02052294d6cffd448
RUN set -ex; \
    mkdir -p /usr/share/man/man1; \
    mkdir -p /usr/share/man/man7;

# Needed system dependencies for gem compilation
RUN set -ex; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      file \
      graphicsmagick-imagemagick-compat \
      graphviz \
      libcurl4-openssl-dev \
      libfontconfig \
      libgmp-dev \
      libpq-dev \
      libyaml-dev \
      postgresql-client \
      rsync \
      zlib1g-dev \
    ; \
    rm -rf /var/lib/apt/lists/*

# dockerize : https://github.com/jwilder/dockerize
ENV DOCKERIZE_VERSION v0.6.1
RUN wget https://github.com/jwilder/dockerize/releases/download/$DOCKERIZE_VERSION/dockerize-linux-amd64-$DOCKERIZE_VERSION.tar.gz \
    && tar -C /usr/local/bin -xzvf dockerize-linux-amd64-$DOCKERIZE_VERSION.tar.gz \
    && rm dockerize-linux-amd64-$DOCKERIZE_VERSION.tar.gz

# install fluient-bit within the webapp
COPY --from=fluent-bit /fluent-bit /fluent-bit

WORKDIR /

#####################################################################
# image_ruby_common : image_ruby_base + app + app dependencies
#####################################################################
FROM image_ruby_base AS image_ruby_common

# App specific
RUN mkdir -p /webapp
WORKDIR /webapp

# Yarn install
COPY package.json /webapp
COPY yarn.lock /webapp
RUN yarn install

# setup env
ENV PATH="/webapp/bin:${PATH}"

# Firstly bundle install (faster build)
COPY Gemfile /webapp
COPY Gemfile.lock /webapp

# Get correct bundler version
RUN gem install bundler -v 2.1.4

# Production dependencies
RUN bundle config set without 'development test'
RUN bundle install --jobs=$(nproc)

# Copy application code
COPY . /webapp

# create static assets echange dir
RUN mkdir -p /tmp/public/

# create temporary dir for sidekiq
RUN mkdir -p /webapp/tmp/
RUN mkdir -p /webapp/tmp/pids/
RUN mkdir -p /webapp/log/

# Use a dummy SECRET_KEY_BASE
ENV SECRET_KEY_BASE=dummy

#####################################################################
# image_ruby_run : image_ruby_common + production setup
#####################################################################
FROM image_ruby_common AS image_ruby_run

ENV RAILS_ENV=production

# Precompile assets
RUN ./bin/rails assets:precompile

# Check shared
RUN set -ex; \
    mkdir -p ./public/uploads/ ; \
    touch ./public/uploads/.readable ; \
    mkdir -p ./public/packs/ ; \
    touch ./public/packs/.readable ; \
    mkdir -p ./public/assets/ ; \
    touch ./public/assets/.readable ;

ENV RAILS_SERVE_STATIC_FILES=true
ENV RAILS_LOG_TO_STDOUT=1

EXPOSE 3000
CMD ["./bin/docker_run.sh"]

#####################################################################
# image_ruby_test : image_ruby_common + test dependencies + test setup
#####################################################################
FROM image_ruby_common AS image_ruby_test

# Install test dependencies
RUN bundle config unset without && bundle config set without 'development'
RUN bundle install --jobs=$(nproc)

ENV RAILS_ENV=test

# Precompile assets
RUN ./bin/rails assets:precompile

# Run test
CMD ["./bin/docker_run_test.sh"]
