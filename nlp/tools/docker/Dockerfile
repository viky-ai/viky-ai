#########################################################################################
# Minimal dependencies to build NLP from strach
# docker build --pull -t docker-registry.pertimm.net/viky.ai/platform/tools_nlp_base .
#########################################################################################
FROM ubuntu:xenial

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
             git-core \
             libdata-dumper-simple-perl \
             libdatetime-locale-perl \
             libdigest-perl-md5-perl \
             libexpat1-dev \
             libffi-dev \
             liblocal-lib-perl \
             libmysqlclient-dev \
             libncurses5-dev \
             libpath-class-perl \
             libreadline-dev \
             libssl-dev \
             libsub-exporter-perl \
             libtool \
             libxml-dom-perl \
             libxml-xql-perl  \
             libxml2-dev \
             libxml2-utils \
             libxslt1-dev \
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

# Install rbenv : used to build morphology dico
ENV RBENV_VERSION 2.5.6
ENV RBENV_ROOT /usr/local/rbenv
ENV PATH "${RBENV_ROOT}/bin:${RBENV_ROOT}/shims:${PATH}"
RUN set -ex; \
    git clone https://github.com/sstephenson/rbenv.git /usr/local/rbenv ; \
    echo '# rbenv setup' > /etc/profile.d/rbenv.sh ; \
    echo 'export RBENV_ROOT=/usr/local/rbenv' >> /etc/profile.d/rbenv.sh ; \
    echo 'export PATH="$RBENV_ROOT/bin:$PATH"' >> /etc/profile.d/rbenv.sh ; \
    echo 'eval "$(rbenv init -)"' >> /etc/profile.d/rbenv.sh ; \
    chmod +x /etc/profile.d/rbenv.sh ; \
    mkdir /usr/local/rbenv/plugins  ; \
    git clone https://github.com/sstephenson/ruby-build.git /usr/local/rbenv/plugins/ruby-build ; \
    rbenv install ${RBENV_VERSION}


# install mesonbuild for glib build
RUN pip3 --no-cache-dir install meson

ENV LC_ALL   C.UTF-8
ENV LANG     en_US.UTF-8
ENV LANGUAGE en_US.UTF-8

RUN mkdir /builds
WORKDIR /builds

