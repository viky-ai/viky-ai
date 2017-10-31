# Using docker multi stage (docker >= 17.05)
# https://codefresh.io/blog/node_docker_multistage/

# Run
# docker build --target run_image -t docker-registry.pertimm.net/voqal.ai/platform/nlp .
# docker run -it --volume "$(pwd)/import:/nls/import" -p "9345:9345" -t docker-registry.pertimm.net/voqal.ai/platform/nlp

# Tests
# docker build --target test_image -t docker-registry.pertimm.net/voqal.ai/platform/nlp_test .
# docker run -t docker-registry.pertimm.net/voqal.ai/platform/nlp_test

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
RUN factory.pl -c external_dependencies redebug

ADD products/bu2_nls/conf           /builds/nls/products/bu2_nls/conf
ADD products/bu2_nls/manifest.xml   /builds/nls/products/bu2_nls/manifest.xml
ADD products/bu2_nls/LICENSE.TXT    /builds/nls/products/bu2_nls/LICENSE.TXT
ADD sources                         /builds/nls/sources

RUN genmake.pl linux64 all
RUN factory.pl -p bu2_nls redebug
RUN factory.pl -s bu2_nls   debug

# ============================================================================
# Setup a lightweigth image only with minimal binaries
# ============================================================================
FROM ubuntu:xenial AS run_image

WORKDIR /

COPY --from=build_image /builds/nls/ship /
RUN ln -sfn `readlink -f /nls-* | head -n 1` /nls

WORKDIR /nls

VOLUME [ "/nls/import" ]
EXPOSE 9345
CMD ["./ogm_nls"]

# ============================================================================
# Test Nls
# ============================================================================
FROM ruby:2.4.2 AS test_image

WORKDIR /tests

# Firstly bundle install (faster build)
ADD products/bu2_nls/tests/Gemfile      /tests
ADD products/bu2_nls/tests/Gemfile.lock /tests

RUN bundle install --jobs $(nproc)

# Copy test code
ADD products/bu2_nls/tests  /tests

COPY --from=build_image /builds/nls/ship /
RUN ln -sfn `readlink -f /nls-* | head -n 1` /nls

ENV NLS_INSTALL_PATH=/nls
ENV TERM=xterm-256color

# Run test
CMD ["bundle", "exec", "rake", "test"]

