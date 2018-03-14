/****************************************************************************
**
** Copyright (C) 2007-2009 Kevin Clague. All rights reserved.
** Copyright (C) 2015 - 2018 Trevor SANDY. All rights reserved.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include <QDesktopServices>
#include <QSettings>
#include <QFileInfo>
#include <QProcess>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>
#include <QDate>
#include <JlCompress.h>
#include "lpub_preferences.h"
#include "ui_preferences.h"
#include "preferencesdialog.h"
#include "updatecheck.h"
#include "resolution.h"
#include "render.h"
#include "pli.h"
#include "version.h"
#include "name.h"
#include "application.h"
//**3D
#include "lc_profile.h"
//**

Preferences preferences;
QDate date = QDate::currentDate();

QString Preferences::ldrawPath                  = "";
QString Preferences::altLDConfigPath            = "";
QString Preferences::lpub3dLibFile              = "";
QString Preferences::lgeoPath;
QString Preferences::lpub3dPath                 = ".";
QString Preferences::lpub3dExtrasResourcePath   = ".";
QString Preferences::lpub3dDocsResourcePath     = ".";
QString Preferences::lpubDataPath               = ".";
QString Preferences::lpubExtrasPath             = ".";
QString Preferences::ldgliteExe;
QString Preferences::ldviewExe;
QString Preferences::povrayConf;
QString Preferences::povrayIni;
QString Preferences::ldgliteIni;
QString Preferences::ldviewIni;
QString Preferences::ldviewPOVIni;
QString Preferences::povrayIniPath;
QString Preferences::povrayIncPath;
QString Preferences::povrayScenePath;
QString Preferences::povrayExe;
QString Preferences::preferredRenderer;
QString Preferences::pliFile;
QString Preferences::titleAnnotationsFile;
QString Preferences::freeformAnnotationsFile;
QString Preferences::fadeStepColor              = "Very_Light_Bluish_Gray";
QString Preferences::pliSubstitutePartsFile;
QString Preferences::fadeStepColorPartsFile;
QString Preferences::excludedPartsFile;
//page attributes dynamic
QString Preferences::defaultAuthor;
QString Preferences::defaultURL;
QString Preferences::defaultEmail;
QString Preferences::documentLogoFile;
QString Preferences::publishDescription;
QString Preferences::ldrawiniFile;
QString Preferences::moduleVersion               = qApp->applicationVersion();
QString Preferences::availableVersions;
QString Preferences::ldgliteSearchDirs;
QStringList Preferences::ldSearchDirs;
QStringList Preferences::ldgliteParms;
//Static page attributes
QString Preferences::disclaimer                 = QString(QObject::trUtf8("LEGO® is a registered trademark of the LEGO Group, \n"
                                                                          "which does not sponsor, endorse, or authorize these \n"
                                                                          "instructions or the model they depict."));
QString Preferences::copyright                  = QString(QObject::trUtf8("Copyright © %1").arg(date.toString("yyyy")));
QString Preferences::plugImage                  = QString(":/resources/LPub64.png");
QString Preferences::plug                       = QString(QObject::trUtf8("Instructions configured and generated using %1 %2 \n Download %1 at %3")
                                                          .arg(QString::fromLatin1(VER_PRODUCTNAME_STR),
                                                               QString::fromLatin1(VER_FILEVERSION_STR),
                                                               QString::fromLatin1(VER_COMPANYDOMAIN_STR)));
QString Preferences::logPath;
QString Preferences::loggingLevel;               // string
QString Preferences::dataLocation;

bool    Preferences::lgeoStlLib                 = false;
bool    Preferences::lpub3dLoaded               = false;
bool    Preferences::enableDocumentLogo         = false;
bool    Preferences::enableLDViewSingleCall     = true;
bool    Preferences::useLDViewSingleCall        = false;
bool    Preferences::displayAllAttributes       = false;
bool    Preferences::generateCoverPages         = false;
bool    Preferences::printDocumentTOC           = false;

bool    Preferences::includeLogLevel            = false;
bool    Preferences::includeTimestamp           = false;
bool    Preferences::includeLineNumber          = false;
bool    Preferences::includeFileName            = false;
bool    Preferences::includeFunction            = false;

bool    Preferences::ignoreMixedPageSizesMsg    = false;

bool    Preferences::debugLevel                 = false;
bool    Preferences::traceLevel                 = false;
bool    Preferences::noticeLevel                = false;
bool    Preferences::infoLevel                  = false;
bool    Preferences::statusLevel                = false;
bool    Preferences::errorLevel                 = false;
bool    Preferences::fatalLevel                 = false;

bool    Preferences::includeAllLogAttributes    = false;
bool    Preferences::allLogLevels               = false;

bool    Preferences::logLevel                   = false;
bool    Preferences::logging                    = false;   // logging on/off offLevel (grp box)
bool    Preferences::logLevels                  = false;   // individual logging levels (grp box)

bool    Preferences::enableFadeStep             = false;
bool    Preferences::preferCentimeters          = true;
bool    Preferences::showAllNotifications       = true;
bool    Preferences::showUpdateNotifications    = true;
bool    Preferences::enableDownloader           = true;
bool    Preferences::ldrawiniFound              = false;
bool    Preferences::fadeStepSettingChanged     = false;
bool    Preferences::fadeStepColorChanged       = false;
bool    Preferences::portableDistribution       = false;
bool    Preferences::povrayDisplay              = false;
bool    Preferences::isAppImagePayload          = false;
bool    Preferences::modeGUI                    = true;

int     Preferences::checkUpdateFrequency       = 0;        //0=Never,1=Daily,2=Weekly,3=Monthly

int     Preferences::pageHeight                 = 800;
int     Preferences::pageWidth                  = 600;
int     Preferences::rendererTimeout            = 6;        // measured in seconds

Preferences::Preferences()
{
}

void Preferences::loggingPreferences()
{
    // define log path
    QString lpubDataPath = Preferences::lpubDataPath;
    QDir logDir(lpubDataPath+"/logs");
    if(!QDir(logDir).exists())
        logDir.mkpath(".");
    Preferences::logPath = QDir(logDir).filePath(QString("%1Log.txt").arg(VER_PRODUCTNAME_STR));

    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"))) {
        QVariant uValue(true);
        includeLogLevel = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"),uValue);
    } else {
        includeLogLevel = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeLogLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"))) {
        QVariant uValue(false);
        includeTimestamp = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"),uValue);
    } else {
        includeTimestamp = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeTimestamp")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeFileName"))) {
        QVariant uValue(false);
        includeFileName = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFileName"),uValue);
    } else {
        includeFileName = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeFileName")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"))) {
        QVariant uValue(true);
        includeLineNumber = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"),uValue);
    } else {
        includeLineNumber = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeLineNumber")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeFunction"))) {
        QVariant uValue(true);
        includeFunction = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFunction"),uValue);
    } else {
        includeFunction = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeFunction")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"))) {
        QVariant uValue(false);
        includeAllLogAttributes = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"),uValue);
    } else {
        includeAllLogAttributes = Settings.value(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"Logging "))) {
        QVariant uValue(true);
        logging = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"Logging "),uValue);
    } else {
        logging = Settings.value(QString("%1/%2").arg(LOGGING,"Logging ")).toBool();
    }

    // log levels combo
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LoggingLevel"))) {
        QVariant uValue("STATUS");
        loggingLevel = "STATUS";
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LoggingLevel"),uValue);
    } else {
        loggingLevel = Settings.value(QString("%1/%2").arg(LOGGING,"LoggingLevel")).toString();
    }
    // log levels group box
    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LogLevel"))) {
        QVariant uValue(true);
        logLevel = true;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevel"),uValue);
    } else {
        logLevel = Settings.value(QString("%1/%2").arg(LOGGING,"LogLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"LogLevels"))) {
        QVariant uValue(false);
        logLevels = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevels"),uValue);
    } else {
        logLevels = Settings.value(QString("%1/%2").arg(LOGGING,"LogLevels")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"DebugLevel"))) {
        QVariant uValue(false);
        debugLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"DebugLevel"),uValue);
    } else {
        debugLevel = Settings.value(QString("%1/%2").arg(LOGGING,"DebugLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"TraceLevel"))) {
        QVariant uValue(false);
        traceLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"TraceLevel"),uValue);
    } else {
        traceLevel = Settings.value(QString("%1/%2").arg(LOGGING,"TraceLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"NoticeLevel"))) {
        QVariant uValue(false);
        noticeLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"NoticeLevel"),uValue);
    } else {
        noticeLevel = Settings.value(QString("%1/%2").arg(LOGGING,"NoticeLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"InfoLevel"))) {
        QVariant uValue(false);
        infoLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"InfoLevel"),uValue);
    } else {
        infoLevel = Settings.value(QString("%1/%2").arg(LOGGING,"InfoLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"StatusLevel"))) {
        QVariant uValue(false);
        statusLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"StatusLevel"),uValue);
    } else {
        statusLevel = Settings.value(QString("%1/%2").arg(LOGGING,"StatusLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"ErrorLevel"))) {
        QVariant uValue(false);
        errorLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"ErrorLevel"),uValue);
    } else {
        errorLevel = Settings.value(QString("%1/%2").arg(LOGGING,"ErrorLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"FatalLevel"))) {
        QVariant uValue(false);
        fatalLevel = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"FatalLevel"),uValue);
    } else {
        fatalLevel = Settings.value(QString("%1/%2").arg(LOGGING,"FatalLevel")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(LOGGING,"AllLogLevels"))) {
        QVariant uValue(false);
        allLogLevels = false;
        Settings.setValue(QString("%1/%2").arg(LOGGING,"AllLogLevels"),uValue);
    } else {
        allLogLevels = Settings.value(QString("%1/%2").arg(LOGGING,"AllLogLevels")).toBool();
    }

}

void Preferences::lpubPreferences()
{
    modeGUI = Application::instance()->modeGUI();
    QDir cwd(QCoreApplication::applicationDirPath());

#ifdef Q_OS_MAC

    //qDebug() << qPrintable(QString("macOS Binary Directory (%1), AbsPath (%2)").arg(cwd.dirName()).arg(cwd.absolutePath()));
    qDebug() << qPrintable(QString("macOS Binary Directory..(%1)").arg(cwd.dirName()));
    if (cwd.dirName() == "MacOS") {   // MacOS/LPub3D   (app bundle executable)
        cwd.cdUp();                   // Contents/      (app bundle contents)
        cwd.cdUp();                   // LPub3D.app/    (app bundle)
        cwd.cdUp();                   // Applications/  (app bundle installation path root)
    }
    //qDebug() << qPrintable(QString("macOS Base Directory (%1), AbsPath (%2)").arg(cwd.dirName()).arg(cwd.absolutePath()));
    qDebug() << qPrintable(QString("macOS Base Directory....(%1)").arg(cwd.dirName()));

    lpub3dExtrasResourcePath = QString("LPub3D.app/Contents/Resources");
    lpub3dDocsResourcePath   = lpub3dExtrasResourcePath;

#elif defined Q_OS_LINUX

    // Check if running as AppImage payload
    // Check for the presence of $ORIGIN/../AppRun *or* $ORIGIN/../../AppRun
    // (can be a file or a symlink) (with $ORIGIN being the application itself)
    QFileInfo appRunInfo("../AppRun");
    if (appRunInfo.exists() && (appRunInfo.isFile() || appRunInfo.isSymLink()))
        isAppImagePayload = true;
    if (! isAppImagePayload) {
        appRunInfo.setFile("../../AppRun");
        if (appRunInfo.exists() && (appRunInfo.isFile() || appRunInfo.isSymLink()))
            isAppImagePayload = true;
    }
#ifdef X11_BINARY_BUILD                                               // Standard User Rights Install
    lpub3dDocsResourcePath   = QString("doc");
#else                                                                 // Elevated User Rights Install
    lpub3dDocsResourcePath   = QString("../share/doc/" VER_APPNAME);
    lpub3dExtrasResourcePath = QString("../share/" VER_APPNAME);      // Elevated User Rights Install

    //qDebug() << QString("lpub3dExtrasResourcePath (%1)").arg(lpub3dExtrasResourcePath);
#endif
    //qDebug() << QString("lpub3dDocsResourcePath (%1)").arg(lpub3dDocsResourcePath);

#elif defined Q_OS_WIN

    lpub3dDocsResourcePath   = QString("docs");

#endif

    lpub3dPath = cwd.absolutePath();

#ifdef Q_OS_WIN //... Windows portable or installed

    if (QDir(lpub3dPath + "/extras").exists()) { // we have a portable distribution

        portableDistribution = true;

        bool programFolder = QCoreApplication::applicationDirPath().contains("Program Files") ||
                QCoreApplication::applicationDirPath().contains("Program Files (x86)");

        if (programFolder) {                     // ...installed in Program Folder directory

            if (modeGUI) {
                // Get the application icon as a pixmap
                QPixmap _icon = QPixmap(":/icons/lpub96.png");
                QMessageBox box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (_icon);
                box.setTextFormat (Qt::RichText);
                box.setWindowTitle(QMessageBox::tr ("Installation"));
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                QString header  = "<b> " + QMessageBox::tr ("Data directory installation folder.") + "</b>";
                QString body = QMessageBox::tr ("Would you like to create a folder outside the Program Files / (x86) directory? \n"
                                                "If you choose No, the data directory will automatically be created in the user's AppData directory.");
                QString detail = QMessageBox::tr ("It looks like this installation is a portable or packaged (i.e. AIOI) distribution \n"
                                                  "of LPub3D installed under the system's Program Files/(x86) directory.\n\n"
                                                  "Updatable data will not be able to be written to unless you modify\n"
                                                  "user account access for this folder which is not recommended.\n\n"
                                                  "You should consider changing the installation folder or placing\n"
                                                  "the updatable data folder outside the Program Files/(x86) directory\n\n"
                                                  "Choose yes to continue and select a data folder outside Program Files/(x86).\n\n"
                                                  "If you choose No, the data directory will automatically be created in the user's AppData directory.");
                box.setText (header);
                box.setInformativeText (body);
                box.setDetailedText(detail);
                box.setStandardButtons (QMessageBox::No | QMessageBox::Yes);
                box.setDefaultButton   (QMessageBox::Yes);

                if (box.exec() != QMessageBox::Yes) {   // user choose not to create user data direcory outside program folder, so create automatically

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
                    QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                    lpubDataPath = dataPathList.first();
#endif
                } else {                                // capture user's choice for user data directory

                    QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                    lpubDataPath = dataPathList.first();
                    QString result = QFileDialog::getExistingDirectory(NULL,
                                                                       QFileDialog::tr("Select Directory"),
                                                                       lpubDataPath,
                                                                       QFileDialog::ShowDirsOnly |
                                                                       QFileDialog::DontResolveSymlinks);
                    if (! result.isEmpty()) {
                        lpubDataPath = QDir::toNativeSeparators(result);
                    }
                }

            } else {                                          // console mode create automatically

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
                QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
                lpubDataPath = dataPathList.first();
#endif
            }

        } else {                                 // ...installed outside Program Folder directory

            lpubDataPath = lpub3dPath;
        }


    } else {                                     // we have an installed distribution

        QSettings Settings;
        QString const LPub3DDataPathKey("LPub3DDataPath");

        if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DDataPathKey))) {

            lpubDataPath = Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DDataPathKey)).toString();

        } else {

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
            QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
            lpubDataPath = dataPathList.first();
#endif
            Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DDataPathKey), lpubDataPath);
        }
    }

#else  // ...Linux or OSX

#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
    QStringList dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
    lpubDataPath = dataPathList.first();
#endif

#endif

    qDebug() << qPrintable(QString("LPub3D Root Path........(%1)").arg(lpub3dPath));
    qDebug() << qPrintable(QString("LPub3D User Data Path...(%1)").arg(lpubDataPath));

    QDir extrasDir(lpubDataPath + "/extras");
    if(!QDir(extrasDir).exists())
        extrasDir.mkpath(".");

#ifdef Q_OS_WIN
    dataLocation = QString("%1").arg((portableDistribution ? "/extras/" : "/data/"));
#else
    dataLocation = QString("/%1/").arg(lpub3dExtrasResourcePath); // ../share/lpub3d [linux], LPub3D.app/Contents/Resources [macOS]
#endif

    QFileInfo paramFile;
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_FADESTEP_COLORPARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_FREEFOM_ANNOTATIONS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_EXTRAS_LDCONFIG_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PDFPRINT_IMAGE_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_MPD_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_PLI_SUBSTITUTE_PARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_TITLE_ANNOTATIONS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
    paramFile.setFile(QString("%1/%2").arg(extrasDir.absolutePath(), VER_EXCLUDED_PARTS_FILE));
    if (!paramFile.exists())
        QFile::copy(lpub3dPath + dataLocation + paramFile.fileName(), paramFile.absoluteFilePath());
}

void Preferences::lpub3dLibPreferences(bool force)
{
    emit Application::instance()->splashMsgSig("10% - Locate LDraw archive libraries...");

#ifdef Q_OS_WIN
    QString filter(QFileDialog::tr("Archive (*.zip *.bin);;All Files (*.*)"));
#else
    QString filter(QFileDialog::tr("All Files (*.*)"));
#endif

    QSettings Settings;
    QFileInfo validFile;
    QString const LPub3DLibKey("PartsLibrary");

    // Start by checking the registry value, if not exist set the user data path
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DLibKey))) {
        lpub3dLibFile = Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)).toString();
    } else {
        lpub3dLibFile = QString("%1/%2/%3").arg(lpubDataPath, "libraries", VER_LDRAW_OFFICIAL_ARCHIVE);
        Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);
    }

    // If we have a valid library archive file, update the 3DViewer's parts_library variable, else clear the registry value
    if (! lpub3dLibFile.isEmpty() && ! force) {
        validFile.setFile(lpub3dLibFile);

        if (validFile.exists()) {
            lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)).toString());
            return;
        }
        else {
            lpub3dLibFile.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
        }
    }

    // Request to open a dialogue to select library path
    if (! lpub3dLibFile.isEmpty() && force){

        QString result = QFileDialog::getOpenFileName(NULL,
                                                      QFileDialog::tr("Select LDraw Library Archive"),
                                                      lpub3dLibFile,
                                                      filter);
        if (! result.isEmpty())
            lpub3dLibFile = QDir::toNativeSeparators(result);
    }

    // Archive library exist, so set registry value
    if (! lpub3dLibFile.isEmpty()) {

        Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);

    } else {

        // lets go look for the archive files...

        validFile.setFile(lpub3dPath + dataLocation + VER_LDRAW_OFFICIAL_ARCHIVE);
        bool archivesExist = validFile.exists();

        // DEBUG DEBUG DEBUG
        //        QMessageBox::information(NULL,
        //                                 QMessageBox::tr("LPub3D"),
        //                                 QMessageBox::tr("lpub3dPath: (%1)\n\n"
        //                                                 "lpub3dExtrasResourcePath [dataLocation]: (%2)\n\n"
        //                                                 "ArchivesExist: (%3)\n\n"
        //                                                 "Full Archives Path: (%4)")
        //                                                 .arg(lpub3dPath)
        //                                                 .arg(lpub3dExtrasResourcePath)
        //                                                 .arg(archivesExist ? "Yes" : "No")
        //                                                 .arg(validFile.absoluteFilePath()));

        if (archivesExist) {  // This condition should always fire as archive files are deposited at installation

            emit Application::instance()->splashMsgSig("10% - Copying archive libraries...");

            QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
            if (!QDir(libraryDir).exists())
                libraryDir.mkpath(".");

            validFile.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_OFFICIAL_ARCHIVE));
            if (!validFile.exists())
                QFile::copy(lpub3dPath + dataLocation + validFile.fileName(), validFile.absoluteFilePath());

            lpub3dLibFile = validFile.absoluteFilePath();
            Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);

            validFile.setFile(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LPUB3D_UNOFFICIAL_ARCHIVE));
            if (!validFile.exists())
                QFile::copy(lpub3dPath + dataLocation + validFile.fileName(), validFile.absoluteFilePath());

        } else if (modeGUI) { // This condition should never fire - left over old code that offers to select or download the archive libraries (in case the user removes them)

#ifdef Q_OS_MAC
            if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                Application::instance()->splash->hide();
#endif
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(QMessageBox::tr ("Library Selection"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

            QAbstractButton* downloadButton = box.addButton(QMessageBox::tr("Download"),QMessageBox::YesRole);
            QAbstractButton* selectButton   = box.addButton(QMessageBox::tr("Select"),QMessageBox::YesRole);

            QString header = "<b> " + QMessageBox::tr ("No LDraw library archive defined!") + "</b>";
            QString body = QMessageBox::tr ("Note: The LDraw library archives are not provided and <u>must be downloaded</u> - or selected.\n"
                                            "Would you like to download or select the library archives?");
            QString detail = QMessageBox::tr ("You must select or create your LDraw library archive files.\n"
                                              "The location of your official archive file (complete.zip) should "
                                              "also have the unofficial archive file (lpub3dldrawunf.zip).\n"
                                              "LDraw library archive files can be copied, downloaded or selected to your '%1/%2/' folder now.")
                    .arg(lpubDataPath, "libraries");
            box.setText (header);
            box.setInformativeText (body);
            box.setDetailedText(detail);
            box.setStandardButtons (QMessageBox::Cancel);
            box.exec();

            if (box.clickedButton()==selectButton) {
                emit Application::instance()->splashMsgSig("10% - Selecting archive libraries...");

                QFileDialog dlgGetFileName(NULL,QFileDialog::tr("Select LDraw Library Archive "));
                dlgGetFileName.setDirectory(lpubDataPath);
                dlgGetFileName.setNameFilter(filter);

                if (dlgGetFileName.exec() == QFileDialog::Accepted) {
                    lpub3dLibFile = dlgGetFileName.selectedFiles().at(0);
                    Settings.setValue(QString("%1/%2").arg(SETTINGS, LPub3DLibKey), lpub3dLibFile);

                } else {

                    Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
                    body = QMessageBox::tr ("<u>Selection cancelled</u>.\n"
                                            "The application will terminate.");
                    box.removeButton(selectButton);
                    box.removeButton(downloadButton);
                    box.setStandardButtons (QMessageBox::Close);
                    box.setText(header);
                    box.setInformativeText(body);

                    box.exec();
                    lpub3dLibFile.clear();

                    exit(-1);
                }

            } else
                if (box.clickedButton()==downloadButton) {

#ifdef Q_OS_MAC
                    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
                        Application::instance()->splash->show();
#endif
                    emit Application::instance()->splashMsgSig("10% - Downloading archive libraries...");

                    QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
                    if (!QDir(libraryDir).exists())
                        libraryDir.mkpath(".");

                    UpdateCheck *libraryDownload;
                    QEventLoop  *wait = new QEventLoop();
                    libraryDownload  = new UpdateCheck(NULL, (void*)LDrawOfficialLibraryDirectDownload);
                    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
                    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
                    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), libraryDir.absolutePath());
                    wait->exec();
                    if (libraryDownload->getCancel()) {
                        body = QMessageBox::tr ("<u>Official LDraw archive download cancelled</u>.\n"
                                                "The application will terminate.");
                        box.removeButton(selectButton);
                        box.removeButton(downloadButton);
                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);
                        box.exec();

                        exit(-1);
                    }
                    libraryDownload  = new UpdateCheck(NULL, (void*)LDrawUnofficialLibraryDirectDownload);
                    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
                    wait->connect(libraryDownload, SIGNAL(cancel()),                          wait, SLOT(quit()));
                    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), libraryDir.absolutePath());
                    wait->exec();
                    if (libraryDownload->getCancel()) {
                        body = QMessageBox::tr ("<u>Unofficial LDraw archive download cancelled</u>.\n"
                                                "The application will continue; however, your LDraw archives\n"
                                                "will not contain unofficial LDraw parts or primitives.");
                        box.removeButton(selectButton);
                        box.removeButton(downloadButton);
                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);
                        box.exec();
                    }

                    // validate downloaded files
                    lpub3dLibFile = QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_OFFICIAL_ARCHIVE));
                    validFile.setFile(lpub3dLibFile);
                    if (!validFile.exists()) {

                        Settings.remove(QString("%1/%2").arg(SETTINGS, LPub3DLibKey));
                        validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
                        if (!validFile.exists()) {
                            body = QMessageBox::tr ("Required archive files\n%1\n%2\ndoes not exist.").arg(lpub3dLibFile, validFile.absoluteFilePath());
                        } else {
                            body = QMessageBox::tr ("Required archive file %1 does not exist.").arg(lpub3dLibFile);
                        }
                        body.append("\nThe application will terminate.");

                        box.setStandardButtons (QMessageBox::Close);
                        box.setText(header);
                        box.setInformativeText(body);

                        box.exec();
                        lpub3dLibFile.clear();

                        exit(-1);
                    }

                    // extract downloaded files
                    extractLDrawLib();

                } else {

                    exit(-1);

                }
#ifdef Q_OS_MAC
            if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
                Application::instance()->splash->show();
#endif
        } else {              // If we get here, inform the user that required archive libraries do not exsit (performing build check or they were probably removed)

            QString officialArchive = validFile.absoluteFilePath();
            validFile.setFile(lpub3dPath + dataLocation + VER_LPUB3D_UNOFFICIAL_ARCHIVE);
            if (!validFile.exists()) {
                fprintf(stderr, "Required archive files\n%s\n%s\ndoes not exist.\n", officialArchive.toLatin1().constData(), validFile.absoluteFilePath().toLatin1().constData());
            } else {
                fprintf(stderr, "Required archive file %s does not exist.\n", lpub3dLibFile.toLatin1().constData());
            }
            fprintf(stderr, "Launching " VER_APPLICATIONNAME_STR " in GUI mode offers a dialogue to download, and extract, archive libraries.\n");
        }
    }

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)))
        lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Settings.value(QString("%1/%2").arg(SETTINGS,LPub3DLibKey)).toString());
}

void Preferences::ldrawPreferences(bool force)
{
    emit Application::instance()->splashMsgSig("10% - Locate LDraw directory...");

    QSettings Settings;
    QString const ldrawKey("LDrawDir");
                                                // check registry
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,ldrawKey))) {
        ldrawPath = Settings.value(QString("%1/%2").arg(SETTINGS,ldrawKey)).toString();
    }

    if (! ldrawPath.isEmpty() && ! force) {
        QDir ldrawDir(ldrawPath);

        if (ldrawDir.exists()) {
            return;
        } else {
            ldrawPath.clear();
            Settings.remove(QString("%1/%2").arg(SETTINGS,ldrawKey));
        }
    }


    char* EnvPath = getenv("LDRAWDIR");        // check environment variable LDRAWDIR

    if (EnvPath && EnvPath[0])
         ldrawPath = QString(EnvPath);

    if (! ldrawPath.isEmpty() && ! force) {
        QDir ldrawDir(ldrawPath);

        if (ldrawDir.exists()) {
            Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawKey),ldrawPath);
            QString PartsLibrary = lcGetProfileString(LC_PROFILE_PARTS_LIBRARY);
            if (PartsLibrary.isEmpty())
                lcSetProfileString(LC_PROFILE_PARTS_LIBRARY, Settings.value(QString("%1/%2").arg(SETTINGS,ldrawKey)).toString());
            return;
        } else {
            ldrawPath.clear();
        }
    }

    if (ldrawPath.isEmpty() && ! force) {      // check home path

        QString homePath,userLocalDataPath,userDocumentsPath;
        QStringList dataPathList;
#if (QT_VERSION >= QT_VERSION_CHECK(5, 0, 0))
        dataPathList = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
        homePath = dataPathList.first();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 4, 0))
        dataPathList = QStandardPaths::standardLocations(QStandardPaths::AppLocalDataLocation);
#else
        dataPathList = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
#endif
        userLocalDataPath = dataPathList.first();
        dataPathList = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);
        userDocumentsPath = dataPathList.first();
#endif

#if defined Q_OS_WIN || defined Q_OS_MAC
        ldrawPath = QDir::toNativeSeparators(QString("%1/LDraw").arg(homePath));
#else
        ldrawPath = QDir::toNativeSeparators(QString("%1/ldraw").arg(homePath));
#endif

        if ( ! QDir(ldrawPath).exists()) {     // check user documents path

#if defined Q_OS_WIN || defined Q_OS_MAC
            ldrawPath = QDir::toNativeSeparators(QString("%1/LDraw").arg(userDocumentsPath));
#else
            ldrawPath = QDir::toNativeSeparators(QString("%1/ldraw").arg(userDocumentsPath));
#endif

            if ( ! QDir(ldrawPath).exists()) { // check system data path

                dataPathList = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
#if defined Q_OS_WIN || defined Q_OS_MAC
                ldrawPath = QDir::toNativeSeparators(QString("%1/LDraw").arg(dataPathList.at(1))); /* C:/ProgramData/LDraw, /Library/Application Support/LDraw */
#else
                ldrawPath = QDir::toNativeSeparators(QString("%1/ldraw").arg(dataPathList.at(2))); /* /usr/share/ldraw" */
#endif

                if ( ! QDir(ldrawPath).exists()) { // check user data path

#if defined Q_OS_WIN || defined Q_OS_MAC
                    ldrawPath = QDir::toNativeSeparators(QString("%1/LDraw").arg(userLocalDataPath));
#else
                    ldrawPath = QDir::toNativeSeparators(QString("%1/ldraw").arg(userLocalDataPath));
#endif

                    if ( ! QDir(ldrawPath).exists()) { // manual prompt for LDraw Library location

                        QString searchDetail;
#if defined Q_OS_WIN || defined Q_OS_MAC
                        searchDetail = QMessageBox::tr (" %1\n %2\n %3\n %4")
                                     .arg(QDir::toNativeSeparators(QString("%1/LDraw").arg(homePath)))
                                     .arg(QDir::toNativeSeparators(QString("%1/LDraw").arg(userDocumentsPath)))
                                     .arg(QDir::toNativeSeparators(QString("%1/LDraw").arg(dataPathList.at(1))))
                                     .arg(QDir::toNativeSeparators(QString("%1/LDraw").arg(userLocalDataPath)));
#else
                        searchDetail = QMessageBox::tr (" %1\n %2\n %3\n %4")
                                     .arg(QDir::toNativeSeparators(QString("%1/ldraw").arg(homePath)))
                                     .arg(QDir::toNativeSeparators(QString("%1/ldraw").arg(userDocumentsPath)))
                                     .arg(QDir::toNativeSeparators(QString("%1/ldraw").arg(dataPathList.at(2))))
                                     .arg(QDir::toNativeSeparators(QString("%1/ldraw").arg(userLocalDataPath)));
#endif

                        if (modeGUI) {
#ifdef Q_OS_MAC
                            if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                                Application::instance()->splash->hide();
#endif
                            QPixmap _icon = QPixmap(":/icons/lpub96.png");
                            QMessageBox box;
                            box.setWindowIcon(QIcon());
                            box.setIconPixmap (_icon);
                            box.setTextFormat (Qt::RichText);
                            box.setWindowTitle(QMessageBox::tr ("LDraw Library"));
                            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                            QString header;
                            QString body;
                            QString detail;

                            if (isAppImagePayload ) { // For AppImage, autimatically install LDraw library if not found

                                if (extractLDrawLib()) {

                                    header = "<b> " + QMessageBox::tr ("LDraw library installed") + "</b>";
                                    body = QMessageBox::tr ("LDraw library was not found. The bundled library archives were installed at\n"
                                                            "%1").arg(ldrawPath);
                                    detail = QMessageBox::tr ("The following locations were searched for the LDraw library.\n%1\n"
                                                              "You can change the library path in the Preferences dialogue.").arg(searchDetail);
                                    box.setText (header);
                                    box.setInformativeText (body);
                                    box.setDetailedText(detail);
                                    box.setStandardButtons (QMessageBox::Close);
                                    box.exec();

                                }

                            } else {                  // For all except AppImage, prompt user to select or install LDraw library if not found

                                QAbstractButton* extractButton = box.addButton(QMessageBox::tr("Extract Archive"),QMessageBox::YesRole);
                                QAbstractButton* selectButton  = box.addButton(QMessageBox::tr("Select Folder"),QMessageBox::YesRole);

                                header = "<b> " + QMessageBox::tr ("LDraw library folder not found!") + "</b>";
                                body = QMessageBox::tr ("You may select the LDraw folder or extract LDraw from the bundled library archives.\n"
                                                        "Would you like to extract the library or select the LDraw folder?");
                                detail = QMessageBox::tr ("The following locations were searched for the LDraw library:\n%1\n"
                                                          "You must select an LDraw library folder or extract the library.\n"
                                                          "It is possible to create your library folder from the official archive "
                                                          "file (complete.zip) and the unofficial archive file (lpub3dldrawunf.zip).\n"
                                                          "The extracted library folder will be located at '%2'").arg(searchDetail, ldrawPath);
                                box.setText (header);
                                box.setInformativeText (body);
                                box.setDetailedText(detail);
                                box.setStandardButtons (QMessageBox::Cancel);
                                box.exec();

                                if (box.clickedButton()==selectButton) {
                                    emit Application::instance()->splashMsgSig("10% - Selecting LDraw folder...");

                                    ldrawPath = QFileDialog::getExistingDirectory(NULL,
                                                                                  QFileDialog::tr("Select LDraw libray folder"),
                                                                                  "/",
                                                                                  QFileDialog::ShowDirsOnly |
                                                                                  QFileDialog::DontResolveSymlinks);

                                    if (! ldrawPath.isEmpty()) {
                                        Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawKey),ldrawPath);
                                    }

                                } else {
                                    if (box.clickedButton()==extractButton) {
                                        extractLDrawLib();
                                    } else {
                                        ldrawPath.clear();
                                    }
                                }
                            }

                        } else {                  // Console mode so install extract LDraw Library atutomatically if not exist in searched paths.
                            QString message = QString("LDraw library was not found. The following locations were searched for the LDraw library:\n%1.\n").arg(searchDetail);
                            fprintf(stdout,"%s",message.toLatin1().constData());
                            if (extractLDrawLib()) {
                                message = QString("The bundled library archives were installed at:\n%1\n"
                                                  "You can change the library path in the Preferences dialogue.\n").arg(ldrawPath);
                                fprintf(stdout,"%s",message.toLatin1().constData());
                            }
                        }
                    }
                }
            }
        }
    }

    if (! ldrawPath.isEmpty() && force && modeGUI) {
#ifdef Q_OS_MAC
        if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
            Application::instance()->splash->hide();
#endif
        QString result = QFileDialog::getExistingDirectory(NULL,
                                                           QFileDialog::tr("Select LDraw Directory"),
                                                           ldrawPath,
                                                           QFileDialog::ShowDirsOnly |
                                                           QFileDialog::DontResolveSymlinks);
        if (! result.isEmpty())
            ldrawPath = QDir::toNativeSeparators(result);
    }

    if (! ldrawPath.isEmpty()) {

        Settings.setValue(QString("%1/%2").arg(SETTINGS,ldrawKey),ldrawPath);

    } else {

        if (modeGUI) {
#ifdef Q_OS_MAC
            if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                Application::instance()->splash->hide();
#endif
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(QMessageBox::tr ("LDraw Directory"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

            QString header = "<b> " + QMessageBox::tr ("No LDraw library defined!") + "</b>";
            QString body = QMessageBox::tr ("You must enter your LDraw library path. \nDo you wish to continue?");
            QString detail = QMessageBox::tr ("The LDraw library is required by the LPub3D renderer(s). "
                                              "If an LDraw library is not defined, the renderer will not "
                                              "be able to find the parts and primitives needed to render images.");

            box.setText (header);
            box.setInformativeText (body);
            box.setDetailedText(detail);
            box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);

            if (box.exec() != QMessageBox::Yes) {

                exit(-1);

            } else {
                ldrawPreferences(false);
            }

        } else {
            fprintf(stderr, "No LDraw library defined! The application will terminate.\n");
            exit(-1);
        }
    }

    // Check for and set alternate LDConfig file if specified
    QString const altLDConfigPathKey("AltLDConfigPath");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,altLDConfigPathKey))) {
        altLDConfigPath = Settings.value(QString("%1/%2").arg(SETTINGS,altLDConfigPathKey)).toString();
    }

    if (! altLDConfigPath.isEmpty()) {

        QFileInfo altLDConfigFile(altLDConfigPath);

        if (altLDConfigFile.exists()) {
            return;
        } else {

            if (modeGUI) {
#ifdef Q_OS_MAC
                if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
                    Application::instance()->splash->hide();
#endif
                QPixmap _icon = QPixmap(":/icons/lpub96.png");
                QMessageBox box;
                box.setWindowIcon(QIcon());
                box.setIconPixmap (_icon);
                box.setTextFormat (Qt::RichText);
                box.setWindowTitle(QMessageBox::tr ("Alternate LDraw LDConfig"));
                box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);

                QAbstractButton* selectButton  = box.addButton(QMessageBox::tr("Select File"),QMessageBox::YesRole);

                QString header = "<b> " + QMessageBox::tr ("Alternate LDraw LDConfig file not detected!") + "</b>";
                QString body = QMessageBox::tr ("The alternate LDraw LDConfig file:\n"
                                                "%1 does not exist.\n"
                                                "Would you like to select the alternate LDConfig file?").arg(altLDConfigFile.absoluteFilePath());;
                QString detail = QMessageBox::tr ("The alternate LDraw LDConfig file is optional.\n"
                                                  "You can use 'Select File' to select or 'Cancel' to\n"
                                                  "abandon the alternate LDraw LDConfig file.\n");
                box.setText (header);
                box.setInformativeText (body);
                box.setDetailedText(detail);
                box.setStandardButtons (QMessageBox::Cancel);
                box.exec();

                if (box.clickedButton()==selectButton) {
#ifdef Q_OS_WIN
                    QString filter(QMessageBox::tr("LDraw (*.ldr);;All Files (*.*)"));
#else
                    QString filter(QMessageBox::tr("All Files (*.*)"));
#endif
                    altLDConfigPath = QFileDialog::getOpenFileName(NULL,
                                                                   QFileDialog::tr("Select LDRaw LDConfig file"),
                                                                   ldrawPath,
                                                                   filter);

                    if (! altLDConfigPath.isEmpty()) {

                        Settings.setValue(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"),altLDConfigPath);

                    }

                } else {
                    altLDConfigPath.clear();
                }

            } else {
                fprintf(stdout, "The alternate LDraw LDConfig file %s does not exist. Setting ignored.\n", altLDConfigFile.absoluteFilePath().toLatin1().constData());
            }
        }
    }
#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif
}

void Preferences::lpub3dUpdatePreferences(){

    emit Application::instance()->splashMsgSig("15% - Selecting update settings...");

    QSettings Settings;

    moduleVersion = qApp->applicationVersion();

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"))) {
        checkUpdateFrequency = 0;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"),checkUpdateFrequency);
    } else {
        checkUpdateFrequency = Settings.value(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency")).toInt();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"))) {
        QVariant pValue(true);
        showUpdateNotifications = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"),pValue);
    } else {
        showUpdateNotifications = Settings.value(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"EnableDownloader"))) {
        QVariant pValue(true);
        enableDownloader = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"EnableDownloader"),pValue);
    } else {
        enableDownloader = Settings.value(QString("%1/%2").arg(UPDATES,"EnableDownloader")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"))) {
        QVariant pValue(true);
        showAllNotifications = true;
        Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"),pValue);
    } else {
        showAllNotifications = Settings.value(QString("%1/%2").arg(UPDATES,"ShowAllNotifications")).toBool();
    }
}

void Preferences::lgeoPreferences()
{
    logInfo() << "LGEO library status...";
    QSettings Settings;
    QString const lgeoDirKey("LGEOPath");
    QString lgeoDir = "";
    if (Settings.contains(QString("%1/%2").arg(POVRAY,lgeoDirKey))){
        lgeoDir = Settings.value(QString("%1/%2").arg(POVRAY,lgeoDirKey)).toString();
    } else { // check in ldraw directory path for lgeo
        lgeoDir = ldrawPath + "/lgeo";
    }
    QDir lgeoDirInfo(lgeoDir);
    if (lgeoDirInfo.exists()) {
        lgeoPath = QDir::toNativeSeparators(lgeoDir);
        logInfo() << QString("LGEO library path  : %1").arg(lgeoDirInfo.absolutePath());
        /* Durat's lgeo stl library Check */
        QDir lgeoStlLibInfo(lgeoPath + "/stl");
        lgeoStlLib = lgeoStlLibInfo.exists();
        if (lgeoStlLib)
            logInfo() << QString("Durat's Stl library: %1").arg(lgeoStlLibInfo.absolutePath());
    } else {
        Settings.remove(QString("%1/%2").arg(POVRAY,lgeoDirKey));
        logInfo() << QString("LGEO library path  : Not found");
        lgeoPath.clear();
    }
}

void Preferences::renderPreferences(bool force)
{
    /* Set 3rdParty application locations */

    logInfo() << "Image renderers...";
#ifdef Q_OS_WIN
    QString lpub3d3rdPartyContent = QString("%1/3rdParty").arg(lpub3dPath);

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite.exe").arg(lpub3d3rdPartyContent, VER_LDGLITE_STR));
#ifdef __i386__
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView.exe").arg(lpub3d3rdPartyContent, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui32.exe").arg(lpub3d3rdPartyContent, VER_POVRAY_STR));
#elif defined __x86_64__
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView64.exe").arg(lpub3d3rdPartyContent, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui64.exe").arg(lpub3d3rdPartyContent, VER_POVRAY_STR));
#endif
#elif defined Q_OS_MAC
    QString lpub3d3rdPartyContent = QString("%1/LPub3D.app/Contents/3rdParty").arg(lpub3dPath);

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite").arg(lpub3d3rdPartyContent, VER_LDGLITE_STR));
    QFileInfo ldviewInfo(QString("%1/%2/bin/LDView").arg(lpub3d3rdPartyContent, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui").arg(lpub3d3rdPartyContent, VER_POVRAY_STR));
#else
    QString lpub3d3rdPartyContent = QString("%1/../share/%2/3rdParty").arg(lpub3dPath, VER_APPLICATIONNAME_STR);
    QString lpub3d3rdPartyRendererExe = QString("/opt/%1/3rdParty").arg(VER_APPLICATIONNAME_STR);

    QFileInfo ldgliteInfo(QString("%1/%2/bin/ldglite").arg(lpub3d3rdPartyRendererExe, VER_LDGLITE_STR));
    QFileInfo ldviewInfo(QString("%1/%2/bin/ldview").arg(lpub3d3rdPartyRendererExe, VER_LDVIEW_STR));
    QFileInfo povrayInfo(QString("%1/%2/bin/lpub3d_trace_cui").arg(lpub3d3rdPartyRendererExe, VER_POVRAY_STR));
#endif

    /* 3rd Party application installation status */

    bool    ldgliteInstalled = false;
    bool    ldviewInstalled = false;
    bool    povRayInstalled = false;

    // LDGLite EXE
    if (ldgliteInfo.exists()) {
        ldgliteInstalled = true;
        ldgliteExe = QDir::toNativeSeparators(ldgliteInfo.absoluteFilePath());
    }
    logInfo() << QString("LDGLIte            : %1").arg(ldgliteInfo.exists() ? ldgliteInfo.absoluteFilePath() : "Not found");

    // LDView EXE
    if (ldviewInfo.exists()) {
        ldviewInstalled = true;
        ldviewExe = QDir::toNativeSeparators(ldviewInfo.absoluteFilePath());
    }
    logInfo() << QString("LDView             : %1").arg(ldviewInfo.exists() ? ldviewInfo.absoluteFilePath() : "Not found");

    // POV-Ray EXE
    if (povrayInfo.exists()) {
        povRayInstalled = true;
        povrayExe = QDir::toNativeSeparators(povrayInfo.absoluteFilePath());
    }
    logInfo() << QString("POVRay             : %1").arg(povrayInfo.exists() ? povrayInfo.absoluteFilePath() : "Not found");

    /* Find out if we have a valid preferred renderer */

    QSettings Settings;

    QString const preferredRendererKey("PreferredRenderer");

    if (Settings.contains(QString("%1/%2").arg(SETTINGS,preferredRendererKey))) {
        preferredRenderer = Settings.value(QString("%1/%2").arg(SETTINGS,preferredRendererKey)).toString();
        if (preferredRenderer == "LDGLite") {
            if ( ! ldgliteInstalled)  {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        } else if (preferredRenderer == "LDView") {
            if ( ! ldviewInstalled) {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        } else if (preferredRenderer == "POVRay") {
            if ( ! povRayInstalled) {
                preferredRenderer.clear();
                Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));
            }
        }
    }

    if (preferredRenderer == "") {
        Settings.remove(QString("%1/%2").arg(SETTINGS,preferredRendererKey));

        if (ldviewInstalled && ldgliteInstalled) {
            preferredRenderer = povRayInstalled ? "POVRay" : "LDView";
        } else if (povRayInstalled) {
            preferredRenderer = "POVRay";
        } else if (ldviewInstalled) {
            preferredRenderer = "LDView";
        } else if (ldgliteInstalled) {
            preferredRenderer = "LDGLite";
        }
    } else {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,preferredRendererKey),preferredRenderer);
    }

    /* Set use multiple files single call rendering option */

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"))) {
        QVariant eValue(false);
        if (preferredRenderer == "LDView") {
            enableLDViewSingleCall = true;
        } else {
            enableLDViewSingleCall = false;
        }
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),eValue);
    } else {
        enableLDViewSingleCall = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall")).toBool();
    }

    if (preferredRenderer == "LDView" && enableLDViewSingleCall) {
        useLDViewSingleCall = true;
    } else {
        useLDViewSingleCall = false;
    }

    //Renderer Timeout
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"RendererTimeout"))) {
        rendererTimeout = 6;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererTimeout"),rendererTimeout);
    } else {
        rendererTimeout = Settings.value(QString("%1/%2").arg(SETTINGS,"RendererTimeout")).toInt();
    }

    // display povray image during rendering
    QString const povrayDisplayKey("PovRayDisplay");
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,povrayDisplayKey))) {
        QVariant uValue(false);
        povrayDisplay = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,povrayDisplayKey),uValue);
    } else {
        povrayDisplay = Settings.value(QString("%1/%2").arg(SETTINGS,povrayDisplayKey)).toBool();
    }

    logInfo() << "Processing renderer configuration files...";

    // Copy required config and ini resource files to application data directory
    QString lpub3d3rdPartyConfigDir = QString("%1/3rdParty").arg(lpubDataPath);
    QDateTime timeStamp = QDateTime::currentDateTime();
    QString inFileName;
    QFileInfo resourceFile;
    QFile confFileIn, confFileOut, oldFile;

    // LDView Ini
    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_INI_FILE));
    if (resourceFile.exists() && force)
    {
       logInfo() << QString("Backup and remove old file %1...").arg(resourceFile.absoluteFilePath());
       oldFile.setFileName(resourceFile.absoluteFilePath());
       oldFile.rename(resourceFile.absoluteFilePath() + "." + timeStamp.toString("ddMMyyhhmmss"));
       oldFile.remove();
       inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
    } else {
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyContent, VER_LDVIEW_STR, resourceFile.fileName());
    }
    if (!resourceFile.exists()) {
        if (!resourceFile.absoluteDir().exists())
            resourceFile.absoluteDir().mkpath(".");
        confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
        confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_INI_FILE));
        logInfo() << QString("Configuring %1...").arg(confFileOut.fileName());
        if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
        {
            QTextStream input(&confFileIn);
            QTextStream output(&confFileOut);
            while (!input.atEnd())
            {
                QString line = input.readLine();
                //logDebug() << QString("Line INPUT: %1").arg(line);
                // set ldraw dir
                if (line.contains(QRegExp("^LDrawDir=")))
                {
                    line.clear();
                    line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawPath));
                }
                // set lgeo paths as required
                if (lgeoPath != ""){
                    if (line.contains(QRegExp("^XmlMapPath=")))
                    {
                        line.clear();
                        line = QString("XmlMapPath=%1").arg(QDir::toNativeSeparators(ldrawPath));
                    }
                }
                logInfo() << QString("Line OUTPUT: %1").arg(line);
                output << line << endl;
            }
            confFileIn.close();
            confFileOut.flush();
            confFileOut.close();
        } else {
            QString confFileError;
            if (!confFileIn.errorString().isEmpty())
                confFileError.append(QString("confFileInError: %1\n").arg(confFileIn.errorString()));
            if (!confFileOut.errorString().isEmpty())
                confFileError.append(QString("confFileOutError: %1").arg(confFileOut.errorString()));
            logError() << QString("Could not open input or output file: %1").arg(qPrintable(confFileError));
        }
    }
    if (resourceFile.exists())
        ldviewIni = resourceFile.absoluteFilePath(); // populate ldview ini file
    logInfo() << QString("LDView ini file    : %1").arg(ldviewIni.isEmpty() ? "Not found" : ldviewIni);

    // LDView POV Generation Ini
    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_POV_INI_FILE));
    if (resourceFile.exists() && force)
    {
       logInfo() << QString("Backup and remove old file %1...").arg(resourceFile.absoluteFilePath());
       oldFile.setFileName(resourceFile.absoluteFilePath());
       oldFile.rename(resourceFile.absoluteFilePath() + "." + timeStamp.toString("ddMMyyhhmmss"));
       oldFile.remove();
       inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
    } else {
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyContent, VER_LDVIEW_STR, resourceFile.fileName());
    }
    if (!resourceFile.exists()) {
        if (!resourceFile.absoluteDir().exists())
            resourceFile.absoluteDir().mkpath(".");
        confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
        confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDVIEW_STR, VER_LDVIEW_POV_INI_FILE));
        logInfo() << QString("Configuring %1...").arg(confFileOut.fileName());
        if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
        {
            QTextStream input(&confFileIn);
            QTextStream output(&confFileOut);
            while (!input.atEnd())
            {
                QString line = input.readLine();
                //logDebug() << QString("Line INPUT: %1").arg(line);
                // set ldraw dir
                if (line.contains(QRegExp("^LDrawDir=")))
                {
                    line.clear();
                    line = QString("LDrawDir=%1").arg(QDir::toNativeSeparators(ldrawPath));
                }
                // set lgeo paths as required
                if (lgeoPath != ""){
                    if (line.contains(QRegExp("^XmlMapPath=")))
                    {
                        line.clear();
                        line = QString("XmlMapPath=%1").arg(QDir::toNativeSeparators(ldrawPath));
                    }
                }
                logInfo() << QString("Line OUTPUT: %1").arg(line);
                output << line << endl;
            }
            confFileIn.close();
            confFileOut.close();
        } else {
            QString confFileError;
            if (!confFileIn.errorString().isEmpty())
                confFileError.append(QString("confFileInError: %1\n").arg(confFileIn.errorString()));
            if (!confFileOut.errorString().isEmpty())
                confFileError.append(QString("confFileOutError: %1").arg(confFileOut.errorString()));
            logError() << QString("Could not open input or output file: %1").arg(qPrintable(confFileError));
        }
    }
    if (resourceFile.exists())
        ldviewPOVIni = resourceFile.absoluteFilePath(); // populate ldview pov ini file
    logInfo() << QString("LDViewPOV ini file : %1").arg(ldviewPOVIni.isEmpty() ? "Not found" : ldviewPOVIni);

    // LDGLite Ini
    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_LDGLITE_STR, VER_LDGLITE_INI_FILE));
    if (!resourceFile.exists()) {
        logInfo() << QString("Configuring %1...").arg(resourceFile.absoluteFilePath());
        if (!resourceFile.absoluteDir().exists())
            resourceFile.absoluteDir().mkpath(".");
        QFile::copy(lpub3dPath + dataLocation + resourceFile.fileName(), resourceFile.absoluteFilePath());
    }
    confFileIn.setFileName(resourceFile.absoluteFilePath());  
    if (confFileIn.open(QIODevice::ReadOnly))
    {
        ldgliteParms.clear();
        QTextStream input(&confFileIn);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            if (line.contains(QRegExp("^-.*")))
            {
                //logDebug() << QString("Line PARAM: %1").arg(line);
                ldgliteParms << line;
            }
        }
        confFileIn.close();
    } else {
        QString confFileError;
        if (!confFileIn.errorString().isEmpty())
            confFileError.append(QString("confFileInError: %1").arg(confFileIn.errorString()));
        logError() << QString("Could not open input file: %1").arg(qPrintable(confFileError));
    }
    if (preferredRenderer == "LDGLite")
        logInfo() << QString(QString("LDGLite Parameters :%1").arg((ldgliteParms.isEmpty() ? "No parameters" : ldgliteParms.join(" "))));
    if (resourceFile.exists())
        ldgliteIni = resourceFile.absoluteFilePath(); // populate ldglite ini file
    logInfo() << QString("LDGLite ini file   : %1").arg(ldgliteIni.isEmpty() ? "Not found" : ldgliteIni);

    // POV-Ray Conf
    resourceFile.setFile(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR ,VER_POVRAY_CONF_FILE));
    if (resourceFile.exists() && force)
    {
       logInfo() << QString("Backup and remove old file %1...").arg(resourceFile.absoluteFilePath());
       oldFile.setFileName(resourceFile.absoluteFilePath());
       oldFile.rename(resourceFile.absoluteFilePath() + "." + timeStamp.toString("ddMMyyhhmmss"));
       oldFile.remove();
       inFileName = QString("%1.%2").arg(resourceFile.absoluteFilePath(),timeStamp.toString("ddMMyyhhmmss"));
    } else {
       inFileName = QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyContent, VER_POVRAY_STR, resourceFile.fileName());
    }
    if (!resourceFile.exists()) {
        if (!resourceFile.absoluteDir().exists())
            resourceFile.absoluteDir().mkpath(".");
        confFileIn.setFileName(QDir::toNativeSeparators(inFileName));
        confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR, VER_POVRAY_CONF_FILE));
        logInfo() << QString("Configuring %1...").arg(confFileOut.fileName());
        if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
        {
            QTextStream input(&confFileIn);
            QTextStream output(&confFileOut);
            while (!input.atEnd())
            {
                QString line = input.readLine();
                //logDebug() << QString("Line INPUT: %1").arg(line);
                if (line.contains(QRegExp("^read* =")) && force)
                {
                    if (lgeoPath != "")
                    {
                        if (line.contains(QRegExp("[\\/|\\\\]ar")))
                        {
                            line.clear();
                            line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/ar").arg(lgeoPath)));
                        }
                        if (line.contains(QRegExp("[\\/|\\\\]lg")))
                        {
                            line.clear();
                            line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/lg").arg(lgeoPath)));
                        }
                        if (lgeoStlLib && line.contains(QRegExp("[\\/|\\\\]stl")))
                        {
                            line.clear();
                            line = QString("read* = \"%1\"").arg(QDir::toNativeSeparators(QString("%1/stl").arg(lgeoPath)));
                        }
                    }
                } else {
                    // set application 3rd party renderers path
                    line.replace(QString("__POVSYSDIR__"), QDir::toNativeSeparators(QString("%1/3rdParty/%2").arg(lpub3dPath, VER_POVRAY_STR)));
                    // set lgeo paths as required
                    if (lgeoPath != "")
                    {
                        line.replace(QString("; read* = \"__LGEOARDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                        line.replace(QString("; read* = \"__LGEOLGDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                        if (lgeoStlLib){
                            line.replace(QString("; read* = \"__LGEOSTLDIR__"), QString("read* = \"%1").arg(QDir::toNativeSeparators(lgeoPath)));
                        }
                    }
                }
                logInfo() << QString("Line OUTPUT: %1").arg(line);
                output << line << endl;
            }
            confFileIn.close();
            confFileOut.close();
        } else {
            QString confFileError;
            if (!confFileIn.errorString().isEmpty())
                confFileError.append(QString("confFileInError: %1\n").arg(confFileIn.errorString()));
            if (!confFileOut.errorString().isEmpty())
                confFileError.append(QString("confFileOutError: %1").arg(confFileOut.errorString()));
            logError() << QString("Could not open input or output file: %1").arg(qPrintable(confFileError));
        }
    }
    if (resourceFile.exists())
        povrayConf = resourceFile.absoluteFilePath();     // populate povray conf file
    logInfo() << QString("POVRay conf file   : %1").arg(povrayConf.isEmpty() ? "Not found" : povrayConf);

    // POV-Ray Conf
    resourceFile.setFile(QString("%1/%2/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR "/config" ,VER_POVRAY_INI_FILE));
    if (!resourceFile.exists()){
        if (!resourceFile.absoluteDir().exists())
            resourceFile.absoluteDir().mkpath(".");
        confFileIn.setFileName(QString("%1/%2/resources/config/%3").arg(lpub3d3rdPartyContent, VER_POVRAY_STR, resourceFile.fileName()));
        confFileOut.setFileName(QString("%1/%2/config/%3").arg(lpub3d3rdPartyConfigDir, VER_POVRAY_STR, VER_POVRAY_INI_FILE));
        logInfo() << QString("Configuring %1...").arg(confFileOut.fileName());
        if (confFileIn.open(QIODevice::ReadOnly) && confFileOut.open(QIODevice::WriteOnly))
        {
            QTextStream input(&confFileIn);
            QTextStream output(&confFileOut);
            while (!input.atEnd())
            {
                // set the application 3rd party renderers path
                QString line = input.readLine();
                line.replace(QString("__POVSYSDIR__"), QDir::toNativeSeparators(QString("%1/3rdParty/%2").arg(lpub3dPath, VER_POVRAY_STR)));
                output << line << endl;
            }
            confFileIn.close();
            confFileOut.close();
        } else {
            QString confFileError;
            if (!confFileIn.errorString().isEmpty())
                confFileError.append(QString("confFileInError: %1\n").arg(confFileIn.errorString()));
            if (!confFileOut.errorString().isEmpty())
                confFileError.append(QString("confFileOutError: %1").arg(confFileOut.errorString()));
            logError() << QString("Could not open input or output file: %1").arg(qPrintable(confFileError));
        }
    }
    if (resourceFile.exists())
        povrayIni = resourceFile.absoluteFilePath();     // populate povray ini file
    logInfo() << QString("POVRay conf file   : %1").arg(povrayIni.isEmpty() ? "Not found" : povrayIni);

    // Populate POVRay Library paths
    resourceFile.setFile(QString("%1/%2/resources/ini/%3").arg(lpub3d3rdPartyContent, VER_POVRAY_STR, VER_POVRAY_INI_FILE));
    if (resourceFile.exists())
        povrayIniPath = resourceFile.absolutePath();
    logInfo() << QString("POVRay ini path    : %1").arg(povrayIniPath.isEmpty() ? "Not found" : povrayIniPath);

    resourceFile.setFile(QString("%1/%2/resources/include/%3").arg(lpub3d3rdPartyContent, VER_POVRAY_STR, VER_POVRAY_INC_FILE));
    if (resourceFile.exists())
        povrayIncPath = resourceFile.absolutePath();
    logInfo() << QString("POVRay include path: %1").arg(povrayIncPath.isEmpty() ? "Not found" : povrayIncPath);

    resourceFile.setFile(QString("%1/%2/resources/scenes/%3").arg(lpub3d3rdPartyContent, VER_POVRAY_STR, VER_POVRAY_SCENE_FILE));
    if (resourceFile.exists())
        povrayScenePath = resourceFile.absolutePath();
    logInfo() << QString("POVRay scene path  : %1").arg(povrayScenePath.isEmpty() ? "Not found" : povrayScenePath);
}

void Preferences::pliPreferences()
{
    bool allIsWell = true;
    QSettings Settings;
    pliFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliControl")).toString();
    pliSubstitutePartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile")).toString();
    excludedPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"ExlcudedPartsFile")).toString();

    QFileInfo fileInfo(pliFile);
    if (! fileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControl"));
        allIsWell = false;
    }

    QFileInfo pliSubstituteFileInfo(pliSubstitutePartsFile);
    if (! pliSubstituteFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"));
        allIsWell = false;
    }

    QFileInfo excludeFileInfo(excludedPartsFile);
    if (! excludeFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"ExlcudedPartsFile"));
        allIsWell = false;
    }

    if (allIsWell)
        return;

    pliFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/pli.mpd"));
    pliSubstitutePartsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/pliSubstituteParts.lst"));
    excludedPartsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/excludedParts.lst"));

    QFileInfo popPliFileInfo(pliFile);
    popPliFileInfo.setFile(pliFile);
    if (popPliFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControl"),pliFile);
    }

    QFileInfo popPliSubstituteFileInfo(pliSubstitutePartsFile);
    popPliSubstituteFileInfo.setFile(pliSubstitutePartsFile);
    if (popPliSubstituteFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliSubstitutePartsFile"),pliSubstitutePartsFile);
    }

    QFileInfo popExlcudedFileInfo(excludedPartsFile);
    popExlcudedFileInfo.setFile(excludedPartsFile);
    if (popExlcudedFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"ExlcudedPartsFile"),excludedPartsFile);
    }
}

void Preferences::unitsPreferences()
{
    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(SETTINGS,"Centimeters"))) {
        QVariant uValue(true);
        preferCentimeters = true;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),uValue);
    } else {
        preferCentimeters = Settings.value(QString("%1/%2").arg(SETTINGS,"Centimeters")).toBool();
    }
}

void Preferences::annotationPreferences()
{
    bool allIsWell = true;
    QSettings Settings;
    titleAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile")).toString();
    freeformAnnotationsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile")).toString();

    QFileInfo titleFileInfo(titleAnnotationsFile);
    if (! titleFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"));
        allIsWell = false;
    }

    QFileInfo freeformFileInfo(freeformAnnotationsFile);
    if (! freeformFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"));
        allIsWell = false;
    }

    if (allIsWell)
        return;

    titleAnnotationsFile    = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/titleAnnotations.lst"));
    freeformAnnotationsFile = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/freeformAnnotations.lst"));

    QFileInfo popTitleFileInfo(titleAnnotationsFile);
    popTitleFileInfo.setFile(titleAnnotationsFile);
    if (popTitleFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"TitleAnnotationFile"),titleAnnotationsFile);
    }

    QFileInfo popFreeFormFileInfo(freeformAnnotationsFile);
    popFreeFormFileInfo.setFile(freeformAnnotationsFile);
    if (popFreeFormFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FreeFormAnnotationsFile"),freeformAnnotationsFile);
    }
}

void Preferences::fadestepPreferences()
{
    QSettings Settings;
    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"))) {
        QVariant eValue(false);
        enableFadeStep = false;
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"),eValue);
    } else {
        enableFadeStep = Settings.value(QString("%1/%2").arg(SETTINGS,"EnableFadeStep")).toBool();
    }

    if (! Settings.contains(QString("%1/%2").arg(SETTINGS,"FadeStepColor"))) {
        QVariant cValue("Very_Light_Bluish_Gray");
        fadeStepColor = "Very_Light_Bluish_Gray";
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColor"),cValue);
    } else {
        fadeStepColor = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepColor")).toString();
    }

    fadeStepColorPartsFile = Settings.value(QString("%1/%2").arg(SETTINGS,"FadeStepColorPartsFile")).toString();
    QFileInfo fadeStepColorFileInfo(fadeStepColorPartsFile);
    if (!fadeStepColorFileInfo.exists()) {
        Settings.remove(QString("%1/%2").arg(SETTINGS,"FadeStepColorPartsFile"));
    }

    fadeStepColorPartsFile    = QDir::toNativeSeparators(QString("%1/%2").arg(lpubDataPath,"extras/fadeStepColorParts.lst"));

    QFileInfo popFadeStepColorFileInfo(fadeStepColorPartsFile);
    popFadeStepColorFileInfo.setFile(fadeStepColorPartsFile);
    if (popFadeStepColorFileInfo.exists()) {
        Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColorPartsFile"),fadeStepColorPartsFile);
    }
}

void Preferences::exportPreferences()
{
    QSettings Settings;
    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg"))) {
        QVariant uValue(false);
        ignoreMixedPageSizesMsg = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg"),uValue);
    } else {
        ignoreMixedPageSizesMsg = Settings.value(QString("%1/%2").arg(DEFAULTS,"IgnoreMixedPageSizesMsg")).toBool();
    }
}

void Preferences::publishingPreferences()
{
    QSettings Settings;

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"))) {
        QVariant pValue(false);
        displayAllAttributes = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"),pValue);
    } else {
        displayAllAttributes = Settings.value(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"generateCoverPages"))) {
        QVariant pValue(false);
        generateCoverPages = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"generateCoverPages"),pValue);
    } else {
        generateCoverPages = Settings.value(QString("%1/%2").arg(DEFAULTS,"generateCoverPages")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"))) {
        QVariant pValue(false);
        printDocumentTOC = false;
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"),pValue);
    } else {
        printDocumentTOC = Settings.value(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC")).toBool();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"Email"))) {
        QVariant eValue("");
        defaultEmail = "";
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Email"),eValue);
    } else {
        defaultEmail = Settings.value(QString("%1/%2").arg(DEFAULTS,"Email")).toString();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"URL"))) {
        QVariant uValue("");
        defaultURL = "";
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"URL"),uValue);
    } else {
        defaultURL = Settings.value(QString("%1/%2").arg(DEFAULTS,"URL")).toString();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"Author"))) {
        QVariant eValue("");
        defaultAuthor = "";
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Author"),eValue);
    } else {
        defaultAuthor = Settings.value(QString("%1/%2").arg(DEFAULTS,"Author")).toString();
    }

    if ( ! Settings.contains(QString("%1/%2").arg(DEFAULTS,"PublishDescription"))) {
        QVariant eValue("");
        publishDescription = "";
        Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PublishDescription"),eValue);
    } else {
        publishDescription = Settings.value(QString("%1/%2").arg(DEFAULTS,"PublishDescription")).toString();
    }

    if (Settings.contains(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"))) {
        documentLogoFile = Settings.value(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile")).toString();
        QFileInfo fileInfo(documentLogoFile);
        if (fileInfo.exists()) {
            return;
        } else {
            Settings.remove(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"));
        }
    }
}

void Preferences::viewerPreferences()
{  
    lcSetProfileInt(LC_PROFILE_GRID_STUDS, 0);
    lcSetProfileInt(LC_PROFILE_GRID_LINES, 0);
    lcSetProfileInt(LC_PROFILE_CHECK_UPDATES, 0);

    if (!povrayExe.isEmpty())
        lcSetProfileString(LC_PROFILE_POVRAY_PATH, povrayExe);
    if (!lgeoPath.isEmpty())
        lcSetProfileString(LC_PROFILE_POVRAY_LGEO_PATH, lgeoPath);
    if (!defaultAuthor.isEmpty())
        lcSetProfileString(LC_PROFILE_DEFAULT_AUTHOR_NAME, defaultAuthor);

    QSettings Settings;
    if (Settings.contains(QString("%1/%2").arg(SETTINGS,"ProjectsPath")))
        lcSetProfileString(LC_PROFILE_PROJECTS_PATH, Settings.value(QString("%1/%2").arg(SETTINGS,"ProjectsPath")).toString());
}

bool Preferences::getPreferences()
{
#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isVisible())
        Application::instance()->splash->hide();
#endif
    fadeStepSettingChanged     = false;
    fadeStepColorChanged       = false;

    PreferencesDialog *dialog = new PreferencesDialog();

    QSettings Settings;

    if (dialog->exec() == QDialog::Accepted) {

        if (ldrawPath != dialog->ldrawPath()) {
            ldrawPath = dialog->ldrawPath();
            if (ldrawPath == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"LDrawDir"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDrawDir"),ldrawPath);
            }
        }

        if (altLDConfigPath != dialog->altLDConfigPath())
        {
            altLDConfigPath = dialog->altLDConfigPath();
            if (altLDConfigPath == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"AltLDConfigPath"),altLDConfigPath);
            }
        }

        if (pliFile != dialog->pliFile()) {
            pliFile = dialog->pliFile();
            if (pliFile == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PliControl"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PliControl"),pliFile);
            }
        }

        if (preferredRenderer != dialog->preferredRenderer()) {
            preferredRenderer = dialog->preferredRenderer();
            if (preferredRenderer == "") {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"));
            } else {
                Settings.setValue(QString("%1/%2").arg(SETTINGS,"PreferredRenderer"),preferredRenderer);
            }
        }

        if (lgeoPath != dialog->lgeoPath()) {
            lgeoPath = dialog->lgeoPath();
            if(lgeoPath == "") {
                Settings.remove(QString("%1/%2").arg(POVRAY,"LGEOPath"));
            } else {
                Settings.setValue(QString("%1/%2").arg(POVRAY,"LGEOPath"),lgeoPath);
            }
        }

        if (povrayDisplay != dialog->povrayDisplay())
        {
            povrayDisplay = dialog->povrayDisplay();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"PovRayDisplay"),povrayDisplay);
        }

        if (ldSearchDirs != dialog->searchDirSettings()) {
            if (! dialog->searchDirSettings().isEmpty()){
                ldSearchDirs.clear();
                QString unoffDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("unofficial"));
                QString fadeDirPath  = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg("fade"));
                QString modelsDirPath = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::ldrawPath).arg("models"));
                foreach (QString dirPath, dialog->searchDirSettings()) {
                    QDir searchDir(dirPath);
                    bool invalidSearchDir = dirPath.contains(unoffDirPath.toLower()) && !dirPath.contains(fadeDirPath.toLower());
                    if (!searchDir.exists() || (dirPath.size() > 1 && invalidSearchDir && dirPath.toLower() != modelsDirPath.toLower())){
                        QMessageBox::warning(NULL,
                                             QMessageBox::tr("LPub3D"),
                                             QMessageBox::tr("%1 is not a valid directory.\nAdded directories must be under the Unofficial directory. This path will not be saved.")
                                             .arg(dirPath));
                        continue;
                    } else {
                        ldSearchDirs << dirPath;
                    }
                }
                if (! ldSearchDirs.isEmpty())
                    Settings.setValue(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"),ldSearchDirs);
                else
                    Settings.remove(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"));
            } else {
                Settings.remove(QString("%1/%2").arg(SETTINGS,"LDSearchDirs"));
            }
            setLDViewSearchPaths(true);   // update ldviewPOVIni
            setLDViewSearchPaths(false);  // update ldviewInn
        }

        if (rendererTimeout != dialog->rendererTimeout()) {
            rendererTimeout = dialog->rendererTimeout();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"RendererTimeout"),rendererTimeout);
        }

        if (documentLogoFile != dialog->documentLogoFile()) {
            documentLogoFile = dialog->documentLogoFile();
            if (documentLogoFile == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DocumentLogoFile"),documentLogoFile);
            }
        }

        if (defaultAuthor != dialog->defaultAuthor()) {
            defaultAuthor = dialog->defaultAuthor();
            if (defaultAuthor == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"Author"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Author"),defaultAuthor);
            }
        }

        if (defaultURL != dialog->defaultURL()) {
            defaultURL = dialog->defaultURL();
            if (defaultURL == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"URL"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"URL"),defaultURL);
            }
        }

        if (defaultEmail != dialog->defaultEmail()) {
            defaultEmail = dialog->defaultEmail();
            if (defaultEmail == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"Email"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"Email"),defaultEmail);
            }
        }

        if (publishDescription != dialog->publishDescription()) {
            publishDescription = dialog->publishDescription();
            if (publishDescription == "") {
                Settings.remove(QString("%1/%2").arg(DEFAULTS,"PublishDescription"));
            } else {
                Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PublishDescription"),publishDescription);
            }
        }

        if (enableFadeStep != dialog->enableFadeStep())
        {
            fadeStepSettingChanged = true;
            enableFadeStep = dialog->enableFadeStep();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableFadeStep"),enableFadeStep);
        }

        if (enableFadeStep && (fadeStepColor != dialog->fadeStepColor()))
        {
            fadeStepColorChanged = true;
            fadeStepColor = dialog->fadeStepColor();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"FadeStepColor"),fadeStepColor);
        }

        if (preferCentimeters != dialog->centimeters())
        {
            preferCentimeters = dialog->centimeters();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"Centimeters"),preferCentimeters);
            defaultResolutionType(preferCentimeters);
        }

        if (enableLDViewSingleCall != dialog->enableLDViewSingleCall()) {
            enableLDViewSingleCall = dialog->enableLDViewSingleCall();
            Settings.setValue(QString("%1/%2").arg(SETTINGS,"EnableLDViewSingleCall"),enableLDViewSingleCall);
        }

        if (enableDownloader != dialog->enableDownloader()) {
            enableDownloader = dialog->enableDownloader();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"EnableDownloader"),enableDownloader);
        }

        if (showUpdateNotifications != dialog->showUpdateNotifications()) {
            showUpdateNotifications = dialog->showUpdateNotifications();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowUpdateNotifications"),showUpdateNotifications);
        }

        if (showAllNotifications != dialog->showAllNotifications()) {
            showAllNotifications = dialog->showAllNotifications();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"ShowAllNotifications"),showAllNotifications);
        }

        if (checkUpdateFrequency != dialog->checkUpdateFrequency()) {
            checkUpdateFrequency = dialog->checkUpdateFrequency();
            Settings.setValue(QString("%1/%2").arg(UPDATES,"CheckUpdateFrequency"),checkUpdateFrequency);
        }

        if (displayAllAttributes != dialog->displayAllAttributes()) {
            displayAllAttributes = dialog->displayAllAttributes();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DisplayAllAttributes"),displayAllAttributes);
        }

        if (generateCoverPages != dialog->generateCoverPages()) {
            generateCoverPages = dialog->generateCoverPages();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"generateCoverPages"),generateCoverPages);
        }

        if (printDocumentTOC != dialog->printDocumentTOC()) {
            printDocumentTOC = dialog->printDocumentTOC();
            Settings.setValue(QString("%1/%2").arg(DEFAULTS,"PrintDocumentTOC"),printDocumentTOC);
        }

        if (preferredRenderer == "LDView" && enableLDViewSingleCall) {
            useLDViewSingleCall = true;
        } else {
            useLDViewSingleCall = false;
        }

        if (moduleVersion != dialog->moduleVersion()){
            moduleVersion = dialog->moduleVersion();
        }

        if (includeLogLevel != dialog->includeLogLevel())
        {
            includeLogLevel = dialog->includeLogLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLogLevel"),includeLogLevel);
        }

        if (includeTimestamp != dialog->includeTimestamp())
        {
            includeTimestamp = dialog->includeTimestamp();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeTimestamp"),includeTimestamp);
        }

        if (includeLineNumber != dialog->includeLineNumber())
        {
            includeLineNumber = dialog->includeLineNumber();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeLineNumber"),includeLineNumber);
        }

        if (includeFileName != dialog->includeFileName())
        {
            includeFileName = dialog->includeFileName();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeFileName"),includeFileName);
        }

        if (includeAllLogAttributes != dialog->includeAllLogAttrib())
        {
            includeAllLogAttributes = dialog->includeAllLogAttrib();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"IncludeAllLogAttributes"),includeAllLogAttributes);
        }

        if (logging != dialog->loggingGrpBox())
        {
            logging = dialog->loggingGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"Logging"),logging);
        }

        if (loggingLevel != dialog->logLevelCombo())
        {
            loggingLevel = dialog->logLevelCombo();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LoggingLevel"),loggingLevel);
        }

        if (logLevel != dialog->logLevelGrpBox())
        {
            logLevel = dialog->logLevelGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevel"),logLevel);
        }

        if (logLevels != dialog->logLevelsGrpBox())
        {
            logLevels = dialog->logLevelsGrpBox();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"LogLevels"),logLevels);
        }

        if (debugLevel != dialog->debugLevel())
        {
            debugLevel = dialog->debugLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"DebugLevel"),debugLevel);
        }

        if (traceLevel != dialog->traceLevel())
        {
            traceLevel = dialog->traceLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"TraceLevel"),traceLevel);
        }

        if (noticeLevel != dialog->noticeLevel())
        {
            noticeLevel = dialog->noticeLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"NoticeLevel"),noticeLevel);
        }

        if (infoLevel != dialog->infoLevel())
        {
            infoLevel = dialog->infoLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"InfoLevel"),infoLevel);
        }

        if (statusLevel != dialog->statusLevel())
        {
            statusLevel = dialog->statusLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"StatusLevel"),statusLevel);
        }

        if (errorLevel != dialog->errorLevel())
        {
            errorLevel = dialog->errorLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"ErrorLevel"),errorLevel);
        }

        if (fatalLevel != dialog->fatalLevel())
        {
            fatalLevel = dialog->fatalLevel();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"FatalLevel"),fatalLevel);
        }

        if (allLogLevels != dialog->allLogLevels())
        {
            allLogLevels = dialog->allLogLevels();
            Settings.setValue(QString("%1/%2").arg(LOGGING,"AllLogLevels"),allLogLevels);
        }

        return true;
    } else {
        return false;
    }
#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif
}

void Preferences::getRequireds()
{
    if (preferredRenderer == "" && ! getPreferences()) {
        exit (-1);
    }
}

void Preferences::setLPub3DLoaded(){
    lpub3dLoaded = true;
}

void Preferences::setLDViewSearchPaths(bool povini){
    QFile confFile;
    if (povini) {
        confFile.setFileName(ldviewPOVIni);
    } else {
        confFile.setFileName(ldviewIni);
    }
    QStringList contentList;
    // populate QStringList
    if (confFile.open(QIODevice::ReadOnly))
    {
        bool foundSearchDirsTag = false;
        QTextStream input(&confFile);
        while (!input.atEnd())
        {
            QString line = input.readLine();
            if (line.trimmed() == "[ExtraSearchDirs]") {
                contentList += line;
                foreach (QString searchDir, ldSearchDirs)
                    contentList += searchDir;
                foundSearchDirsTag = true;
            } else {
                if (!contentList.contains(line) && !line.isEmpty())
                    contentList += line;
            }
        }
        confFile.close();
        if (! foundSearchDirsTag)
            logError() << QString("Did not find [ExtraSearchDirs] tag in %1 "
                                  "The inf file may be malformed or corrupt.").arg(confFile.fileName());
    } else {
        QString confFileError;
        if (!confFile.errorString().isEmpty())
            confFileError.append(QString("confFileInError: %1\n").arg(confFile.errorString()));
        logError() << QString("Could not open input: %1").arg(qPrintable(confFileError));
    }
    // write search dir to ldview ini files
    if (confFile.open(QIODevice::WriteOnly))
    {
        QTextStream output(&confFile);
        foreach (QString line, contentList)
            output << line << endl;
        confFile.flush();
        confFile.close();
    } else {
        QString confFileError;
        if (!confFile.errorString().isEmpty())
            confFileError.append(QString("confFileOutError: %1").arg(confFile.errorString()));
        logError() << QString("Could not open input or output file: %1").arg(qPrintable(confFileError));
    }
}

bool Preferences::extractLDrawLib() {

#ifdef Q_OS_MAC
    if (! lpub3dLoaded && modeGUI && Application::instance()->splash->isHidden())
        Application::instance()->splash->show();
#endif

    QString const ldrawKey("LDrawDir");
    QSettings Settings;
    QFileInfo validFile;
    QString message;
    bool r = true;

    message = QMessageBox::tr("Extracting LDraw library, please wait...");

    emit Application::instance()->splashMsgSig(message.prepend("10% - "));

    if (!modeGUI) {
      fprintf(stdout,"%s",message.toLatin1().constData());
      fflush(stdout);
    }

    // if ldraw directory path is empty use the default location (datapath)
    if (ldrawPath.isEmpty()) {
        ldrawPath = QString("%1/%2/").arg(lpubDataPath, "ldraw");
    }

    // set ldraw parent directory to extract complete.zip
    bool dirNameNotLdraw = false;
    QDir ldrawDir(ldrawPath);
    logInfo() << qPrintable(QString("LDraw directory: %1").arg(ldrawDir.absolutePath()));
    if (ldrawDir.dirName().toLower() != "ldraw")
        dirNameNotLdraw = true;
    if (!ldrawDir.isRoot())
        ldrawDir.cdUp();            // ldraw path parent directory
    //logInfo() << qPrintable(QString("LDraw parent directory (%1), AbsPath (%2)").arg(ldrawDir.dirName()).arg(ldrawDir.absolutePath()));

    // set the archive library path
    QDir libraryDir(QString("%1/%2").arg(lpubDataPath, "libraries"));
    validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_OFFICIAL_ARCHIVE)));

    // archive library exist so let's proceed...
    if (validFile.exists()) {

        // extract compplete.zip
        QString destination = ldrawDir.absolutePath();
        QStringList result = JlCompress::extractDir(validFile.absoluteFilePath(),destination);
        if (result.isEmpty()){
            logError() << QString("Failed to extract %1 to %2ldraw").arg(validFile.absoluteFilePath()).arg(destination);
            r = false;
        } else {
            message = QMessageBox::tr("%1 Official Library files extracted to %2ldraw").arg(result.size()).arg(destination);
            logInfo() << message;
        }
        // extract lpub3dldunof.zip
        validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LPUB3D_UNOFFICIAL_ARCHIVE)));
        if (validFile.exists()) {
            QString destination = QString("%1/unofficial").arg(Preferences::ldrawPath);
            QStringList result = JlCompress::extractDir(validFile.absoluteFilePath(),destination);
            if (result.isEmpty()){
                logError() << QString("Failed to extract %1 to %2").arg(validFile.absoluteFilePath()).arg(destination);
            } else {
                message = QMessageBox::tr("%1 Unofficial Library files extracted to %2").arg(result.size()).arg(destination);
                logInfo() << message;
            }
        } else {
            message = QMessageBox::tr ("Unofficial Librar archive file %1 does not exist.").arg(validFile.absoluteFilePath());
            logError() << message;
        }

        // copy extracted contents to ldraw directory and delete extract dir if needed
        if (dirNameNotLdraw) {
            QDir extractDir(QString("%1/ldraw").arg(ldrawDir.absolutePath()));
            if (!copyRecursively(extractDir.absolutePath(),ldrawPath)) {
                message = QMessageBox::tr("Unable to copy %1 to %2").arg(extractDir.absolutePath(),ldrawPath);
                logInfo() << message;
                r = false;
            }
            extractDir.removeRecursively();
        }

        // if no errors, (re)set ldrawPath in registry
        if (r)
            Settings.setValue(QString("%1/%2").arg(SETTINGS, ldrawKey), ldrawPath);

    } else {

        QString body;
        validFile.setFile(QDir::toNativeSeparators(QString("%1/%2").arg(libraryDir.absolutePath(), VER_LDRAW_UNOFFICIAL_ARCHIVE)));
        if (!validFile.exists()) {
            body = QMessageBox::tr ("LPub3D attempted to extract the LDraw library however the required archive files\n%1\n%2\ndoes not exist.\n").arg(lpub3dLibFile, validFile.absoluteFilePath());
        } else {
            body = QMessageBox::tr ("LPub3D attempted to extract the LDraw library however the required archive file\n%1\ndoes not exist.\n").arg(lpub3dLibFile);
        }

        if (modeGUI) {
            QPixmap _icon = QPixmap(":/icons/lpub96.png");
            QMessageBox box;
            box.setWindowIcon(QIcon());
            box.setIconPixmap (_icon);
            box.setTextFormat (Qt::RichText);
            box.setWindowTitle(QMessageBox::tr ("LDraw Library"));
            box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
            QString header = "<b> " + QMessageBox::tr ("LDraw library archive not found!") + "</b>";
            box.setStandardButtons (QMessageBox::Close);
            box.setText(header);
            box.setInformativeText(body);
            box.exec();
        } else {
            fprintf(stderr,"%s",body.toLatin1().constData());
        }

        r = false;

        // remove registry setting and clear ldrawPath
        validFile.setFile(QDir::toNativeSeparators(QString("%1/parts/1.dat").arg(ldrawPath)));
        if (!validFile.exists()) {
            Settings.remove(QString("%1/%2").arg(SETTINGS, ldrawKey));
            ldrawPath.clear();
        }

    }

    return r;
}


bool Preferences::copyRecursively(const QString &srcFilePath,
                            const QString &tgtFilePath)
{
    QFileInfo srcFileInfo(srcFilePath);
    if (srcFileInfo.isDir()) {
        QDir targetDir(tgtFilePath);
        targetDir.cdUp();
        if (!targetDir.mkdir(QFileInfo(tgtFilePath).fileName()))
            return false;
        QDir sourceDir(srcFilePath);
        QStringList fileNames = sourceDir.entryList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden | QDir::System);
        foreach (const QString &fileName, fileNames) {
            const QString newSrcFilePath
                    = srcFilePath + QLatin1Char('/') + fileName;
            const QString newTgtFilePath
                    = tgtFilePath + QLatin1Char('/') + fileName;
            if (!copyRecursively(newSrcFilePath, newTgtFilePath))
                return false;
        }
    } else {
        if (!QFile::copy(srcFilePath, tgtFilePath))
            return false;
    }
    return true;
}
