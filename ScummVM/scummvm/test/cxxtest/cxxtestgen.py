#!/usr/bin/python
'''Usage: %s [OPTIONS] <input file(s)>
Generate test source file for CxxTest.

  -v, --version          Write CxxTest version
  -o, --output=NAME      Write output to file NAME
  --runner=CLASS         Create a main() function that runs CxxTest::CLASS
  --gui=CLASS            Like --runner, with GUI component
  --error-printer        Same as --runner=ErrorPrinter
  --abort-on-fail        Abort tests on failed asserts (like xUnit)
  --have-std             Use standard library (even if not found in tests)
  --no-std               Don\'t use standard library (even if found in tests)
  --have-eh              Use exception handling (even if not found in tests)
  --no-eh                Don\'t use exception handling (even if found in tests)
  --longlong=[TYPE]      Use TYPE (default: long long) as long long
  --template=TEMPLATE    Use TEMPLATE file to generate the test runner
  --include=HEADER       Include HEADER in test runner before other headers
  --root                 Write CxxTest globals
  --part                 Don\'t write CxxTest globals
  --no-static-init       Don\'t rely on static initialization
'''

import re
import sys
import getopt
import glob
import string

# Global variables
suites = []
suite = None
inBlock = 0

outputFileName = None
runner = None
gui = None
root = None
part = None
noStaticInit = None
templateFileName = None
headers = []

haveExceptionHandling = 0
noExceptionHandling = 0
haveStandardLibrary = 0
noStandardLibrary = 0
abortOnFail = 0
factor = 0
longlong = 0

def main():
    '''The main program'''
    files = parseCommandline()
    scanInputFiles( files )
    writeOutput()

def usage( problem = None ):
    '''Print usage info and exit'''
    if problem is None:
        print( usageString() )
        sys.exit(0)
    else:
        sys.stderr.write( usageString() )
        abort( problem )

def usageString():
    '''Construct program usage string'''
    return __doc__ % sys.argv[0]

def abort( problem ):
    '''Print error message and exit'''
    sys.stderr.write( '\n' )
    sys.stderr.write( problem )
    sys.stderr.write( '\n\n' )
    sys.exit(2)

def parseCommandline():
    '''Analyze command line arguments'''
    try:
        options, patterns = getopt.getopt( sys.argv[1:], 'o:r:',
                                           ['version', 'output=', 'runner=', 'gui=',
                                            'error-printer', 'abort-on-fail', 'have-std', 'no-std',
                                            'have-eh', 'no-eh', 'template=', 'include=',
                                            'root', 'part', 'no-static-init', 'factor', 'longlong='] )
    except getopt.error as problem:
        usage( problem )
    setOptions( options )
    return setFiles( patterns )

def setOptions( options ):
    '''Set options specified on command line'''
    global outputFileName, templateFileName, runner, gui, haveStandardLibrary, factor, longlong
    global haveExceptionHandling, noExceptionHandling, abortOnFail, headers, root, part, noStaticInit
    for o, a in options:
        if o in ('-v', '--version'):
            printVersion()
        elif o in ('-o', '--output'):
            outputFileName = a
        elif o == '--template':
            templateFileName = a
        elif o == '--runner':
            runner = a
        elif o == '--gui':
            gui = a
        elif o == '--include':
            if not re.match( r'^["<].*[>"]$', a ):
                a = ('"%s"' % a)
            headers.append( a )
        elif o == '--error-printer':
            runner = 'ErrorPrinter'
            haveStandardLibrary = 1
        elif o == '--abort-on-fail':
            abortOnFail = 1
        elif o == '--have-std':
            haveStandardLibrary = 1
        elif o == '--no-std':
            noStandardLibrary = 1
        elif o == '--have-eh':
            haveExceptionHandling = 1
        elif o == '--no-eh':
            noExceptionHandling = 1
        elif o == '--root':
            root = 1
        elif o == '--part':
            part = 1
        elif o == '--no-static-init':
            noStaticInit = 1
        elif o == '--factor':
            factor = 1
        elif o == '--longlong':
            if a:
                longlong = a
            else:
                longlong = 'long long'

    if noStaticInit and (root or part):
        abort( '--no-static-init cannot be used with --root/--part' )

    if gui and not runner:
        runner = 'StdioPrinter'

def printVersion():
    '''Print CxxTest version and exit'''
    sys.stdout.write( "This is CxxTest version 3.10.1.\n" )
    sys.exit(0)

def setFiles( patterns ):
    '''Set input files specified on command line'''
    files = expandWildcards( patterns )
    if len(files) is 0 and not root:
        usage( "No input files found" )
    return files

def expandWildcards( patterns ):
    '''Expand all wildcards in an array (glob)'''
    fileNames = []
    for pathName in patterns:
        patternFiles = glob.glob( pathName )
        for fileName in patternFiles:
            fileNames.append( fixBackslashes( fileName ) )
    return fileNames

def fixBackslashes( fileName ):
    '''Convert backslashes to slashes in file name'''
    return re.sub( r'\\', '/', fileName, 0 )

def scanInputFiles(files):
    '''Scan all input files for test suites'''
    for file in files:
        scanInputFile(file)
    global suites
    if len(suites) is 0 and not root:
        abort( 'No tests defined' )

def scanInputFile(fileName):
    '''Scan single input file for test suites'''
    file = open(fileName)
    lineNo = 0
    while 1:
        line = file.readline()
        if not line:
            break
        lineNo = lineNo + 1

        scanInputLine( fileName, lineNo, line )
    closeSuite()
    file.close()

def scanInputLine( fileName, lineNo, line ):
    '''Scan single input line for interesting stuff'''
    scanLineForExceptionHandling( line )
    scanLineForStandardLibrary( line )

    scanLineForSuiteStart( fileName, lineNo, line )

    global suite
    if suite:
        scanLineInsideSuite( suite, lineNo, line )

def scanLineInsideSuite( suite, lineNo, line ):
    '''Analyze line which is part of a suite'''
    global inBlock
    if lineBelongsToSuite( suite, lineNo, line ):
        scanLineForTest( suite, lineNo, line )
        scanLineForCreate( suite, lineNo, line )
        scanLineForDestroy( suite, lineNo, line )

def lineBelongsToSuite( suite, lineNo, line ):
    '''Returns whether current line is part of the current suite.
    This can be false when we are in a generated suite outside of CXXTEST_CODE() blocks
    If the suite is generated, adds the line to the list of lines'''
    if not suite['generated']:
        return 1

    global inBlock
    if not inBlock:
        inBlock = lineStartsBlock( line )
    if inBlock:
        inBlock = addLineToBlock( suite, lineNo, line )
    return inBlock


std_re = re.compile( r"\b(std\s*::|CXXTEST_STD|using\s+namespace\s+std\b|^\s*\#\s*include\s+<[a-z0-9]+>)" )
def scanLineForStandardLibrary( line ):
    '''Check if current line uses standard library'''
    global haveStandardLibrary, noStandardLibrary
    if not haveStandardLibrary and std_re.search(line):
        if not noStandardLibrary:
            haveStandardLibrary = 1

exception_re = re.compile( r"\b(throw|try|catch|TSM?_ASSERT_THROWS[A-Z_]*)\b" )
def scanLineForExceptionHandling( line ):
    '''Check if current line uses exception handling'''
    global haveExceptionHandling, noExceptionHandling
    if not haveExceptionHandling and exception_re.search(line):
        if not noExceptionHandling:
            haveExceptionHandling = 1

suite_re = re.compile( r'\bclass\s+(\w+)\s*:\s*public\s+((::)?\s*CxxTest\s*::\s*)?TestSuite\b' )
generatedSuite_re = re.compile( r'\bCXXTEST_SUITE\s*\(\s*(\w*)\s*\)' )
def scanLineForSuiteStart( fileName, lineNo, line ):
    '''Check if current line starts a new test suite'''
    m = suite_re.search( line )
    if m:
        startSuite( m.group(1), fileName, lineNo, 0 )
    m = generatedSuite_re.search( line )
    if m:
        sys.stdout.write( "%s:%s: Warning: Inline test suites are deprecated.\n" % (fileName, lineNo) )
        startSuite( m.group(1), fileName, lineNo, 1 )

def startSuite( name, file, line, generated ):
    '''Start scanning a new suite'''
    global suite
    closeSuite()
    suite = { 'name'         : name,
              'file'         : file,
              'cfile'        : cstr(file),
              'line'         : line,
              'generated'    : generated,
              'object'       : 'suite_%s' % name,
              'dobject'      : 'suiteDescription_%s' % name,
              'tlist'        : 'Tests_%s' % name,
              'tests'        : [],
              'lines'        : [] }

def lineStartsBlock( line ):
    '''Check if current line starts a new CXXTEST_CODE() block'''
    return re.search( r'\bCXXTEST_CODE\s*\(', line ) is not None

test_re = re.compile( r'^([^/]|/[^/])*\bvoid\s+([Tt]est\w+)\s*\(\s*(void)?\s*\)' )
def scanLineForTest( suite, lineNo, line ):
    '''Check if current line starts a test'''
    m = test_re.search( line )
    if m:
        addTest( suite, m.group(2), lineNo )

def addTest( suite, name, line ):
    '''Add a test function to the current suite'''
    test = { 'name'   : name,
             'suite'  : suite,
             'class'  : 'TestDescription_%s_%s' % (suite['name'], name),
             'object' : 'testDescription_%s_%s' % (suite['name'], name),
             'line'   : line,
             }
    suite['tests'].append( test )

def addLineToBlock( suite, lineNo, line ):
    '''Append the line to the current CXXTEST_CODE() block'''
    line = fixBlockLine( suite, lineNo, line )
    line = re.sub( r'^.*\{\{', '', line )
    
    e = re.search( r'\}\}', line )
    if e:
        line = line[:e.start()]
    suite['lines'].append( line )
    return e is None

def fixBlockLine( suite, lineNo, line):
    '''Change all [E]TS_ macros used in a line to _[E]TS_ macros with the correct file/line'''
    return re.sub( r'\b(E?TSM?_(ASSERT[A-Z_]*|FAIL))\s*\(',
                   r'_\1(%s,%s,' % (suite['cfile'], lineNo),
                   line, 0 )

create_re = re.compile( r'\bstatic\s+\w+\s*\*\s*createSuite\s*\(\s*(void)?\s*\)' )
def scanLineForCreate( suite, lineNo, line ):
    '''Check if current line defines a createSuite() function'''
    if create_re.search( line ):
        addSuiteCreateDestroy( suite, 'create', lineNo )

destroy_re = re.compile( r'\bstatic\s+void\s+destroySuite\s*\(\s*\w+\s*\*\s*\w*\s*\)' )
def scanLineForDestroy( suite, lineNo, line ):
    '''Check if current line defines a destroySuite() function'''
    if destroy_re.search( line ):
        addSuiteCreateDestroy( suite, 'destroy', lineNo )

def cstr( str ):
    '''Convert a string to its C representation'''
    return '"' + str.replace( '\\', '\\\\' ) + '"'


def addSuiteCreateDestroy( suite, which, line ):
    '''Add createSuite()/destroySuite() to current suite'''
    if suite.has_key(which):
        abort( '%s:%s: %sSuite() already declared' % ( suite['file'], str(line), which ) )
    suite[which] = line

def closeSuite():
    '''Close current suite and add it to the list if valid'''
    global suite
    if suite is not None:
        if len(suite['tests']) is not 0:
            verifySuite(suite)
            rememberSuite(suite)
        suite = None

def verifySuite(suite):
    '''Verify current suite is legal'''
    if 'create' in suite and not 'destroy' in suite:
        abort( '%s:%s: Suite %s has createSuite() but no destroySuite()' %
               (suite['file'], suite['create'], suite['name']) )
    if 'destroy' in suite and not 'create' in suite:
        abort( '%s:%s: Suite %s has destroySuite() but no createSuite()' %
               (suite['file'], suite['destroy'], suite['name']) )

def rememberSuite(suite):
    '''Add current suite to list'''
    global suites
    suites.append( suite )

def writeOutput():
    '''Create output file'''
    if templateFileName:
        writeTemplateOutput()
    else:
        writeSimpleOutput()

def writeSimpleOutput():
    '''Create output not based on template'''
    output = startOutputFile()
    writePreamble( output )
    writeMain( output )
    writeWorld( output )
    output.close()

include_re = re.compile( r"\s*\#\s*include\s+<cxxtest/" )
preamble_re = re.compile( r"^\s*<CxxTest\s+preamble>\s*$" )
world_re = re.compile( r"^\s*<CxxTest\s+world>\s*$" )
def writeTemplateOutput():
    '''Create output based on template file'''
    template = open(templateFileName)
    output = startOutputFile()
    while 1:
        line = template.readline()
        if not line:
            break;
        if include_re.search( line ):
            writePreamble( output )
            output.write( line )
        elif preamble_re.search( line ):
            writePreamble( output )
        elif world_re.search( line ):
            writeWorld( output )
        else:
            output.write( line )
    template.close()
    output.close()

def startOutputFile():
    '''Create output file and write header'''
    if outputFileName is not None:
        output = open( outputFileName, 'w' )
    else:
        output = sys.stdout
    output.write( "/* Generated file, do not edit */\n\n" )
    return output

wrotePreamble = 0
def writePreamble( output ):
    '''Write the CxxTest header (#includes and #defines)'''
    global wrotePreamble, headers, longlong
    if wrotePreamble: return
    output.write( "#ifndef CXXTEST_RUNNING\n" )
    output.write( "#define CXXTEST_RUNNING\n" )
    output.write( "#endif\n" )
    output.write( "\n" )
    if haveStandardLibrary:
        output.write( "#define _CXXTEST_HAVE_STD\n" )
    if haveExceptionHandling:
        output.write( "#define _CXXTEST_HAVE_EH\n" )
    if abortOnFail:
        output.write( "#define _CXXTEST_ABORT_TEST_ON_FAIL\n" )
    if longlong:
        output.write( "#define _CXXTEST_LONGLONG %s\n" % longlong )
    if factor:
        output.write( "#define _CXXTEST_FACTOR\n" )
    for header in headers:
        output.write( "#include %s\n" % header )
    output.write( "#include <cxxtest/TestListener.h>\n" )
    output.write( "#include <cxxtest/TestTracker.h>\n" )
    output.write( "#include <cxxtest/TestRunner.h>\n" )
    output.write( "#include <cxxtest/RealDescriptions.h>\n" )
    if runner:
        output.write( "#include <cxxtest/%s.h>\n" % runner )
    if gui:
        output.write( "#include <cxxtest/%s.h>\n" % gui )
    output.write( "\n" )
    wrotePreamble = 1

def writeMain( output ):
    '''Write the main() function for the test runner'''
    if gui:
        output.write( 'int main( int argc, char *argv[] ) {\n' )
        if noStaticInit:
            output.write( ' CxxTest::initialize();\n' )
        output.write( ' return CxxTest::GuiTuiRunner<CxxTest::%s, CxxTest::%s>( argc, argv ).run();\n' % (gui, runner) )
        output.write( '}\n' )
    elif runner:
        output.write( 'int main() {\n' )
        if noStaticInit:
            output.write( ' CxxTest::initialize();\n' )
        output.write( ' return CxxTest::%s().run();\n' % runner )
        output.write( '}\n' )

wroteWorld = 0
def writeWorld( output ):
    '''Write the world definitions'''
    global wroteWorld, part
    if wroteWorld: return
    writePreamble( output )
    writeSuites( output )
    if root or not part:
        writeRoot( output )
    if noStaticInit:
        writeInitialize( output )
    wroteWorld = 1

def writeSuites(output):
    '''Write all TestDescriptions and SuiteDescriptions'''
    for suite in suites:
        writeInclude( output, suite['file'] )
        if isGenerated(suite):
            generateSuite( output, suite )
        if isDynamic(suite):
            writeSuitePointer( output, suite )
        else:
            writeSuiteObject( output, suite )
        writeTestList( output, suite )
        writeSuiteDescription( output, suite )
        writeTestDescriptions( output, suite )

def isGenerated(suite):
    '''Checks whether a suite class should be created'''
    return suite['generated']

def isDynamic(suite):
    '''Checks whether a suite is dynamic'''
    return 'create' in suite

lastIncluded = ''
def writeInclude(output, file):
    '''Add #include "file" statement'''
    global lastIncluded
    if file == lastIncluded: return
    output.writelines( [ '#include "', file, '"\n\n' ] )
    lastIncluded = file

def generateSuite( output, suite ):
    '''Write a suite declared with CXXTEST_SUITE()'''
    output.write( 'class %s : public CxxTest::TestSuite {\n' % suite['name'] )
    output.write( 'public:\n' )
    for line in suite['lines']:
        output.write(line)
    output.write( '};\n\n' )

def writeSuitePointer( output, suite ):
    '''Create static suite pointer object for dynamic suites'''
    if noStaticInit:
        output.write( 'static %s *%s;\n\n' % (suite['name'], suite['object']) )
    else:
        output.write( 'static %s *%s = 0;\n\n' % (suite['name'], suite['object']) )

def writeSuiteObject( output, suite ):
    '''Create static suite object for non-dynamic suites'''
    output.writelines( [ "static ", suite['name'], " ", suite['object'], ";\n\n" ] )

def writeTestList( output, suite ):
    '''Write the head of the test linked list for a suite'''
    if noStaticInit:
        output.write( 'static CxxTest::List %s;\n' % suite['tlist'] )
    else:
        output.write( 'static CxxTest::List %s = { 0, 0 };\n' % suite['tlist'] )

def writeTestDescriptions( output, suite ):
    '''Write all test descriptions for a suite'''
    for test in suite['tests']:
        writeTestDescription( output, suite, test )

def writeTestDescription( output, suite, test ):
    '''Write test description object'''
    output.write( 'static class %s : public CxxTest::RealTestDescription {\n' % test['class'] )
    output.write( 'public:\n' )
    if not noStaticInit:
        output.write( ' %s() : CxxTest::RealTestDescription( %s, %s, %s, "%s" ) {}\n' %
                      (test['class'], suite['tlist'], suite['dobject'], test['line'], test['name']) )
    output.write( ' void runTest() { %s }\n' % runBody( suite, test ) )
    output.write( '} %s;\n\n' % test['object'] )

def runBody( suite, test ):
    '''Body of TestDescription::run()'''
    if isDynamic(suite): return dynamicRun( suite, test )
    else: return staticRun( suite, test )

def dynamicRun( suite, test ):
    '''Body of TestDescription::run() for test in a dynamic suite'''
    return 'if ( ' + suite['object'] + ' ) ' + suite['object'] + '->' + test['name'] + '();'
    
def staticRun( suite, test ):
    '''Body of TestDescription::run() for test in a non-dynamic suite'''
    return suite['object'] + '.' + test['name'] + '();'
    
def writeSuiteDescription( output, suite ):
    '''Write SuiteDescription object'''
    if isDynamic( suite ):
        writeDynamicDescription( output, suite )
    else:
        writeStaticDescription( output, suite )

def writeDynamicDescription( output, suite ):
    '''Write SuiteDescription for a dynamic suite'''
    output.write( 'CxxTest::DynamicSuiteDescription<%s> %s' % (suite['name'], suite['dobject']) )
    if not noStaticInit:
        output.write( '( %s, %s, "%s", %s, %s, %s, %s )' %
                      (suite['cfile'], suite['line'], suite['name'], suite['tlist'],
                       suite['object'], suite['create'], suite['destroy']) )
    output.write( ';\n\n' )

def writeStaticDescription( output, suite ):
    '''Write SuiteDescription for a static suite'''
    output.write( 'CxxTest::StaticSuiteDescription %s' % suite['dobject'] )
    if not noStaticInit:
        output.write( '( %s, %s, "%s", %s, %s )' %
                      (suite['cfile'], suite['line'], suite['name'], suite['object'], suite['tlist']) )
    output.write( ';\n\n' )

def writeRoot(output):
    '''Write static members of CxxTest classes'''
    output.write( '#include <cxxtest/Root.cpp>\n' )

def writeInitialize(output):
    '''Write CxxTest::initialize(), which replaces static initialization'''
    output.write( 'namespace CxxTest {\n' )
    output.write( ' void initialize()\n' )
    output.write( ' {\n' )
    for suite in suites:
        output.write( '  %s.initialize();\n' % suite['tlist'] )
        if isDynamic(suite):
            output.write( '  %s = 0;\n' % suite['object'] )
            output.write( '  %s.initialize( %s, %s, "%s", %s, %s, %s, %s );\n' %
                          (suite['dobject'], suite['cfile'], suite['line'], suite['name'],
                           suite['tlist'], suite['object'], suite['create'], suite['destroy']) )
        else:
            output.write( '  %s.initialize( %s, %s, "%s", %s, %s );\n' %
                          (suite['dobject'], suite['cfile'], suite['line'], suite['name'],
                           suite['object'], suite['tlist']) )

        for test in suite['tests']:
            output.write( '  %s.initialize( %s, %s, %s, "%s" );\n' %
                          (test['object'], suite['tlist'], suite['dobject'], test['line'], test['name']) )

    output.write( ' }\n' )
    output.write( '}\n' )

main()
