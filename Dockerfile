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
    libnss3 \
    lsb-release \
    xdg-utils \
    && \
  echo "installed headless firefox dependencies"

# magic from https://github.com/puppeteer/puppeteer/issues/3451#issuecomment-523961368
RUN echo 'kernel.unprivileged_userns_clone=1' > /etc/sysctl.d/userns.conf

RUN \
  apt-get install -y \
    g++-8 \
    cmake \
    build-essential \
    python-virtualenv \
    python-pip \
    python3-virtualenv \
    python-pip \
    python3-pip \
    nodejs \
    npm \
    tar \
    gzip \
    libpthread-stubs0-dev \
    libc6-dbg \
    gdb \
    doxygen \
    && \
  echo "installed core dependencies"

RUN \
  apt-get install -y \
    curl \
    git \
    htop \
    man \
    unzip \
    vim \
    nano \
    wget \
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
