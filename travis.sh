#!/bin/sh

set -e

apt-get -qq update
apt-get install --no-install-recommends -y \
	build-essential \
	libglib2.0-dev

gcc --version

./autogen.sh
make V=1
make V=1 distcheck
make V=1 clean

git status
