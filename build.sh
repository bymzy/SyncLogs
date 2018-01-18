#! /bin/bash

echo "-step1: make objs dir"
mkdir -p objs

echo "-step2: check libcurl"
rpm -qa | grep "libcurl"
if [ $? -ne 0 ]
then
    yum -y install libcurl-devel
fi

echo "-step3: going to build"
make

echo "Done! FileGet is now avaliable in bin dir!"


