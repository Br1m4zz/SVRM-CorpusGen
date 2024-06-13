
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

RUN pip3 install gcovr==4.2

# Use ubuntu as default username
USER ubuntu
WORKDIR /home/ubuntu
ENV WORKDIR="/home/ubuntu"


# Download and compile LightFTP for fuzzing
COPY --chown=ubuntu:ubuntu fuzzing.patch ${WORKDIR}/fuzzing.patch

RUN git clone https://github.com/hfiref0x/LightFTP.git ${WORKDIR}/LightFTP
RUN cd LightFTP && \    
    git checkout 5980ea1 && \
    patch -p1 < ${WORKDIR}/fuzzing.patch && \
    cd Source/Release 

ENV SVInfos_PATH="/home/ubuntu/SVConfig"    



# Set up Learner
RUN git clone https://github.com/Br1m4zz/state-instrument.git ${WORKDIR}/state-instrument
RUN git clone https://github.com/Br1m4zz/SVRM-Learner.git ${WORKDIR}/svcs-learner

RUN cd state-instrument && \
    make clean all $MAKE_OPT

RUN cd svcs-learner && \
    mvn clean && \
    mvn package shade:shade


COPY --chown=ubuntu:ubuntu SVConfig ${WORKDIR}/SVConfig
COPY --chown=ubuntu:ubuntu in-ftp ${WORKDIR}/in-ftp
COPY --chown=ubuntu:ubuntu fftp.conf ${WORKDIR}/fftp.conf
COPY --chown=ubuntu:ubuntu clean.sh ${WORKDIR}/clean.sh
RUN chmod +x clean.sh

ENV SVCONFIG="${WORKDIR}/SVConfig"
ENV STATERT="${WORKDIR}/state-instrument"
ENV SVCS="${WORKDIR}/svcs-learner/target"
ENV TARGET="${WORKDIR}/LightFTP/Source/Release/fftp"

ENV CC=clang-10
ENV CFLAGS='-fno-discard-value-names -g -O0 -Xclang -load -Xclang /home/ubuntu/state-instrument/SVInstrument_Pass.so' 
ENV SVINFOS_PATH=/home/ubuntu/SVConfig

RUN cd $WORKDIR && \
    cd LightFTP && \    
    cd Source/Release && \  
    make clean all
