# Using docker multi stage (docker >= 17.05)
# https://codefresh.io/blog/node_docker_multistage/

# docker build --target image_ruby_run  -t docker-registry.pertimm.corp:50001/voqal.ai/platform/app .
# docker build --target image_ruby_test -t docker-registry.pertimm.corp:50001/voqal.ai/platform/app_test .

#################################################
# nodejs-base : tar.gz
#################################################

FROM node:8.6-stretch AS nodejs-base

# archive nodejs
RUN tar czPf /nodejs.tgz /usr/local/bin/ /usr/local/lib/ /opt/yarn/

#################################################
# image_ruby_base : ruby + app dependencies
#################################################
FROM ruby:2.4.2-slim-stretch AS image_ruby_base

# Same has buildpack-deps-stretch-curl
RUN set -ex; \
    apt-get update ; \
    apt-get install -y --no-install-recommends \
      build-essential \
      ca-certificates \
      curl \
      dirmngr \
      gnupg2 \
      wget \
      vim \
    ; \
    rm -rf /var/lib/apt/lists/*

# Needed system dependencies for gem compilation
RUN set -ex; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      file \
      graphicsmagick-imagemagick-compat \
      libcurl4-openssl-dev \
      libfontconfig \
      libpq-dev \
      libyaml-dev \
      zlib1g-dev \
    ; \
    rm -rf /var/lib/apt/lists/*

WORKDIR /

# Copy Node and Yarn install
COPY --from=nodejs-base /nodejs.tgz /
RUN tar xzPf /nodejs.tgz

ENV NPM_CONFIG_LOGLEVEL info
ENV NODE_VERSION 8.6.0
ENV YARN_VERSION 1.1.0

# App specific
RUN mkdir -p /webpapp
WORKDIR /webpapp

# Yarn install
COPY package.json /webpapp
COPY yarn.lock /webpapp
RUN yarn install

# setup env
ENV PATH="/webpapp/bin:${PATH}"

# Firstly bundle install (faster build)
COPY Gemfile /webpapp
COPY Gemfile.lock /webpapp

# Production dependencies
RUN bundle install --without development test --deployment --jobs=$(nproc)

#################################################
# image_ruby_run : image_ruby_base + code + assets
#################################################
FROM image_ruby_base AS image_ruby_run

# Copy application code
COPY . /webpapp

# Use a dummy SECRET_KEY_BASE
ENV SECRET_KEY_BASE=dummy

# Precompile assets
RUN ./bin/rails assets:precompile

# create temporary dir for sidekiq
RUN mkdir -p /webpapp/tmp/
RUN mkdir -p /webpapp/tmp/pids/
RUN mkdir -p /webpapp/log/

ENV RAILS_ENV=production
ENV RAILS_SERVE_STATIC_FILES=true
ENV RAILS_LOG_TO_STDOUT=1

EXPOSE 3000
CMD ["./bin/docker_run.sh"]

#################################################
# phantomjs-base
#################################################

FROM wernight/phantomjs AS phantomjs-base

#################################################
# image_ruby_test : image_ruby_base + test dependencies
#################################################
FROM image_ruby_base AS image_ruby_test

# copy phantomjs
COPY --from=phantomjs-base /usr/local/bin/phantomjs /usr/local/bin/phantomjs

# Install test dependencies
RUN bundle install --with test --deployment --jobs=$(nproc)

# Copy application code
COPY . /webpapp

ENV RAILS_ENV=test

# Run test
CMD ["./bin/docker_run_test.sh"]

# docker
