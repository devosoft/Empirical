# Pull base image.
FROM ubuntu:bionic-20210416

COPY . /opt/Empirical

SHELL ["/bin/bash", "-c"]

# Prevent interactive time zone config.
# adapted from https://askubuntu.com/a/1013396
ENV DEBIAN_FRONTEND=noninteractive

RUN \
  echo 'Acquire::http::Timeout "60";' >> "/etc/apt/apt.conf.d/99timeout" \
    && \
  echo 'Acquire::ftp::Timeout "60";' >> "/etc/apt/apt.conf.d/99timeout" \
    && \
  echo 'Acquire::Retries "100";' >> "/etc/apt/apt.conf.d/99timeout" \
    && \
  echo "buffed apt-get resiliency"

# Install apt packages
# xvfb nonsense adapted from https://github.com/samgiles/docker-xvfb
# remove -backports, -updates, -proposed, -security repositories
# looks like we have to grab libxxhash0 from -updates now
RUN \
  apt-get update -y \
    && \
  apt-get install --no-install-recommends libxxhash0 \
    && \
  apt-get clean \
    && \
  rm -rf /var/lib/apt/lists/* \
    && \
  find /etc/apt -type f -name '*.list' -exec sed -i 's/\(^deb.*-backports.*\)/#\1/; s/\(^deb.*-updates.*\)/#\1/; s/\(^deb.*-proposed.*\)/#\1/; s/\(^deb.*-security.*\)/#\1/' {} + \
    && \
  apt-get update -y \
    && \
  apt-get install -y software-properties-common=0.96.24.32.1 \
    && \
  add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && \
  apt-get update -y \
    && \
  apt-get install --no-install-recommends --allow-downgrades -y \
    dpkg-dev \
    libc6=2.27-3ubuntu1 \
    libc6-dev \
    libc6-dbg \
    build-essential \
    xvfb \
    x11vnc \
    x11-xkb-utils \
    xfonts-100dpi \
    xfonts-75dpi \
    xfonts-scalable \
    xfonts-cyrillic \
    x11-apps \
    gtk2-engines-pixbuf \
    firefox \
    libnss3 \
    lsb-release \
    xdg-utils \
    g++-8=8-20180414-1ubuntu2 \
    gcc-8-base=8-20180414-1ubuntu2 \
    cpp-8=8-20180414-1ubuntu2 \
    gcc-8=8-20180414-1ubuntu2 \
    gcc-8-base=8-20180414-1ubuntu2 \
    libgcc-8-dev \
    libstdc++-8-dev \
    cmake \
    python-virtualenv \
    python-pip-whl \
    python-pip \
    python-setuptools \
    python3-setuptools \
    python3-virtualenv \
    python3-pip \
    nodejs \
    npm \
    tar \
    gzip \
    libpthread-stubs0-dev \
    gdb \
    doxygen \
    curl \
    perl \
    perl-base=5.26.1-6 \
    git \
    htop \
    man \
    unzip \
    vim-common \
    vim-runtime \
    vim \
    nano \
    wget \
    ssh-client \
    libasound2 \
    gpg-agent \
    && \
  apt-get clean \
    && \
  rm -rf /var/lib/apt/lists/* \
    && \
  echo "installed apt packages"

RUN \
  echo $' \n\
XVFB=/usr/bin/Xvfb \n\
XVFBARGS="$DISPLAY -ac -screen 0 1024x768x16 +extension RANDR" \n\
PIDFILE=/var/xvfb_${DISPLAY:1}.pid \n\
case "$1" in \n\
  start) \n\
    echo -n "Starting virtual X frame buffer: Xvfb" \n\
    /sbin/start-stop-daemon --start --quiet --pidfile $PIDFILE --make-pidfile --background --exec $XVFB -- $XVFBARGS \n\
    echo "." \n\
    ;; \n\
  stop) \n\
    echo -n "Stopping virtual X frame buffer: Xvfb" \n\
    /sbin/start-stop-daemon --stop --quiet --pidfile $PIDFILE \n\
    echo "." \n\
    ;; \n\
  restart) \n\
    $0 stop \n\
    $0 start \n\
    ;; \n\
  *) \n\
  echo "Usage: /etc/init.d/xvfb {start|stop|restart}" \n\
  exit 1 \n\
esac \n\
exit 0 \n\
' > /etc/init.d/xvfb \
  && \
  echo "write xvfb init.d script"

RUN \
  chmod a+x /etc/init.d/xvfb \
    && \
  echo "make xvfb init.d script executable"

RUN \
  echo $'#!/bin/bash\n\
export DISPLAY=:99\n\
/etc/init.d/xvfb start\n\
sleep 1\n\
$@\n\
exit_value=$?\n\
/etc/init.d/xvfb stop\n\
exit $exit_value\n\
' > /usr/bin/xvfb-daemon-run \
  && \
  echo "write xvfb daemon script"

RUN \
  chmod a+x /usr/bin/xvfb-daemon-run \
    && \
  echo "make xvfb daemon script executable"

ENV DISPLAY :99

# magic from https://github.com/puppeteer/puppeteer/issues/3451#issuecomment-523961368
RUN echo 'kernel.unprivileged_userns_clone=1' > /etc/sysctl.d/userns.conf

RUN \
  update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 90 \
  && \
  npm install -g n \
  && \
  n 14.17 \
  && \
  export python="/usr/bin/python3" \
  && \
  npm install source-map \
  && \
  echo "finalized set up dependency versions"

RUN \
  pip install wheel==0.30.0 \
    && \
  pip3 install wheel==0.30.0 \
    && \
  pip3 install -r /opt/Empirical/doc/requirements.txt \
    && \
  echo "installed documentation build requirements"

RUN \
  cd /opt/Empirical \
    && \
  git submodule deinit -f . \
    && \
  git submodule init \
    && \
  git submodule update -f \
    && \
  echo "initialized submodules"

RUN \
  cd /opt/Empirical \
    && \
  make install-test-dependencies \
    && \
  echo "installed test dependencies"

RUN \
  cd /opt/Empirical \
    && \
  git remote set-url origin https://github.com/devosoft/Empirical.git \
    && \
  echo "switched to https origin remote url"

RUN \
  /etc/init.d/xvfb start \
   && \
  make test-web-ConfigPanel -C /opt/Empirical/tests/web \
    && \
  make test-native-ConfigPanel -C /opt/Empirical/tests/web \
    && \
  make test-vector -C /opt/Empirical/tests/base \
    && \
  make clean -C /opt/Empirical/tests \
    && \
  echo "representative tests passed!"

# Define default working directory.
WORKDIR /opt/Empirical


RUN \
  echo $'#!/bin/bash\n\
set -e\n\
/etc/init.d/xvfb start\n\
exec "$@"\n\
' > /opt/entrypoint.sh \
    && \
  echo "write entrypoint script"

RUN \
  chmod a+x /opt/entrypoint.sh \
    && \
  echo "make entrypoint script executable"

# Adapted from https://github.com/karma-runner/karma-firefox-launcher/issues/93#issuecomment-519333245
# Maybe important for container compatability running on Windows?
RUN \
  cd /opt/ \
  && \
  npm install -g yarn \
  && \
  git clone https://github.com/karma-runner/karma-firefox-launcher.git \
  && \
  cd karma-firefox-launcher \
  && \
  yarn install \
  && \
  echo "installed karma-firefox-launcher"

# @mmore500 10-2021: python3 -m pip fixes UnicodeDecodeError
# when installing charset-normalizer from github
RUN \
  python3 -m pip install -r /opt/Empirical/third-party/requirements.txt \
    && \
  echo "installed documentation build requirements"

# Perform any further action as an unprivileged user.
# adapted from https://stackoverflow.com/a/27703359
# and https://superuser.com/a/235398
RUN \
  useradd --create-home --shell /bin/bash user \
    && \
  groupadd group \
    && \
  gpasswd -a user group \
    && \
  chgrp --recursive user /opt \
    && \
  chmod --recursive g+rwx /opt \
    && \
  echo "user added and granted permissions to /opt"

USER user

# Define default entrypoint.
ENTRYPOINT ["/opt/entrypoint.sh"]

CMD ["bash"]
