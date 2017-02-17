#!/usr/bin/env bash

##partial credit goes to new-kid

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
cd test/test_librt8900
./test_librt8900 --gtest_output=xml:gtestresults.xml


##then point jenkins run "sh -x $WORKSPACE/jenkins.build-test-report.sh"