FROM ubuntu:14.04

# Install everything XEphem needs to compile.

ENV DEBIAN_FRONTEND noninteractive
RUN apt update
RUN apt upgrade -y -y
RUN apt install -y -y apt-file
RUN apt-file update
RUN apt install -y -y \
    build-essential \
    groff-base \
    libmotif-dev \
    libssl-dev \
    libxext-dev \
    libxmu-dev \
    libxp-dev \
    libxt-dev \
    x11proto-print-dev

# Remove the Linux empty /srv directory so it does not interrupt tab
# completion of /src, where we put the XEphem source tree.

RUN rmdir /srv

# Set up /root as our home directory, with a useful bash history and an
# “.xephem” symlink that shows XEphem where to find its support files.

ENV HOME /root
RUN ln -s /src/GUI/xephem/auxil /root/.xephem
RUN echo 'make -C /src/GUI/xephem && (cd /src/GUI/xephem; ./xephem)' \
    > /root/.bash_history

# Start the user in the /src directory.

WORKDIR /src/GUI/xephem
CMD /bin/bash
