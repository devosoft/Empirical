# Pull base image.
FROM ubuntu:16.04

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
    xvfb \
    x11vnc \
    x11-xkb-utils \
    xfonts-100dpi \
    xfonts-75dpi \
    xfonts-scalable \
    xfonts-cyrillic \
    x11-apps \
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
    gtk2-engines-pixbuf \
    firefox \
    && \
  echo "installed headless firefox dependencies"

RUN \
  apt-get install -y \
    g++-8=8.4.0-1ubuntu1~18.04 \
    cmake=3.10.2-1ubuntu2.18.04.1 \
    build-essential=12.4ubuntu1 \
    python-virtualenv=15.1.0+ds-1.1 \
    python-pip=9.0.1-2.3~ubuntu1.18.04.3 \
    nodejs=8.10.0~dfsg-2ubuntu0.4 \
    npm=3.5.2-0ubuntu4 \
    tar=1.29b-2ubuntu0.1 \
    gzip=1.6-5ubuntu1 \
    libpthread-stubs0-dev=0.3-4 \
    libc6-dbg=2.27-3ubuntu1.2 \
    gdb=8.2-0ubuntu1~18.04 \
    doxygen \
    && \
  echo "installed core dependencies"

RUN \
  apt-get install -y \
    curl=7.58.0-2ubuntu3.10 \
    git=1:2.17.1-1ubuntu0.7 \
    htop=2.1.0-3 \
    man \
    unzip=6.0-21ubuntu1 \
    vim=2:8.0.1453-1ubuntu1.3 \
    nano=2.9.3-2 \
    wget=1.19.4-1ubuntu2.2 \
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

# Define default entrypoint.
ENTRYPOINT ["/opt/entrypoint.sh"]

CMD ["bash"]
