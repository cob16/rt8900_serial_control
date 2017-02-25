#!/usr/bin/env bash

##partial credit goes to new-kid on github

## requires install of cmake cppcheck build-essentials

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
make

cat << EndOfMessage
#####################
## running tests   ##
#####################
EndOfMessage
$WORKSPACE/test/test_librt8900/test_librt8900 --gtest_color=yes --gtest_output=xml:gtestresults.xml
cppcheck -j`nproc` --enable=warning,performance,portability,information,missingInclude --std=c11 --inconclusive --xml --xml-version=2 $WORKSPACE 2> cppcheck.xml


##then point jenkins run "sh -x $WORKSPACE/jenkins.build-test-report.sh"