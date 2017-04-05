#!/usr/bin/env bash

##partial credit goes to new-kid on Github

## requires install of cmake Cppcheck build-essentials

cat << EndOfMessage
##########################
## Creating build files ##
## in a new directory   ##
##########################
EndOfMessage
mkdir -p build/
cd build/
cmake ..

cat << EndOfMessage
#######################
## Starting compile  ##
#######################
EndOfMessage
make -j`nproc`

cat << EndOfMessage
#####################
## running tests   ##
#####################
EndOfMessage
${WORKSPACE}/build/test/test_librt8900/test_librt8900 --gtest_color=yes --gtest_output=xml:gtestresults.xml
cppcheck -j`nproc` --quiet --std=c11 --xml --xml-version=2 --enable=warning,performance,portability,information,missingInclude -i ${WORKSPACE}/build/ ${WORKSPACE} 2> ${WORKSPACE}/build/cppcheck.xml


##then point jenkins run "sh -x $WORKSPACE/jenkins.build-test-report.sh"