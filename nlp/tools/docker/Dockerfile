#########################################################################################
# Minimal dependencies to build NLP from strach
# docker build --pull -t docker-registry.pertimm.net/viky.ai/platform/tools_nlp_base .
#########################################################################################

# Ruby 2.5 is at least mdatory for building morphology
FROM ruby:2.6-slim-buster

# Install lib and needed tools
RUN  apt-get update \
  && apt-get install -y --no-install-recommends \
             apt-transport-https \
             autoconf \
             automake \
             build-essential \
             ca-certificates \
             cmake \
             colordiff \
             curl \
             diffutils \
             gettext \
             git \
             iproute2 \
             libdata-dumper-simple-perl \
             libdatetime-locale-perl \
             libdigest-perl-md5-perl \
             libexpat1-dev \
             libffi-dev \
             liblocal-lib-perl \
             libncurses5-dev \
             libpath-class-perl \
             libreadline-dev \
             libsub-exporter-perl \
             libtool \
             libxml-dom-perl \
             libxml-xql-perl  \
             libxml2-dev \
             libxml2-utils \
             libyaml-dev \
             ninja-build \
             perl \
             pkg-config \
             python-yaml \
             python3 \
             python3-pip \
             python3-setuptools \
             python3-wheel \
             software-properties-common \
             texinfo \
             unaccent \
             unzip \
             vim \
             zip \
             zlib1g-dev \
  && apt-get purge -y --auto-remove \
  && apt-get clean -y \
  && rm -rf /var/lib/apt/lists/*

# install mesonbuild for glib build
RUN pip3 --no-cache-dir install meson

ENV LC_ALL   C.UTF-8
ENV LANG     en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

RUN mkdir /builds
WORKDIR /builds

