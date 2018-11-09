@echo off 
make 
test > test-result-1.txt 
cmp test-result-1.txt test-standard-1.txt
if not errorlevel 1 goto ok1 
echo Test Failed! (1) 
:ok1 
test2 > test-result-2.txt 
cmp test-result-2.txt test-standard-2.txt 
if not errorlevel 1 goto ok2 
echo Test Failed! (2) 
:ok2 
