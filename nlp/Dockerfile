# Using docker multi stage (docker >= 17.05)
# https://codefresh.io/blog/node_docker_multistage/

# Run
# docker build --target run_image -t docker-registry.pertimm.net/viky.ai/platform/nlp .
# docker run -it --volume "$(pwd)/import:/nls/import" -p "9345:9345" -t docker-registry.pertimm.net/viky.ai/platform/nlp

# Tests
# docker build --target test_image -t docker-registry.pertimm.net/viky.ai/platform/nlp_test .
# docker run -t docker-registry.pertimm.net/viky.ai/platform/nlp_test

# ============================================================================
# Compile and package Nls
# ============================================================================
FROM docker-registry.pertimm.net/tools/docker-ebusiness-build-stack AS build_image

ENV OG_REPO_PATH=/builds/nls
ENV OG_TOOL_PATH=${OG_REPO_PATH}/tools
ENV PATH=${OG_TOOL_PATH}/bin:${PATH}

# Optimise : prebuild externale dependencies
ADD products/*.dtd                  /builds/nls/products/
ADD resources/manifest.xml          /builds/nls/resources/manifest.xml
ADD sources/manifest.xml            /builds/nls/sources/manifest.xml
ADD sources/makefile.defs.linux     /builds/nls/sources/makefile.defs.linux
ADD sources/external_dependencies   /builds/nls/sources/external_dependencies
ADD tools                           /builds/nls/tools

RUN buildinit.pl
RUN factory.pl -c external_dependencies fullclean
RUN factory.pl -c external_dependencies rebuild

ADD resources/morphology            /builds/nls/resources/morphology
ADD products/bu2_nls/bin            /builds/nls/products/bu2_nls/bin
ADD products/bu2_nls/conf           /builds/nls/products/bu2_nls/conf
ADD products/bu2_nls/ling           /builds/nls/products/bu2_nls/ling
ADD products/bu2_nls/node_modules   /builds/nls/products/bu2_nls/node_modules
ADD products/bu2_nls/manifest.xml   /builds/nls/products/bu2_nls/manifest.xml
ADD products/bu2_nls/LICENSE.TXT    /builds/nls/products/bu2_nls/LICENSE.TXT
ADD sources                         /builds/nls/sources

RUN genmake.pl linux64 all
RUN factory.pl -p bu2_nls rebuild
RUN factory.pl -s bu2_nls   build

# ============================================================================
# Setup a lightweigth image only with minimal binaries
# ============================================================================
FROM ruby:2.4-slim-stretch AS run_image

# Add libxml2 needed for XSD validation
# Add curl needed for healthcheck
RUN set -ex; \
    apt-get update ; \
    apt-get install -y --no-install-recommends \
      build-essential \
      ca-certificates \
      curl \
      g++ \
      libgmp-dev \
      libxml2 \
      wget \
    ; \
    rm -rf /var/lib/apt/lists/*

# dockerize : https://github.com/jwilder/dockerize
ENV DOCKERIZE_VERSION v0.6.0
RUN wget https://github.com/jwilder/dockerize/releases/download/$DOCKERIZE_VERSION/dockerize-linux-amd64-$DOCKERIZE_VERSION.tar.gz \
    && tar -C /usr/local/bin -xzvf dockerize-linux-amd64-$DOCKERIZE_VERSION.tar.gz \
    && rm dockerize-linux-amd64-$DOCKERIZE_VERSION.tar.gz

WORKDIR /

# Firstly bundle install (faster build)
COPY ./nlp_route/Gemfile      /nlp_route/
COPY ./nlp_route/Gemfile.lock /nlp_route/

WORKDIR /nlp_route/

# Production dependencies
RUN bundle install --without development test --deployment --jobs=$(nproc)

# Copy application code
COPY ./nlp_route/ /nlp_route/

COPY --from=build_image /builds/nls/ship /
RUN ln -sfn `readlink -f /nls-* | head -n 1` /nls

WORKDIR /nls
ENV NLS_INSTALL_PATH=/nls

EXPOSE 9345
CMD ["./docker_run.sh"]

# Healthcheck with Docker File
#HEALTHCHECK --interval=30s --timeout=3s --start-period=600s \
#  CMD curl -f http://localhost:9345/list/ || exit 1

# ============================================================================
# Test Nls
# ============================================================================
FROM ruby:2.4-stretch AS test_image

ADD ./docker_run_test.sh /

# NLP
WORKDIR /nlp_tests

# Firstly bundle install (faster build)
ADD products/bu2_nls/tests/Gemfile      /nlp_tests
ADD products/bu2_nls/tests/Gemfile.lock /nlp_tests

RUN bundle install --jobs $(nproc)

# NLP_ROUTE
WORKDIR /nlp_route_tests

# Firstly bundle install (faster build)
ADD ./nlp_route/Gemfile      /nlp_route_tests/
ADD ./nlp_route/Gemfile.lock /nlp_route_tests/

# Production dependencies
RUN bundle install --jobs=$(nproc)

# Copy nlp_route tests code
ADD ./nlp_route/ /nlp_route_tests/

# Copy test code
ADD products/bu2_nls/tests  /nlp_tests

COPY --from=build_image /builds/nls/ship /
RUN ln -sfn `readlink -f /nls-* | head -n 1` /nls

ENV NLS_INSTALL_PATH=/nls
ENV TERM=xterm-256color

# Run test
CMD ["/docker_run_test.sh"]
