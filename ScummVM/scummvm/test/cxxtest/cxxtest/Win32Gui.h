#ifndef __cxxtest__Win32Gui_h__
#define __cxxtest__Win32Gui_h__

//
// The Win32Gui displays a simple progress bar using the Win32 API.
//
// It accepts the following command line options:
//   -minimized    Start minimized, pop up on error
//   -keep         Don't close the window at the end
//   -title TITLE  Set the window caption
//
// If both -minimized and -keep are specified, GUI will only keep the
// window if it's in focus.
//
// N.B. If you're wondering why this class doesn't use any standard
// library or STL (<string> would have been nice) it's because it only
// uses "straight" Win32 API.
//

#include <cxxtest/Gui.h>

#include <windows.h>
#include <commctrl.h>

namespace CxxTest
{
    class Win32Gui : public GuiListener
    {
    public:
        void enterGui( int &argc, char **argv )
	{
	    parseCommandLine( argc, argv );
	}

        void enterWorld( const WorldDescription &wd )
        {
            getTotalTests( wd );
            _testsDone = 0;
            startGuiThread();
        }

	void guiEnterSuite( const char *suiteName )
	{
	    showSuiteName( suiteName );
            reset( _suiteStart );
	}

        void guiEnterTest( const char *suiteName, const char *testName )
        {
            ++ _testsDone;
            setTestCaption( suiteName, testName );
            showTestName( testName );
	    showTestsDone();
            progressBarMessage( PBM_STEPIT );
            reset( _testStart );
        }

        void yellowBar()
        {
	    setColor( 255, 255, 0 );
            setIcon( IDI_WARNING );
            getTotalTests();
        }

        void redBar()
        {
            if ( _startMinimized )
                showMainWindow( SW_SHOWNORMAL );
	    setColor( 255, 0, 0 );
	    setIcon( IDI_ERROR );
            getTotalTests();
        }

        void leaveGui()
        {
            if ( keep() )
            {
                showSummary();
                WaitForSingleObject( _gui, INFINITE );
            }
            DestroyWindow( _mainWindow );
        }

    private:
        const char *_title;
        bool _startMinimized, _keep;
        HANDLE _gui;
        WNDCLASSEX _windowClass;
        HWND _mainWindow, _progressBar, _statusBar;
        HANDLE _canStartTests;
        unsigned _numTotalTests, _testsDone;
        char _strTotalTests[WorldDescription::MAX_STRLEN_TOTAL_TESTS];
        enum {
            STATUS_SUITE_NAME, STATUS_SUITE_TIME,
            STATUS_TEST_NAME, STATUS_TEST_TIME,
            STATUS_TESTS_DONE, STATUS_WORLD_TIME,
            STATUS_TOTAL_PARTS
        };
        int _statusWidths[STATUS_TOTAL_PARTS];
        unsigned _statusOffsets[STATUS_TOTAL_PARTS];
        unsigned _statusTotal;
        char _statusTestsDone[sizeof("1000000000 of  (100%)") + WorldDescription::MAX_STRLEN_TOTAL_TESTS];
        DWORD _worldStart, _suiteStart, _testStart;
        char _timeString[sizeof("00:00:00")];

        void parseCommandLine( int argc, char **argv )
        {
            _startMinimized = _keep = false;
	    _title = argv[0];

            for ( int i = 1; i < argc; ++ i )
            {
                if ( !lstrcmpA( argv[i], "-minimized" ) )
                    _startMinimized = true;
                else if ( !lstrcmpA( argv[i], "-keep" ) )
                    _keep = true;
                else if ( !lstrcmpA( argv[i], "-title" ) && (i + 1 < argc) )
                    _title = argv[++i];
            }
        }

        void getTotalTests()
        {
            getTotalTests( tracker().world() );
        }

        void getTotalTests( const WorldDescription &wd )
        {
            _numTotalTests = wd.numTotalTests();
            wd.strTotalTests( _strTotalTests );
        }

        void startGuiThread()
        {
            _canStartTests = CreateEvent( NULL, TRUE, FALSE, NULL );
			DWORD threadId;
            _gui = CreateThread( NULL, 0, &(Win32Gui::guiThread), (LPVOID)this, 0, &threadId );
            WaitForSingleObject( _canStartTests, INFINITE );
        }

        static DWORD WINAPI guiThread( LPVOID parameter )
        {
            ((Win32Gui *)parameter)->gui();
            return 0;
        }

        void gui()
        {
            registerWindowClass();
            createMainWindow();
            initCommonControls();
            createProgressBar();
            createStatusBar();
            centerMainWindow();
            showMainWindow();
            startTimer();
            startTests();

            messageLoop();
        }

        void registerWindowClass()
        {
            _windowClass.cbSize = sizeof(_windowClass);
            _windowClass.style = CS_HREDRAW | CS_VREDRAW;
            _windowClass.lpfnWndProc = &(Win32Gui::windowProcedure);
            _windowClass.cbClsExtra = 0;
            _windowClass.cbWndExtra = sizeof(LONG);
            _windowClass.hInstance = (HINSTANCE)NULL;
            _windowClass.hIcon = (HICON)NULL;
            _windowClass.hCursor = (HCURSOR)NULL;
            _windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
            _windowClass.lpszMenuName = NULL;
            _windowClass.lpszClassName = TEXT("CxxTest Window Class");
            _windowClass.hIconSm = (HICON)NULL;

            RegisterClassEx( &_windowClass );
        }

        void createMainWindow()
        {
            _mainWindow = createWindow( _windowClass.lpszClassName, WS_OVERLAPPEDWINDOW );
        }

        void initCommonControls()
        {
            HMODULE dll = LoadLibraryA( "comctl32.dll" );
            if ( !dll )
		return;

	    typedef void (WINAPI *FUNC)( void );
	    FUNC func = (FUNC)GetProcAddress( dll, "InitCommonControls" );
	    if ( !func )
                return;

	    func();
        }

        void createProgressBar()
        {
            _progressBar = createWindow( PROGRESS_CLASS, WS_CHILD | WS_VISIBLE | PBS_SMOOTH, _mainWindow );

#ifdef PBM_SETRANGE32
            progressBarMessage( PBM_SETRANGE32, 0, _numTotalTests );
#else // No PBM_SETRANGE32, use PBM_SETRANGE
	    progressBarMessage( PBM_SETRANGE, 0, MAKELPARAM( 0, (WORD)_numTotalTests ) );
#endif // PBM_SETRANGE32
            progressBarMessage( PBM_SETPOS, 0 );
            progressBarMessage( PBM_SETSTEP, 1 );
            greenBar();
            UpdateWindow( _progressBar );
        }

        void createStatusBar()
        {
            _statusBar = createWindow( STATUSCLASSNAME, WS_CHILD | WS_VISIBLE, _mainWindow );
            setRatios( 4, 1, 3, 1, 3, 1 );
        }

        void setRatios( unsigned suiteNameRatio, unsigned suiteTimeRatio,
                        unsigned testNameRatio, unsigned testTimeRatio,
                        unsigned testsDoneRatio, unsigned worldTimeRatio )
        {
            _statusTotal = 0;
            _statusOffsets[STATUS_SUITE_NAME] = (_statusTotal += suiteNameRatio);
            _statusOffsets[STATUS_SUITE_TIME] = (_statusTotal += suiteTimeRatio);
            _statusOffsets[STATUS_TEST_NAME] = (_statusTotal += testNameRatio);
            _statusOffsets[STATUS_TEST_TIME] = (_statusTotal += testTimeRatio);
            _statusOffsets[STATUS_TESTS_DONE] = (_statusTotal += testsDoneRatio);
            _statusOffsets[STATUS_WORLD_TIME] = (_statusTotal += worldTimeRatio);
        }

        HWND createWindow( LPCTSTR className, DWORD style, HWND parent = (HWND)NULL )
        {
            return CreateWindow( className, NULL, style, 0, 0, 0, 0, parent,
                                 (HMENU)NULL, (HINSTANCE)NULL, (LPVOID)this );
        }

        void progressBarMessage( UINT message, WPARAM wParam = 0, LPARAM lParam = 0 )
        {
            SendMessage( _progressBar, message, wParam, lParam );
        }

        void centerMainWindow()
        {
            RECT screen;
            getScreenArea( screen );

            LONG screenWidth = screen.right - screen.left;
            LONG screenHeight = screen.bottom - screen.top;

            LONG xCenter = (screen.right + screen.left) / 2;
            LONG yCenter = (screen.bottom + screen.top) / 2;

            LONG windowWidth = (screenWidth * 4) / 5;
            LONG windowHeight = screenHeight / 10;
            LONG minimumHeight = 2 * (GetSystemMetrics( SM_CYCAPTION ) + GetSystemMetrics( SM_CYFRAME ));
            if ( windowHeight < minimumHeight )
                windowHeight = minimumHeight;

            SetWindowPos( _mainWindow, HWND_TOP,
                          xCenter - (windowWidth / 2), yCenter - (windowHeight / 2),
                          windowWidth, windowHeight, 0 );
        }

        void getScreenArea( RECT &area )
        {
            if ( !getScreenAreaWithoutTaskbar( area ) )
                getWholeScreenArea( area );
        }

        bool getScreenAreaWithoutTaskbar( RECT &area )
        {
            return (SystemParametersInfo( SPI_GETWORKAREA, sizeof(RECT), &area, 0 ) != 0);
        }

        void getWholeScreenArea( RECT &area )
        {
            area.left = area.top = 0;
            area.right = GetSystemMetrics( SM_CXSCREEN );
            area.bottom = GetSystemMetrics( SM_CYSCREEN );
        }

        void showMainWindow()
        {
            showMainWindow( _startMinimized ? SW_MINIMIZE : SW_SHOWNORMAL );
            UpdateWindow( _mainWindow );
        }

        void showMainWindow( int mode )
        {
            ShowWindow( _mainWindow, mode );
        }

        enum { TIMER_ID = 1, TIMER_DELAY = 1000 };

        void startTimer()
        {
            reset( _worldStart );
            reset( _suiteStart );
            reset( _testStart );
            SetTimer( _mainWindow, TIMER_ID, TIMER_DELAY, 0 );
        }

        void reset( DWORD &tick )
        {
            tick = GetTickCount();
        }

        void startTests()
        {
            SetEvent( _canStartTests );
        }

        void messageLoop()
        {
            MSG message;
            while ( BOOL haveMessage = GetMessage( &message, NULL, 0, 0 ) )
                if ( haveMessage != -1 )
                    DispatchMessage( &message );
        }

        static LRESULT CALLBACK windowProcedure( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
        {
            if ( message == WM_CREATE )
                setUp( window, (LPCREATESTRUCT)lParam );

            Win32Gui *that = (Win32Gui *)GetWindowLong( window, GWL_USERDATA );
            return that->handle( window, message, wParam, lParam );
        }

        static void setUp( HWND window, LPCREATESTRUCT create )
        {
            SetWindowLong( window, GWL_USERDATA, (LONG)create->lpCreateParams );
        }

        LRESULT handle( HWND window, UINT message, WPARAM wParam, LPARAM lParam )
        {
            switch ( message )
            {
            case WM_SIZE: resizeControls(); break;

            case WM_TIMER: updateTime(); break;

            case WM_CLOSE:
            case WM_DESTROY:
            case WM_QUIT:
                ExitProcess( 0 );

            default: return DefWindowProc( window, message, wParam, lParam );
            }
            return 0;
        }

        void resizeControls()
        {
            RECT r;
            GetClientRect( _mainWindow, &r );
            LONG width = r.right - r.left;
            LONG height = r.bottom - r.top;

            GetClientRect( _statusBar, &r );
            LONG statusHeight = r.bottom - r.top;
            LONG resizeGripWidth = statusHeight;
            LONG progressHeight = height - statusHeight;

            SetWindowPos( _progressBar, HWND_TOP, 0, 0, width, progressHeight, 0 );
            SetWindowPos( _statusBar, HWND_TOP, 0, progressHeight, width, statusHeight, 0 );
            setStatusParts( width - resizeGripWidth );
        }

        void setStatusParts( LONG width )
        {
            for ( unsigned i = 0; i < STATUS_TOTAL_PARTS; ++ i )
                _statusWidths[i] = (width * _statusOffsets[i]) / _statusTotal;

            statusBarMessage( SB_SETPARTS, STATUS_TOTAL_PARTS, _statusWidths );
        }

        void statusBarMessage( UINT message, WPARAM wParam = 0, const void *lParam = 0 )
        {
            SendMessage( _statusBar, message, wParam, (LPARAM)lParam );
        }

        void greenBar()
        {
            setColor( 0, 255, 0 );
            setIcon( IDI_INFORMATION );
        }

#ifdef PBM_SETBARCOLOR
        void setColor( BYTE red, BYTE green, BYTE blue )
        {
            progressBarMessage( PBM_SETBARCOLOR, 0, RGB( red, green, blue ) );
        }
#else // !PBM_SETBARCOLOR
        void setColor( BYTE, BYTE, BYTE )
        {
        }
#endif // PBM_SETBARCOLOR

        void setIcon( LPCTSTR icon )
        {
            SendMessage( _mainWindow, WM_SETICON, ICON_BIG, (LPARAM)loadStandardIcon( icon ) );
        }

        HICON loadStandardIcon( LPCTSTR icon )
        {
            return LoadIcon( (HINSTANCE)NULL, icon );
        }

        void setTestCaption( const char *suiteName, const char *testName )
        {
            setCaption( suiteName, "::", testName, "()" );
        }

        void setCaption( const char *a = "", const char *b = "", const char *c = "", const char *d = "" )
        {
            unsigned length = lstrlenA( _title ) + sizeof( " - " ) +
                lstrlenA( a ) + lstrlenA( b ) + lstrlenA( c ) + lstrlenA( d );
            char *name = allocate( length );
            lstrcpyA( name, _title );
            lstrcatA( name, " - " );
            lstrcatA( name, a );
            lstrcatA( name, b );
            lstrcatA( name, c );
            lstrcatA( name, d );
            SetWindowTextA( _mainWindow, name );
            deallocate( name );
        }

        void showSuiteName( const char *suiteName )
        {
            setStatusPart( STATUS_SUITE_NAME, suiteName );
	}

	void showTestName( const char *testName )
	{
            setStatusPart( STATUS_TEST_NAME, testName );
	}

	void showTestsDone()
	{
            wsprintfA( _statusTestsDone, "%u of %s (%u%%)",
                       _testsDone, _strTotalTests,
                       (_testsDone * 100) / _numTotalTests );
            setStatusPart( STATUS_TESTS_DONE, _statusTestsDone );
        }

        void updateTime()
        {
            setStatusTime( STATUS_WORLD_TIME, _worldStart );
            setStatusTime( STATUS_SUITE_TIME, _suiteStart );
            setStatusTime( STATUS_TEST_TIME, _testStart );
        }

        void setStatusTime( unsigned part, DWORD start )
        {
            unsigned total = (GetTickCount() - start) / 1000;
            unsigned hours = total / 3600;
            unsigned minutes = (total / 60) % 60;
            unsigned seconds = total % 60;

            if ( hours )
                wsprintfA( _timeString, "%u:%02u:%02u", hours, minutes, seconds );
            else
                wsprintfA( _timeString, "%02u:%02u", minutes, seconds );

            setStatusPart( part, _timeString );
        }

        bool keep()
        {
            if ( !_keep )
                return false;
            if ( !_startMinimized )
                return true;
            return (_mainWindow == GetForegroundWindow());
        }

        void showSummary()
        {
            stopTimer();
            setSummaryStatusBar();
            setSummaryCaption();
        }

        void setStatusPart( unsigned part, const char *text )
        {
            statusBarMessage( SB_SETTEXTA, part, text );
        }

        void stopTimer()
        {
            KillTimer( _mainWindow, TIMER_ID );
            setStatusTime( STATUS_WORLD_TIME, _worldStart );
        }

        void setSummaryStatusBar()
        {
            setRatios( 0, 0, 0, 0, 1, 1 );
            resizeControls();

            const char *tests = (_numTotalTests == 1) ? "test" : "tests";
            if ( tracker().failedTests() )
                wsprintfA( _statusTestsDone, "Failed %u of %s %s",
                          tracker().failedTests(), _strTotalTests, tests );
            else
                wsprintfA( _statusTestsDone, "%s %s passed", _strTotalTests, tests );

            setStatusPart( STATUS_TESTS_DONE, _statusTestsDone );
        }

        void setSummaryCaption()
        {
            setCaption( _statusTestsDone );
        }

        char *allocate( unsigned length )
        {
            return (char *)HeapAlloc( GetProcessHeap(), 0, length );
        }

        void deallocate( char *data )
        {
            HeapFree( GetProcessHeap(), 0, data );
        }
    };
}

#endif // __cxxtest__Win32Gui_h__
