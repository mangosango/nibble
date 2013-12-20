#!/bin/bash
#
# test script for lab4 crawler
# by Sang Jin Lee for CS23
#
# input	  - none
# output  - error/success logging

echo TESTS FOR INPUT ERROR > logfile.dat

echo no args: >> logfile.dat
echo ===== OUTPUT START ===== >> logfile.dat
crawler >> logfile.dat
echo =====  OUTPUT END  ===== >> logfile.dat

if [ $? -eq 1 ]
then
	echo -e "\nTEST PASS" >> logfile.dat
else
	echo -e "\nTEST FAIL" >> logfile.dat
fi

echo not enough args: >> logfile.dat
echo ===== OUTPUT START ===== >> logfile.dat
crawler cs.dartmouth.edu >> logfile.dat
echo =====  OUTPUT END  ===== >> logfile.dat
if [ $? -eq 1 ]
then
        echo -e "\nTEST PASS" >> logfile.dat
else
        echo -e "\nTEST FAIL" >> logfile.dat
fi

echo too many args: >> logfile.dat
echo ===== OUTPUT START ===== >> logfile.dat
crawler cs.dartmouth.edu ./temp 1 extraArg >> logfile.dat
echo =====  OUTPUT END  ===== >> logfile.dat
if [ $? -eq 1 ]
then
        echo -e "\nTEST PASS" >> logfile.dat
else
        echo -e "\nTEST FAIL" >> logfile.dat
fi


echo incorrect url: >> logfile.dat
echo ===== OUTPUT START ===== >> logfile.dat
crawler dartmouth ./temp 1 >> logfile.dat
echo =====  OUTPUT END  ===== >> logfile.dat
if [ $? -eq 1 ]
then
        echo -e "\nTEST PASS" >> logfile.dat
else
        echo -e "\nTEST FAIL" >> logfile.dat
fi

echo incorrect directory: >> logfile.dat
echo ===== OUTPUT START ===== >> logfile.dat
crawler http://www.cs.dartmouth.edu ./invalid 1 >> logfile.dat
echo =====  OUTPUT END  ===== >> logfile.dat
if [ $? -eq 1 ]
then
        echo -e "\nTEST PASS" >> logfile.dat
else
        echo -e "\nTEST FAIL" >> logfile.dat
fi

echo depth is too large >> logfile.dat
echo ===== OUTPUT START ===== >> logfile.dat
crawler cs.dartmouth.edu ./temp/ 1100 >> logfile.dat
echo =====  OUTPUT END  ===== >> logfile.dat
if [ $? -eq 1 ]
then
        echo -e "\nTEST PASS" >> logfile.dat
else
        echo -e "\nTEST FAIL" >> logfile.dat
fi

echo TESTS FOR PROGRAM RUNNING >> logfile.dat

echo depth of 1 >> logfile.dat
echo ===== OUTPUT START ===== >> logfile.dat
crawler http://www.cs.dartmouth.edu ./temp 1 >> logfile.dat
echo =====  OUTPUT END  ===== >> logfile.dat
if [ $? -eq 1 ] 
then
        echo -e "\nTEST FAIL" >> logfile.dat
else
        echo -e "\nTEST PASS" >> logfile.dat
fi

echo depth of 3 >> logfile.dat
echo ===== OUTPUT START ===== >> logfile.dat
crawler http://www.cs.dartmouth.edu ./temp 3 >> logfile.dat
echo =====  OUTPUT END  ===== >> logfile.dat
if [ $? -eq 1 ] 
then
        echo -e "\nTEST FAIL" >> logfile.dat
else
        echo -e "\nTEST PASS" >> logfile.dat
fi
