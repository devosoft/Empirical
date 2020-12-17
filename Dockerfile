# Pull base image.
FROM ubuntu:18.04

COPY . /opt/Empirical

SHELL ["/bin/bash", "-c"]

# Install.
RUN \
  apt-get update -y \
    && \
  apt-get install -y software-properties-common \
    && \
  add-apt-repository -y ppa:ubuntu-toolchain-r/test \
    && \
  apt-get update -y \
    && \
  apt-get -y upgrade \
    && \
  echo "configured packaging system"

# xvfb nonsense adapted from https://github.com/samgiles/docker-xvfb
RUN \
  apt-get install -y \
    xvfb=2:1.19.6-1ubuntu4 \
    x11vnc=0.9.13-3 \
    x11-xkb-utils=7.7+3 \
    xfonts-100dpi=1:1.0.4+nmu1 \
    xfonts-75dpi=1:1.0.4+nmu1 \
    xfonts-scalable=1:1.0.3-1.1 \
    xfonts-cyrillic=1:1.0.4 \
    x11-apps=7.7+6ubuntu1 \
    && \
  echo "installed xvfb"

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

RUN \
  apt-get install -y \
    gtk2-engines-pixbuf=2.24.32-1ubuntu1 \
    firefox=59.0.2+build1-0ubuntu1 \
    libnss3=2:3.35-2ubuntu2 \
    lsb-release=9.20170808ubuntu1 \
    xdg-utils=1.1.2-1ubuntu2 \
    && \
  echo "installed headless firefox dependencies"

# magic from https://github.com/puppeteer/puppeteer/issues/3451#issuecomment-523961368
RUN echo 'kernel.unprivileged_userns_clone=1' > /etc/sysctl.d/userns.conf

RUN \
  apt-get install -y \
    g++-8=8-20180414-1ubuntu2 \
    cmake=3.10.2-1ubuntu2 \
    build-essential=12.4ubuntu1 \
    python-virtualenv=15.1.0+ds-1.1 \
    python-pip=9.0.1-2 \
    python3-virtualenv=8.1.1-2 \
    python3-pip=8.1.1-2 \
    nodejs=8.10.0~dfsg-2 \
    npm=3.5.2-0ubuntu4 \
    tar=1.29b-2 \
    gzip=1.6-5ubuntu1 \
    libpthread-stubs0-dev=0.3-4 \
    libc6-dbg=2.27-3ubuntu1 \
    gdb=8.1-0ubuntu3 \
    doxygen=1.8.13-10 \
    && \
  echo "installed core dependencies"

RUN \
  apt-get install -y \
    curl=7.58.0-2ubuntu3 \
    git=1:2.17.0-1ubuntu1 \
    htop=2.1.0-3 \
    man \
    unzip=6.0-21ubuntu1 \
    vim=2:8.0.1453-1ubuntu1 \
    nano=2.9.3-2 \
    wget=1.19.4-1ubuntu2 \
    && \
  echo "installed creature comforts"

RUN \
  update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 90 \
  && \
  npm install -g n \
  && \
  n 12.18.2 \
  && \
  export python="/usr/bin/python" \
  && \
  npm install source-map \
  && \
  echo "finalized set up dependency versions"

RUN \
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
  cd /opt/Empirical/tests \
    && \
  make test-web \
    && \
  make test-base \
    && \
  make clean \
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

RUN \
  pip install -r /opt/Empirical/third-party/requirements.txt \
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
