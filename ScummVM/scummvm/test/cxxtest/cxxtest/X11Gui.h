#ifndef __cxxtest__X11Gui_h__
#define __cxxtest__X11Gui_h__

//
// X11Gui displays a simple progress bar using X11
//
// It accepts the following command-line arguments:
//  -title <title>              - Sets the application title
//  -fn or -font <font>         - Sets the font
//  -bg or -background <color>  - Sets the background color (default=Grey)
//  -fg or -foreground <color>  - Sets the text color (default=Black)
//  -green/-yellow/-red <color> - Sets the colors of the bar
//

#include <cxxtest/Gui.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace CxxTest
{
    class X11Gui : public GuiListener
    {
    public:
        void enterGui( int &argc, char **argv )
        {
            parseCommandLine( argc, argv );
        }

        void enterWorld( const WorldDescription &wd )
        {
            openDisplay();
            if ( _display ) {
                createColors();
                createWindow();
                createGc();
                createFont();
                centerWindow();
                initializeEvents();
                initializeBar( wd );
                processEvents();
            }
        }

        void guiEnterTest( const char *suiteName, const char *testName )
        {
            if ( _display ) {
                ++ _testsDone;
                setWindowName( suiteName, testName );
                redraw();
            }
        }

        void yellowBar()
        {
            if ( _display ) {
                _barColor = getColor( _yellowName );
                getTotalTests();
                processEvents();
            }
        }

        void redBar()
        {
            if ( _display ) {
                _barColor = getColor( _redName );
                getTotalTests();
                processEvents();
            }
        }

        void leaveGui()
        {
            if ( _display ) {
                freeFontInfo();
                destroyGc();
                destroyWindow();
                closeDisplay();
            }
        }

    private:
        const char *_programName;
        Display *_display;
        Window _window;
        unsigned _numTotalTests, _testsDone;
        char _strTotalTests[WorldDescription::MAX_STRLEN_TOTAL_TESTS];
        const char *_foregroundName, *_backgroundName;
        const char *_greenName, *_yellowName, *_redName;
        unsigned long _foreground, _background, _barColor;
        int _width, _height;
        GC _gc;
        const char *_fontName;
        XID _fontId;
        XFontStruct *_fontInfo;
        int _textHeight, _textDescent;
        long _eventMask;
        Colormap _colormap;

        void parseCommandLine( int &argc, char **argv )
        {
            _programName = argv[0];

            _fontName = 0;
            _foregroundName = "Black";
            _backgroundName = "Grey";
            _greenName = "Green";
            _yellowName = "Yellow";
            _redName = "Red";

            for ( int i = 1; i + 1 < argc; ++ i ) {
                if ( !strcmp( argv[i], "-title" ) )
                    _programName = argv[++ i];
                else if ( !strcmp( argv[i], "-fn" ) || !strcmp( argv[i], "-font" ) )
                    _fontName = argv[++ i];
                else if ( !strcmp( argv[i], "-fg" ) || !strcmp( argv[i], "-foreground" ) )
                    _foregroundName = argv[++ i];
                else if ( !strcmp( argv[i], "-bg" ) || !strcmp( argv[i], "-background" ) )
                    _backgroundName = argv[++ i];
                else if ( !strcmp( argv[i], "-green" ) )
                    _greenName = argv[++ i];
                else if ( !strcmp( argv[i], "-yellow" ) )
                    _yellowName = argv[++ i];
                else if ( !strcmp( argv[i], "-red" ) )
                    _redName = argv[++ i];
            }
        }

        void openDisplay()
        {
            _display = XOpenDisplay( NULL );
        }

        void createColors()
        {
            _colormap = DefaultColormap( _display, 0 );
            _foreground = getColor( _foregroundName );
            _background = getColor( _backgroundName );
        }

        unsigned long getColor( const char *colorName )
        {
            XColor color;
            XParseColor( _display, _colormap, colorName, &color );
            XAllocColor( _display, _colormap, &color );
            return color.pixel;
        }

        void createWindow()
        {
            _window = XCreateSimpleWindow( _display, RootWindow( _display, 0 ), 0, 0, 1, 1, 0, 0, _background );
        }

        void createGc()
        {
            _gc = XCreateGC( _display, _window, 0, 0 );
        }

        void createFont()
        {
            if ( !loadFont() )
                useDefaultFont();
            getFontInfo();
            _textHeight = _fontInfo->ascent + _fontInfo->descent;
            _textDescent = _fontInfo->descent;
        }

        bool loadFont()
        {
            if ( !_fontName )
                return false;
            _fontId = XLoadFont( _display, _fontName );
            return (XSetFont( _display, _gc, _fontId ) == Success);
        }

        void useDefaultFont()
        {
            _fontId = XGContextFromGC( _gc );
        }

        void getFontInfo()
        {
            _fontInfo = XQueryFont( _display, _fontId );
        }

        void freeFontInfo()
        {
            XFreeFontInfo( NULL, _fontInfo, 1 );
        }

        void initializeEvents()
        {
            _eventMask = ExposureMask;
            XSelectInput( _display, _window, _eventMask );
        }

        void initializeBar( const WorldDescription &wd )
        {
            getTotalTests( wd );
            _testsDone = 0;
            _barColor = getColor( _greenName );
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

        void centerWindow()
        {
            XMapWindow( _display, _window );

            Screen *screen = XDefaultScreenOfDisplay( _display );
            int screenWidth = WidthOfScreen( screen );
            int screenHeight = HeightOfScreen( screen );
            int xCenter = screenWidth / 2;
            int yCenter = screenHeight / 2;

            _width = (screenWidth * 4) / 5;
            _height = screenHeight / 14;

            XMoveResizeWindow( _display, _window, xCenter - (_width / 2), yCenter - (_height / 2), _width, _height );
        }

        void processEvents()
        {
            redraw();

            XEvent event;
            while( XCheckMaskEvent( _display, _eventMask, &event ) )
                redraw();
        }

        void setWindowName( const char *suiteName, const char *testName )
        {
            unsigned length = strlen( _programName ) + strlen( suiteName ) + strlen( testName ) + sizeof( " - ::()" );
            char *name = (char *)malloc( length );
            sprintf( name, "%s - %s::%s()", _programName, suiteName, testName );
            XSetStandardProperties( _display, _window, name, 0, 0, 0, 0, 0 );
            free( name );
        }

        void redraw()
        {
            getWindowSize();
            drawSolidBar();
            drawDividers();
            drawPercentage();
            flush();
        }

        void getWindowSize()
        {
            XWindowAttributes attributes;
            XGetWindowAttributes( _display, _window, &attributes );
            _width = attributes.width;
            _height = attributes.height;
        }

        void drawSolidBar()
        {
            unsigned barWidth = (_width * _testsDone) / _numTotalTests;

            XSetForeground( _display, _gc, _barColor );
            XFillRectangle( _display, _window, _gc, 0, 0, barWidth, _height );

            XSetForeground( _display, _gc, _background );
            XFillRectangle( _display, _window, _gc, barWidth, 0, _width + 1 - barWidth, _height );
        }

        void drawDividers()
        {
            if(_width / _numTotalTests < 5)
                return;
            for ( unsigned i = 1; i < _testsDone; ++ i ) {
                int x = (_width * i) / _numTotalTests;
                XDrawLine( _display, _window, _gc, x, 0, x, _height);
            }
        }

        void drawPercentage()
        {
            XSetForeground( _display, _gc, _foreground );

            char str[sizeof("1000000000 of ") + sizeof(_strTotalTests) + sizeof(" (100%)")];
            sprintf( str, "%u of %s (%u%%)", _testsDone, _strTotalTests, (_testsDone * 100) / _numTotalTests );
            unsigned len = strlen( str );

            int textWidth = XTextWidth( _fontInfo, str, len );

            XDrawString( _display, _window, _gc,
                         (_width - textWidth) / 2, ((_height + _textHeight) / 2) - _textDescent,
                         str, len );
        }

        void flush()
        {
            XFlush( _display );
        }

        void destroyGc()
        {
            XFreeGC( _display, _gc );
        }

        void destroyWindow()
        {
            XDestroyWindow( _display, _window );
        }

        void closeDisplay()
        {
            XCloseDisplay( _display );
        }
    };
}

#endif //__cxxtest__X11Gui_h__
