FROM ubuntu:20.04

#Install common dependencies
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get -y update && \
    apt-get -y install sudo \ 
    apt-utils \
    build-essential \
    openssl \
    clang-10 \
    graphviz-dev \
    graphviz \
    git \
    autoconf \
    libgnutls28-dev \
    libssl-dev \
    llvm-10-dev \
    python3-pip \
    nano \
    net-tools \
    vim \
    gdb \
    netcat \
    strace \
    wget \
    openjdk-11-jdk \
    maven

# Add a new user ubuntu, pass: ubuntu
RUN groupadd ubuntu && \
    useradd -rm -d /home/ubuntu -s /bin/bash -g ubuntu -G sudo -u 1000 ubuntu -p "$(openssl passwd -1 ubuntu)"
RUN chmod 777 /tmp

# Use ubuntu as default username
USER ubuntu
WORKDIR /home/ubuntu
ENV WORKDIR="/home/ubuntu"


# Download and compile live555 for fuzzing
COPY --chown=ubuntu:ubuntu fuzzing.patch ${WORKDIR}/fuzzing.patch
RUN git clone https://github.com/rgaufman/live555.git ${WORKDIR}/live555
RUN cd live555 && \
    git checkout ceeb4f4 && \
    patch -p1 < $WORKDIR/fuzzing.patch


# Set up Learner
RUN git clone https://gitee.com/Br1m4zz16/state-instrument.git ${WORKDIR}/state-instrument
RUN git clone https://gitee.com/Br1m4zz16/svcs-learner.git ${WORKDIR}/svcs-learner

RUN cd state-instrument && \
    make clean all $MAKE_OPT

RUN cd svcs-learner && \
    mvn package shade:shade


COPY --chown=ubuntu:ubuntu SVConfig ${WORKDIR}/SVConfig
COPY --chown=ubuntu:ubuntu in-rtsp ${WORKDIR}/in-rtsp
COPY --chown=ubuntu:ubuntu sample_media_sources ${WORKDIR}/sample_media_sources


ENV SVCONFIG="${WORKDIR}/SVConfig"
ENV CC=clang-10
ENV CFLAGS='-fno-discard-value-names -g -O0 -Xclang -load -Xclang /home/ubuntu/state-instrument/SVInstrument_Pass.so' 
ENV SVINFOS_PATH=/home/ubuntu/SVConfig


COPY --chown=ubuntu:ubuntu config.forlearner ${WORKDIR}/live555/config.forlearner
COPY --chown=ubuntu:ubuntu kill-server.sh ${WORKDIR}/kill-server
RUN chmod +x kill-server

ENV SVCONFIG="${WORKDIR}/SVConfig"


RUN cd $WORKDIR && \
    cd live555 && \
    ./genMakefiles forlearner &&\
    make clean all

RUN cp $WORKDIR/sample_media_sources/*.* /home/ubuntu/live555/testProgs
