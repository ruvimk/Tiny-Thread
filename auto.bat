@echo off 
make 
test > output2.txt 
cmp output1.txt output2.txt 
if not errorlevel 1 goto ok 
echo Test Failed! 
:ok 