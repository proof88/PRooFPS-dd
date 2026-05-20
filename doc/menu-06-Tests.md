\page page_tests Tests

There are 4 kind of tests defined for PRooFPS-dd:
 - unit tests,
 - performance tests,
 - E2E (end-to-end) tests,
 - manual tests.

Unfortunately, which test is active or inactive, depends on the content of the Tests/PRooFPS-dd-Tests.cpp file (except the manual tests), this shall be changed in the future.  
Running unit-, performance- and E2E-tests is explained in [Tests/_how-to-auto-test.txt](https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/_how-to-auto-test.txt).  
Executing manual tests is explained in [Tests/_how-to-manual-test-00.txt](https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/_how-to-manual-test-00.txt).

[TOC]

\section tests_unit Unit Tests

Unit tests are in the Tests directory.  
They are built on the [455-355-7357-88 (ASS-ESS-TEST-88) framework](https://github.com/proof88/455-355-7357-88).

The main source file is Tests/PRooFPS-dd-Tests.cpp.  
To build and run them, either the DebugTest_PRooFPS-dd or the ReleaseTest_PRooFPS-dd solution configuration needs to be active.  
Please read [Tests/_how-to-auto-test.txt](https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/_how-to-auto-test.txt) fore more details.

\section tests_perf Performance Tests

Performance tests are also in the Tests directory.  
They are also built on the [455-355-7357-88 (ASS-ESS-TEST-88) framework](https://github.com/proof88/455-355-7357-88).

The main source file is Tests/PRooFPS-dd-Tests.cpp.  
To build and run them, either the DebugTest_PRooFPS-dd or the ReleaseTest_PRooFPS-dd solution configuration needs to be active.  
Please read [Tests/_how-to-auto-test.txt](https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/_how-to-auto-test.txt) fore more details.

\section tests_e2e E2E / Regression Tests

End-to-end / Regression tests are also in the Tests directory.  
They are also built on the [455-355-7357-88 (ASS-ESS-TEST-88) framework](https://github.com/proof88/455-355-7357-88).

The main source file is Tests/PRooFPS-dd-Tests.cpp.  
To build and run them, either the DebugTest_PRooFPS-dd or the ReleaseTest_PRooFPS-dd solution configuration needs to be active.  
Please read [Tests/_how-to-auto-test.txt](https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/_how-to-auto-test.txt) fore more details.

\section tests_manual Manual Tests

Manual tests are defined in simple text files in the Tests directory.  
Executing manual tests is explained in [Tests/_how-to-manual-test-00.txt](https://github.com/proof88/PRooFPS-dd/blob/main/PRooFPS-dd/Tests/_how-to-manual-test-00.txt).