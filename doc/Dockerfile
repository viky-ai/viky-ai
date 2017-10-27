# Using docker multi stage (docker >= 17.05)
# https://codefresh.io/blog/node_docker_multistage/

# docker build --target image_doc_run -t docker-registry.pertimm.net/voqal.ai/platform/doc .

ARG RUBY_BASE_VERSION=latest

#################################################
# image_doc_build : image_ruby_deps  + build doc
#################################################
FROM docker-registry.pertimm.net/voqal.ai/platform/tools_ruby_base:${RUBY_BASE_VERSION} AS image_doc_build

# Needed system dependencies for Slate gem compilation
RUN set -ex; \
    apt-get update; \
    apt-get install -y --no-install-recommends \
      libgmp-dev \
    ; \
    rm -rf /var/lib/apt/lists/*

WORKDIR /

# Firstly bundle install (faster build)
RUN mkdir -p /src
COPY Gemfile /src
COPY Gemfile.lock /src

WORKDIR /src

# Production dependencies
RUN bundle install --without development test --deployment --jobs=$(nproc)

# Copy application code
RUN mkdir -p /src
COPY . /src

# Generate static site
RUN bundle exec middleman build --clean --build-dir=/doc

#################################################
# image_doc_run : build doc +
#################################################
FROM nginx:stable-alpine AS image_doc_run

COPY nginx_default.conf /etc/nginx/conf.d/default.conf

# copy doc to nginx web server
COPY --from=image_doc_build /doc /doc
