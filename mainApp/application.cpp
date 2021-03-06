/****************************************************************************
**
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the
** GNU General Public Liceense (GPL) version 3.0
** which accompanies this distribution, and is
** available at http://www.gnu.org/licenses/gpl.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "application.h"

#include <QDir>
#include <iostream>
#include <QMessageBox>

#include "lpub_preferences.h"
#include "lpub.h"
#include "resolution.h"
#include "updatecheck.h"

#include "QsLogDest.h"

#ifdef Q_OS_WIN

  #include <stdio.h>
  #include <fcntl.h>
  #include <io.h>
  #include <fstream>

  void Application::RedirectIOToConsole()
  {
    // Attach to the existing console of the parent process
    m_allocated_console = !AttachConsole( ATTACH_PARENT_PROCESS );

    if (m_allocated_console) {
      // maximum number of lines the output console should have
      static const WORD MAX_CONSOLE_LINES = 1000;

      // Allocate the new console.
      AllocConsole();

      // set the screen buffer to be big enough to let us scroll text
      GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                                 &ConsoleInfo);
      ConsoleInfo.dwSize.Y = MAX_CONSOLE_LINES;
      SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),
                                 ConsoleInfo.dwSize);
    }

    // Get STDOUT handle
    ConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    int SystemOutput = _open_osfhandle(intptr_t(ConsoleOutput), _O_TEXT);
    COutputHandle = _fdopen(SystemOutput, "w");

    // Get STDERR handle
    ConsoleError = GetStdHandle(STD_ERROR_HANDLE);
    int SystemError = _open_osfhandle(intptr_t(ConsoleError), _O_TEXT);
    CErrorHandle = _fdopen(SystemError, "w");

    // Get STDIN handle
    ConsoleInput = GetStdHandle(STD_INPUT_HANDLE);
    int SystemInput = _open_osfhandle(intptr_t(ConsoleInput), _O_TEXT);
    CInputHandle = _fdopen(SystemInput, "r");

    //retrieve and save the current attributes
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                               &ConsoleInfo);
    m_currentConsoleAttr = ConsoleInfo.wAttributes;
    FlushConsoleInputBuffer(ConsoleOutput);

    //change the attribute to what you like
    SetConsoleTextAttribute (
                ConsoleOutput,
                FOREGROUND_RED |
                FOREGROUND_GREEN);

    //make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
    std::ios_base::sync_with_stdio(true);

    // Redirect the CRT standard input, output, and error handles to the console
    freopen_s(&CInputHandle, "CONIN$", "r", stdin);
    freopen_s(&COutputHandle, "CONOUT$", "w", stdout);
    freopen_s(&CErrorHandle, "CONOUT$", "w", stderr);

    //Clear the error state for each of the C++ standard stream objects. We need to do this, as
    //attempts to access the standard streams before they refer to a valid target will cause the
    //iostream objects to enter an error state. In versions of Visual Studio after 2005, this seems
    //to always occur during startup regardless of whether anything has been read from or written to
    //the console or not.
    std::wcout.clear();
    std::cout.clear();
    std::wcerr.clear();
    std::cerr.clear();
    std::wcin.clear();
    std::cin.clear();
  }
#endif

// Initializes the Application instance as null
Application* Application::m_instance = nullptr;

Application::Application(int &argc, char **argv)
  : m_application(argc, argv)
{
#ifdef Q_OS_WIN
    RedirectIOToConsole();
#endif
  m_instance = this;
  m_console_mode = false;
  m_print_output = false;

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
  m_application.setAttribute(Qt::AA_UseDesktopOpenGL);
#endif

  QCoreApplication::setOrganizationName(VER_COMPANYNAME_STR);
  QCoreApplication::setApplicationVersion(VER_PRODUCTVERSION_STR);

  connect(this, SIGNAL(splashMsgSig(QString)), this, SLOT(splashMsg(QString)));

//qDebug() << "QStyleFactory valid styles:" << QStyleFactory::keys();
#ifdef Q_OS_MAC
  QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus);
  m_application.setStyle(QStyleFactory::create("macintosh"));
#endif

}

Application* Application::instance()
{
  return m_instance;
}

QStringList Application::arguments()
{
  return m_application.arguments();
}

bool Application::modeGUI()
{
  return ! m_console_mode;
}

void Application::initialize()
{
  // process arguments
  bool headerPrinted = false;
  QStringList ListArgs, Arguments = arguments();
  const int NumArguments = Arguments.size();
  for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
    ListArgs << Arguments[ArgIdx];
  for (int ArgIdx = 1; ArgIdx < NumArguments; ArgIdx++)
  {
    const QString& Param = Arguments[ArgIdx];
    if (Param[0] != '-') {
      if (!m_console_mode && QFileInfo(Param).exists())
        m_commandline_file = Param;
    }
    else if (! headerPrinted)
    {
      m_console_mode = true;
      fprintf(stdout, "\n%s for %s\n",VER_PRODUCTNAME_STR,VER_COMPILED_FOR);
      fprintf(stdout, "==========================\n");
      fprintf(stdout, "Arguments: %s\n",QString(ListArgs.join(" ")).toLatin1().constData());
      fflush(stdout);
      headerPrinted = true;
    }
    if (Param == QLatin1String("-v") || Param == QLatin1String("--version"))
    {
      m_console_mode = true;
      m_print_output = true;
      fprintf(stdout, "%s, Version %s, Revision %s, Build %s, ShaHash %s\n",VER_PRODUCTNAME_STR,VER_PRODUCTVERSION_STR,VER_REVISION_STR,VER_BUILD_STR,VER_SHA_HASH_STR);
      fprintf(stdout, "Compiled on " __DATE__ "\n");
      return;
    }
    else if (Param == QLatin1String("-vv") || Param == QLatin1String("--viewer-version"))
    {
      m_console_mode = true;
      m_print_output = true;
      fprintf(stdout, "3DViewer - by LeoCAD, Version %s, ShaHash %s\n",LC_VERSION_TEXT,LC_VERSION_BUILD);
      fprintf(stdout, "Compiled " __DATE__ "\n");
      return;
    }
    else if (Param == QLatin1String("-?") || Param == QLatin1String("--help"))
    {
      m_console_mode = true;
      m_print_output = true;
      fprintf(stdout, "Usage: %s [options] [ldraw file]\n",qApp->applicationName().toLatin1().constData());
      fprintf(stdout, "  [file]:\n");
      fprintf(stdout, "  Absolute file path and name with .ldr, .mpd or .dat extension.");
      fprintf(stdout, "  [options]:\n");
      fprintf(stdout, "  -pf, --process-file: Process ldraw file and generate images in png format..\n");
      fprintf(stdout, "  -pe, --process-export: Export instruction document or images. Used with export-option. Default is pdf document.\n");
      fprintf(stdout, "  -x, --clear-cache: Turn of reset the LDraw file and image caches. Used with export-option change. Default is off.\n");
      fprintf(stdout, "  -r, --range <page range>: Set page range - e.g. 1,2,9,10-42. Default is all pages.\n");
      fprintf(stdout, "  -o, --export-option <option>: Set output format pdf, png, jpeg or bmp. Used with process-export. Default is pdf.\n");
      fprintf(stdout, "  -f, --pdf-output-file <path>: Designate the pdf document save file using absolute path.\n");
      fprintf(stdout, "  -d, --image-output-directory <directory>: Designate the png, jpg or bmp save folder using absolute path.\n");
      fprintf(stdout, "  -p, --preferred-renderer <renderer>: Set renderer ldglite, ldview, ldview-sc or povray. Default is ldview.\n ");
//      fprintf(stdout, "  -l, --libpath <path>: Set the Parts Library location to path.\n");
//      fprintf(stdout, "  -i, --image <outfile.ext>: Save a picture in the format specified by ext.\n");
//      fprintf(stdout, "  -w, --width <width>: Set the picture width.\n");
//      fprintf(stdout, "  -h, --height <height>: Set the picture height.\n");
//      fprintf(stdout, "  -f, --from <time>: Set the first step to save pictures.\n");
//      fprintf(stdout, "  -t, --to <time>: Set the last step to save pictures.\n");
//      fprintf(stdout, "  -s, --submodel <submodel>: Set the active submodel.\n");
//      fprintf(stdout, "  -c, --camera <camera>: Set the active camera.\n");
//      fprintf(stdout, "  --viewpoint <front|back|left|right|top|bottom|home>: Set the viewpoint.\n");
//      fprintf(stdout, "  --camera-angles <latitude> <longitude>: Set the camera angles in degrees around the model.\n");
//      fprintf(stdout, "  --orthographic: Make the view orthographic.\n");
//      fprintf(stdout, "  --highlight: Highlight pieces in the steps they appear.\n");
//      fprintf(stdout, "  -obj, --export-wavefront <outfile.obj>: Export the model to Wavefront OBJ format.\n");
//      fprintf(stdout, "  -3ds, --export-3ds <outfile.3ds>: Export the model to 3D Studio 3DS format.\n");
//      fprintf(stdout, "  -dae, --export-collada <outfile.dae>: Export the model to COLLADA DAE format.\n");
//      fprintf(stdout, "  -html, --export-html <folder>: Create an HTML page for the model.\n");
//      fprintf(stdout, "  --html-parts-width <width>: Set the HTML part pictures width.\n");
//      fprintf(stdout, "  --html-parts-height <height>: Set the HTML part pictures height.\n");
      fprintf(stdout, "  -v, --version: Output LPub3D version information and exit.\n");
      fprintf(stdout, "  -vv, --viewer-version: Output 3DViewer - by LeoCAD version information and exit.\n");
      fprintf(stdout, "  -?, --help: Display this help message and exit.\n");
      fprintf(stdout, "  \n");
      return;
    }
  }

  // initialize directories
  Preferences::lpubPreferences();

  // initialize the logger
  Preferences::loggingPreferences();

  using namespace QsLogging;

  Logger& logger = Logger::instance();

  // set default log options
  if (Preferences::logging)
  {
    if (Preferences::logLevels)
    {

      logger.setLoggingLevels();
      logger.setDebugLevel( Preferences::debugLevel);
      logger.setTraceLevel( Preferences::traceLevel);
      logger.setNoticeLevel(Preferences::noticeLevel);
      logger.setInfoLevel(  Preferences::infoLevel);
      logger.setStatusLevel(Preferences::statusLevel);
      logger.setErrorLevel( Preferences::errorLevel);
      logger.setFatalLevel( Preferences::fatalLevel);

    }
    else if (Preferences::logLevel)
    {

      bool ok;
      Level logLevel = logger.fromLevelString(Preferences::loggingLevel,&ok);
      if (!ok)
      {
        QString Message = QMessageBox::tr("Failed to set log level %1.\n"
                                          "Logging is off - level set to OffLevel")
            .arg(Preferences::loggingLevel);
        if (modeGUI())
          QMessageBox::critical(nullptr,QMessageBox::tr(VER_PRODUCTNAME_STR), Message);
        else
          fprintf(stderr, "%s", Message.toLatin1().constData());
      }
      logger.setLoggingLevel(logLevel);
    }

    logger.setIncludeLogLevel(    Preferences::includeLogLevel);
    logger.setIncludeTimestamp(   Preferences::includeTimestamp);
    logger.setIncludeLineNumber(  Preferences::includeLineNumber);
    logger.setIncludeFileName(    Preferences::includeFileName);
    logger.setIncludeFunctionInfo(Preferences::includeFunction);

    logger.setColorizeFunctionInfo(true);
    logger.setColorizeOutput(true);

    // Create log destinations
    DestinationPtr fileDestination(DestinationFactory::MakeFileDestination(
                                     Preferences::logPath, EnableLogRotation, MaxSizeBytes(5000000), MaxOldLogCount(5)));
    DestinationPtr debugDestination(
          DestinationFactory::MakeDebugOutputDestination());

    // set log destinations on the logger
    logger.addDestination(debugDestination);
    logger.addDestination(fileDestination);

    // logging examples
    bool showLogExamples = false;
    if (showLogExamples)
    {
      logStatus() << "Uh-oh! - this level is not displayed in the console only the log";
      logInfo()   << "LPub3D started";
      logInfo()   << "Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
      logTrace()  << "Here's a" << QString("trace") << "message";
      logDebug()  << "Here's a" << static_cast<int>(QsLogging::DebugLevel) << "message";
      logNotice() << "Here's a" << QString("Notice") << "message";
      qDebug()    << "This message won't be picked up by the logger";
      logError()  << "An error has occurred";
      qWarning()  << "Neither will this one";
      logFatal()  << "Fatal error!";

      Level level = logger.loggingLevel();
      logger.setLoggingLevel(QsLogging::OffLevel);
      for (int i = 0;i < 10;++i)
      {
        logError() << QString::fromUtf8("this message should not be visible");
      }
      logger.setLoggingLevel(level);
    } // end init logging

  } else {
    logger.setLoggingLevel(OffLevel);
  }

  logInfo() << QString("Initializing application...");

  // splash
  if (modeGUI())
  {

    QPixmap pixmap(":/resources/LPub512Splash.png");
    splash = new QSplashScreen(pixmap);

    QFont splashFont;
    splashFont.setFamily("Arial");
    splashFont.setPixelSize(16);
    splashFont.setStretch(130);

    splash->setFont(splashFont);
    splash->show();
  }

  emit splashMsgSig("5% - Initializing application...");

  // Preferences
  Preferences::lpub3dLibPreferences(false);
  Preferences::ldrawPreferences(false);

  emit splashMsgSig("15% - Preferences loading...");

  Preferences::lpub3dUpdatePreferences();

  Preferences::unitsPreferences();
  Preferences::annotationPreferences();

  Preferences::fadestepPreferences();
  Preferences::highlightstepPreferences();
  Preferences::pliPreferences();

  // Resolution
  defaultResolutionType(Preferences::preferCentimeters);
  setResolution(150);  // DPI

  // Translator
  QTranslator Translator;
  Translator.load(QString("lpub_") + QLocale::system().name().section('_', 0, 0) + ".qm", ":../lc_lib/resources");
  m_application.installTranslator(&Translator);

  qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");

  QList<QPair<QString, bool>> LibraryPaths;

/* disable LibraryPaths - library paths managed by ldrawPreferences() in Gui
#if defined(Q_OS_WIN)
  lcSehInit();
  if (QDir(Preferences::lpub3dPath + "/extras").exists()) { // we have a portable distribution
    LibraryPaths += qMakePair(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/extras/complete.zip"), true);
  } else {
    LibraryPaths += qMakePair(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/data/complete.zip"), true);
  }
#endif
#ifdef Q_OS_LINUX
  LibraryPaths += qMakePair(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../share/lpub3d/complete.zip"), true);
#endif

#ifdef Q_OS_MAC
  LibraryPaths += qMakePair(QDir::cleanPath(QCoreApplication::applicationDirPath() + "/../../Contents/Resources/complete.zip"), true);
#endif

#ifdef LDRAW_LIBRARY_PATH
  LibraryPaths += qMakePair(QString::fromLatin1(LDRAW_LIBRARY_PATH), false);
#endif
*/

  setlocale(LC_NUMERIC, "C");

/* load sequence
* lcApplication::lcApplication()                         (gApplication)  [3DViewer]
* lcApplication::LoadDefaults                            (gApplication)
* Gui::gui()                                             (gui)           [LPub3D]
* lcApplication::Initialize()                            (gApplication->Initialize)
* Initialize::gMainWindow                                (gApplication->gMainWindow)
* Initialize::lcLoadDefaultKeyboardShortcuts()           (gApplication->Initialize)
* Initialize::lcLoadDefaultMouseShortcuts()              (gApplication->Initialize)
* Initialize::LoadPartsLibrary()                         (gApplication->Initialize)
* LoadPartsLibrary::ldsearchDirPreferences()             (gApplication->Initialize)
* ldsearchDirPreferences::setLDViewExtraSearchDirs()     (gApplication->Initialize) [LDView Renderer]
* Initialize::CreateWidgets()                            (gApplication->Initialize)
* gMainWindow::SetColorIndex()                           (gApplication->gMainWindow)
* gMainWindow::GetPartSelectionWidget()                  (gApplication->gMainWindow)
* Gui::initialize()                                      (gui->initialize)
* initialize::populateLdgLiteSearchDirs()                (gui->initialize)          [LDGLite Renderer]
* initialize::createActions                              (gui->initialize)
* initialize::createToolBars                             (gui->initialize)
* initialize::createStatusBar                            (gui->initialize)
* initialize::createDockWindows                          (gui->initialize)
* initialize::toggleLCStatusBar                          (gui->initialize)
*/

  emit splashMsgSig("20% - 3D Viewer window loading...");

  gApplication = new lcApplication();

  emit splashMsgSig(QString("30% - %1 GUI window loading...").arg(VER_PRODUCTNAME_STR));

  gui = new Gui();

  // Check if preferred renderer set and launch Preference dialogue if not to set Renderer
  gui->getRequireds();

  emit splashMsgSig(QString("40% - 3D Viewer initialization..."));

  if (gApplication->Initialize(LibraryPaths, gui)) {
    gui->initialize();
  } else {
    logError() << QString("Unable to initialize 3D Viewer.");
    throw InitException{};
  }
}

void Application::mainApp()
{
  if (m_print_output)
    return;

  emit splashMsgSig(QString("100% - %1 loaded.").arg(VER_PRODUCTNAME_STR));

  Preferences::setLPub3DLoaded();

  GetAvailableVersions();

  if (modeGUI())
  {
    splash->finish(gui);

    if (!m_commandline_file.isEmpty())
       emit gui->loadFileSig(m_commandline_file);

    gui->show();

#ifndef DISABLE_UPDATE_CHECK
    DoInitialUpdateCheck();
#endif
  }
}

int Application::run()
{
  int ExecReturn = EXIT_FAILURE;
  try
  {
    // Call the main function
    logInfo() << QString("Run: Starting application...");

    mainApp();

    if (modeGUI()) {
      logInfo() << QString("Run: Application started.");
      ExecReturn = m_application.exec();
    } else {
      logInfo() << QString("Run: Application started.");
      ExecReturn = gui->processCommandLine();
    }
#ifdef Q_OS_WIN
      if (m_allocated_console)
        Sleep(2000);
      SetConsoleTextAttribute (       //set the console to the original atrributes
                  ConsoleOutput,
                  m_currentConsoleAttr);
#endif
  }
  catch(const std::exception& ex)
  {
    logError() << QString("Run: %1\n%2").arg("LOG_ERROR").arg(ex.what());
  }
  catch(...)
  {
    logError() << QString("Run: An unhandled exception has been thrown.");
  }

  if (!m_print_output)
  {
    delete gMainWindow;
    gMainWindow = nullptr;

    delete gApplication;
    gApplication = nullptr;

    delete gui;
    gui = nullptr;
  }

  logInfo() << QString("Run: Application terminated with return code %1.").arg(ExecReturn);

  return ExecReturn;
}

// Implements the main function here.
ENTRY_POINT
