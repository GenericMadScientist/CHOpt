#!/bin/bash
if [[ ! -d $HOME/boost_1_78_0 ]]
then
    wget -O $HOME/boost_1_78_0.tar.bz2 https://boostorg.jfrog.io/artifactory/main/release/1.78.0/source/boost_1_78_0.tar.bz2
    tar --bzip2 -xf $HOME/boost_1_78_0.tar.bz2 -C $HOME
    cd $HOME/boost_1_78_0
    export CC=gcc-10
    export CXX=g++-10
    ./bootstrap.sh --with-libraries=json,nowide,program_options --prefix=.
    ./b2 install > /dev/null
    cd $HOME/projects/CHOpt/build
fi