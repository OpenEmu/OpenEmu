#ifndef __cxxtest__QtGui_h__
#define __cxxtest__QtGui_h__

//
// The QtGui displays a simple progress bar using the Qt Toolkit.  It
// has been tested with versions 2.x and 3.x.
//
// Apart from normal Qt command-line arguments, it accepts the following options:
//   -minimized    Start minimized, pop up on error
//   -keep         Don't close the window at the end
//   -title TITLE  Set the window caption
//
// If both are -minimized and -keep specified, GUI will only keep the
// window if it's in focus.
//

#include <cxxtest/Gui.h>

#include <qapplication.h>
#include <qglobal.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qpixmap.h>
#include <qprogressbar.h>
#include <qstatusbar.h>

namespace CxxTest
{
    class QtGui : public GuiListener
    {
    public:
        void enterGui( int &argc, char **argv )
        {
            parseCommandLine( argc, argv );
            createApplication( argc, argv );
        }

        void enterWorld( const WorldDescription &wd )
        {
            createWindow( wd );
            processEvents();
        }

        void guiEnterSuite( const char *suiteName )
        {
            showSuiteName( suiteName );
        }

        void guiEnterTest( const char *suiteName, const char *testName )
        {
            setCaption( suiteName, testName );
            advanceProgressBar();
            showTestName( testName );
            showTestsDone( _progressBar->progress() );
            processEvents();
        }

        void yellowBar()
        {
            setColor( 255, 255, 0 );
            setIcon( QMessageBox::Warning );
            getTotalTests();
            processEvents();
        }

        void redBar()
        {
            if ( _startMinimized && _mainWindow->isMinimized() )
                showNormal();
            setColor( 255, 0, 0 );
            setIcon( QMessageBox::Critical );
            getTotalTests();
            processEvents();
        }

        void leaveGui()
        {
            if ( keep() ) {
                showSummary();
                _application->exec();
            }
            else
                _mainWindow->close( true );
        }

    private:
        QString _title;
        bool _startMinimized, _keep;
        unsigned _numTotalTests;
        QString _strTotalTests;
        QApplication *_application;
        QWidget *_mainWindow;
        QVBoxLayout *_layout;
        QProgressBar *_progressBar;
        QStatusBar *_statusBar;
        QLabel *_suiteName, *_testName, *_testsDone;

        void parseCommandLine( int argc, char **argv )
        {
            _startMinimized = _keep = false;
            _title = argv[0];

            for ( int i = 1; i < argc; ++ i ) {
                QString arg( argv[i] );
                if ( arg == "-minimized" )
                    _startMinimized = true;
                else if ( arg == "-keep" )
                    _keep = true;
                else if ( arg == "-title" && (i + 1 < argc) )
                    _title = argv[++i];
            }
        }

        void createApplication( int &argc, char **argv )
        {
            _application = new QApplication( argc, argv );
        }

        void createWindow( const WorldDescription &wd )
        {
            getTotalTests( wd );
            createMainWindow();
            createProgressBar();
            createStatusBar();
            setMainWidget();
            if ( _startMinimized )
                showMinimized();
            else
                showNormal();
        }

        void getTotalTests()
        {
            getTotalTests( tracker().world() );
        }

        void getTotalTests( const WorldDescription &wd )
        {
            _numTotalTests = wd.numTotalTests();
            char s[WorldDescription::MAX_STRLEN_TOTAL_TESTS];
            _strTotalTests = wd.strTotalTests( s );
        }

        void createMainWindow()
        {
            _mainWindow = new QWidget();
            _layout = new QVBoxLayout( _mainWindow );
        }

        void createProgressBar()
        {
            _layout->addWidget( _progressBar = new QProgressBar( _numTotalTests, _mainWindow ) );
            _progressBar->setProgress( 0 );
            setColor( 0, 255, 0 );
            setIcon( QMessageBox::Information );
        }

        void createStatusBar()
        {
            _layout->addWidget( _statusBar = new QStatusBar( _mainWindow ) );
            _statusBar->addWidget( _suiteName = new QLabel( _statusBar ), 2 );
            _statusBar->addWidget( _testName = new QLabel( _statusBar ), 4 );
            _statusBar->addWidget( _testsDone = new QLabel( _statusBar ), 1 );
        }

        void setMainWidget()
        {
            _application->setMainWidget( _mainWindow );
        }

        void showMinimized()
        {
            _mainWindow->showMinimized();
        }

        void showNormal()
        {
            _mainWindow->showNormal();
            centerWindow();
        }

        void setCaption( const QString &suiteName, const QString &testName )
        {
            _mainWindow->setCaption( _title + " - " + suiteName + "::" + testName + "()" );
        }

        void showSuiteName( const QString &suiteName )
        {
            _suiteName->setText( "class " + suiteName );
        }

        void advanceProgressBar()
        {
            _progressBar->setProgress( _progressBar->progress() + 1 );
        }

        void showTestName( const QString &testName )
        {
            _testName->setText( testName + "()" );
        }

        void showTestsDone( unsigned testsDone )
        {
            _testsDone->setText( asString( testsDone ) + " of " + _strTotalTests );
        }

        static QString asString( unsigned n )
        {
            return QString::number( n );
        }

        void setColor( int r, int g, int b )
        {
            QPalette palette = _progressBar->palette();
            palette.setColor( QColorGroup::Highlight, QColor( r, g, b ) );
            _progressBar->setPalette( palette );
        }

        void setIcon( QMessageBox::Icon icon )
        {
#if QT_VERSION >= 0x030000
            _mainWindow->setIcon( QMessageBox::standardIcon( icon ) );
#else // Qt version < 3.0.0
            _mainWindow->setIcon( QMessageBox::standardIcon( icon, QApplication::style().guiStyle() ) );
#endif // QT_VERSION
        }

        void processEvents()
        {
            _application->processEvents();
        }

        void centerWindow()
        {
            QWidget *desktop = QApplication::desktop();
            int xCenter = desktop->x() + (desktop->width() / 2);
            int yCenter = desktop->y() + (desktop->height() / 2);

            int windowWidth = (desktop->width() * 4) / 5;
            int windowHeight = _mainWindow->height();
            _mainWindow->setGeometry( xCenter - (windowWidth / 2), yCenter - (windowHeight / 2), windowWidth, windowHeight );
        }

        bool keep()
        {
            if ( !_keep )
                return false;
            if ( !_startMinimized )
                return true;
            return (_mainWindow == _application->activeWindow());
        }

        void showSummary()
        {
            QString summary = _strTotalTests + (_numTotalTests == 1 ? " test" : " tests");
            if ( tracker().failedTests() )
                summary = "Failed " + asString( tracker().failedTests() ) + " of " + summary;
            else
                summary = summary + " passed";

            _mainWindow->setCaption( _title + " - " + summary );

            _statusBar->removeWidget( _suiteName );
            _statusBar->removeWidget( _testName );
            _testsDone->setText( summary );
        }
    };
}

#endif // __cxxtest__QtGui_h__
