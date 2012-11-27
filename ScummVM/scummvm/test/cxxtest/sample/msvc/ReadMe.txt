Sample files for Visual Studio
==============================

There are three projects in this workspace:

 - CxxTest_3_Generate runs cxxtestgen to create runner.cpp
 - CxxTest_2_Build compiles the generated file
 - CxxTest_1_Run runs the compiled binary

Whenever you build this workspace, the tests are run, and any failed assertions
are displayed as compilation errors (you can browse them using F4).

Note that to run this sample, you need first to create an environment
variable PERL or PYTHON, e.g. PERL=c:\perl\bin\perl.exe


To use these .dsp and .dsw files in your own project, run FixFiles.bat
to adjust them to where you've placed CxxTest and your own tests.

If you want to use just the .dsp files in your own workspace, don't
forget to:

 - Set up the dependencies (CxxTest_3_Generate depends on
   CxxTest_2_Build which depends on CxxTest_1_Run)

 - Add your own include paths, libraries etc. to the CxxTest_2_Build project


NOTE: I haven't used "Post-Build Step" to run the tests because I
wanted the tests to be executed even if nothing has changed.
