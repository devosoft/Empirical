# Pull base image.
FROM ubuntu:focal-20230412

COPY . /opt/Empirical

SHELL ["/bin/bash", "-c"]

# Prevent interactive time zone config.
# adapted from https://askubuntu.com/a/1013396
ENV DEBIAN_FRONTEND=noninteractive
ENV SPHINXBUILD="python3.10 -m sphinx"

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
  apt-get update -y \
    && \
  apt-get install -y software-properties-common \
    && \
  add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && \
  add-apt-repository -y ppa:deadsnakes/ppa \
    && \
  apt-get update -y \
    && \
  apt-get install --no-install-recommends --allow-downgrades -y \
    build-essential \
    dpkg-dev \
    g++-11 \
    libc6 \
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
    cmake \
    python3-distutils \
    python3-setuptools \
    python3-virtualenv \
    python3-pip \
    'python3\.10' \
    'python3\.10-distutils' \
    'python3\.10-venv' \
    nodejs \
    npm \
    tar \
    gzip \
    libpthread-stubs0-dev \
    gdb \
    doxygen \
    curl \
    perl \
    perl-base \
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
  update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 90 \
  && \
  npm install -g n \
  && \
  n 14.17 \
  && \
  hash -r \
  && \
  export python="/usr/bin/python3" \
  && \
  npm install source-map \
  && \
  echo "finalized set up dependency versions"

RUN \
  curl -sS https://bootstrap.pypa.io/get-pip.py | python3 \
    && \
  curl -sS https://bootstrap.pypa.io/get-pip.py | python3.10 \
    && \
  pip install --upgrade --force-reinstall pip virtualenv \
    && \
  pip3 install --upgrade --force-reinstall pip virtualenv \
    && \
  python3.10 -m pip install --upgrade --force-reinstall pip virtualenv \
    && \
  python3 -m pip install --upgrade --force-reinstall pip virtualenv \
    && \
  pip install wheel==0.30.0 six==1.16.0 \
    && \
  pip3 install wheel==0.30.0 six==1.16.0 \
    && \
  python3.10 -m pip install wheel==0.30.0 six==1.16.0 \
    && \
  python3 -m pip install wheel==0.30.0 six==1.16.0 \
    && \
  pip3 install -r /opt/Empirical/doc/requirements.txt \
    && \
  python3.10 -m pip install -r /opt/Empirical/doc/requirements.txt \
    && \
  python3 -m pip install -r /opt/Empirical/doc/requirements.txt \
    && \
  echo "installed documentation build requirements"

RUN \
  cd /opt/Empirical \
    && \
  git submodule deinit -f . \
    && \
  git submodule init \
    && \
  echo "nameserver 8.8.8.8" > /etc/resolv.conf \
    && \
  n=0; until [ $n -ge 3 ]; do git submodule update -f && break || ((n++)); sleep 5; done; if [ $n -eq 3 ]; then echo "Update failed after 3 attempts."; else echo "Update successful!"; fi \
    && \
  echo "initialized submodules"

RUN \
  cd /opt/Empirical \
    && \
  curl -sS https://bootstrap.pypa.io/get-pip.py | python3 \
    && \
  python3 -m pip install virtualenv \
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
