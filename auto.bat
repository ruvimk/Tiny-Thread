@echo off 
make 
echo Date and time of test: 
date /t 
time /t 
echo. 
echo Running test 1 ... 
test > test-result-1.txt 
cmp test-result-1.txt test-standard-1.txt
if not errorlevel 1 goto ok1 
echo Test Failed! (1) 
goto done 
:ok1 
echo Running test 2 ... 
test2 > test-result-2.txt 
cmp test-result-2.txt test-standard-2.txt 
if not errorlevel 1 goto ok2 
echo Test Failed! (2) 
goto done 
:ok2 
echo Running test 3 ... 
test3 > test-result-3.txt 
cmp test-result-3.txt test-standard-3.txt 
if not errorlevel 1 goto ok3 
echo Test Failed! (3) 
goto done 
:ok3 
echo Running test 4 ... 
test4 > test-result-4.txt 
cmp test-result-4.txt test-standard-4.txt 
if not errorlevel 1 goto ok4 
echo Test Failed! (4) 
goto done 
:ok4 
echo Running test 5 ... 
test5 > test-result-5.txt 
cmp test-result-5.txt test-standard-5.txt 
if not errorlevel 1 goto ok5 
echo Test Failed! (5) 
goto done 
:ok5 

REM Can put more test cases here ... 

echo. 

echo All tests passed! 
echo. 

:done 
