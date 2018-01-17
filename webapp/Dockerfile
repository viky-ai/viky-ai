# Using docker multi stage (docker >= 17.05)
# https://codefresh.io/blog/node_docker_multistage/

# docker build --target image_ruby_run  -t docker-registry.pertimm.net/viky.ai/platform/app .
# docker build --target image_ruby_test -t docker-registry.pertimm.net/viky.ai/platform/app_test .

ARG VIKY_IMAGE_TAG=latest

#################################################
# image_ruby_deps : tools_ruby_base + app dependencies
#################################################

# copy static file from doc
FROM docker-registry.pertimm.net/viky.ai/platform/doc:${VIKY_IMAGE_TAG} AS viki_doc

FROM docker-registry.pertimm.net/viky.ai/platform/tools_ruby_base:${VIKY_IMAGE_TAG} AS image_ruby_run

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

# Production dependencies
RUN bundle install --without development test --deployment --jobs=$(nproc)

# Copy application code
COPY . /webapp

# Use a dummy SECRET_KEY_BASE
ENV SECRET_KEY_BASE=dummy
ENV RAILS_ENV=production

# Precompile assets
RUN ./bin/rails assets:precompile

# create temporary dir for sidekiq
RUN mkdir -p /webapp/tmp/
RUN mkdir -p /webapp/tmp/pids/
RUN mkdir -p /webapp/log/

# copy doc
COPY --from=viki_doc /doc /webapp/public/doc

ENV RAILS_SERVE_STATIC_FILES=true
ENV RAILS_LOG_TO_STDOUT=1

EXPOSE 3000
CMD ["./bin/docker_run.sh"]

ENV VIKYAPP_WORKER=false
HEALTHCHECK --interval=30s --timeout=3s --start-period=600s \
  CMD /webapp/bin/docker_healthcheck.sh

#################################################
# image_ruby_test : image_ruby_base_test + test dependencies
#################################################
FROM docker-registry.pertimm.net/viky.ai/platform/tools_ruby_base_test:${VIKY_IMAGE_TAG} AS image_ruby_test

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

# Install test dependencies
RUN bundle install --jobs=$(nproc)

# Copy application code
COPY . /webapp

ENV RAILS_ENV=test

# Run test
CMD ["./bin/docker_run_test.sh"]
