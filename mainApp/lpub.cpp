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
#include "lpub.h"
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
#include <QtWidgets/QWidget>
#else
#include <QWidget>
#endif
#include <QDesktopWidget>
#include <QSizePolicy>
#include <QFileDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QTextEdit>
#include <QCloseEvent>
#include <QUndoStack>
#include <QTextStream>
#include <QStringList>
#include <JlCompress.h>

#include "QPushButton"
#include "QHBoxLayout"
#include "QVBoxLayout"

#include "name.h"
#include "editwindow.h"
#include "parmswindow.h"
#include "paths.h"
#include "globals.h"
#include "resolution.h"
#include "lpub_preferences.h"
#include "preferencesdialog.h"
#include "render.h"
#include "metaitem.h"
#include "ranges_element.h"
#include "updatecheck.h"

#include "step.h"
//** 3D
#include "camera.h"
#include "piece.h"
#include "lc_profile.h"

#include "application.h"
#include <ui_progress_dialog.h>

//**

#if _MSC_VER > 1310
// Visual C++ 2005 and later require the source files in UTF-8, and all strings
// to be encoded as wchar_t otherwise the strings will be converted into the
// local multibyte encoding and cause errors. To use a wchar_t as UTF-8, these
// strings then need to be convert back to UTF-8.
#define wCharToUtf8(str) ConvertWideCharToUTF8(L##str)
const char * ConvertWideCharToUTF8(const wchar_t * wstr) {
	int requiredSize = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, 0, 0, 0, 0);
	if (requiredSize > 0) {
		static char * buffer = new char[requiredSize + 1];
		buffer[requiredSize] = 0;
		WideCharToMultiByte(CP_UTF8, 0, wstr, -1, buffer, requiredSize, 0, 0);
		return buffer;
	}
	return NULL;
}
#else
// Visual C++ 2003 and gcc will use the string literals as is, so the files
// should be saved as UTF-8. gcc requires the files to not have a UTF-8 BOM.
#define wCharToUtf8(str) str
#endif

Gui *gui;

void clearPliCache()
{
  gui->clearPLICache();
}

void clearCsiCache()
{
  gui->clearCSICache();
}

void clearCsi3dCache()
{
    gui->clearTempCache();
}

void clearAndRedrawPage()
{
    gui->clearAndRedrawPage();
}

/****************************************************************************
 * 
 * The Gui constructor and destructor are at the bottom of the file with
 * the code that creates the basic GUI framekwork 
 *
 ***************************************************************************/

int Gui::pageSize(PageMeta &meta, int which){
  int _size;

  // flip orientation for landscape
  if (meta.orientation.value() == Landscape){
      which == 0 ? _size = 1 : _size = 0;
    } else {
      _size = which;
    }
  return meta.size.valuePixels(_size);
}

void Gui::insertCoverPage()
{
  MetaItem mi;
  mi.insertCoverPage();
}

void Gui::appendCoverPage()
{
  MetaItem mi;
  mi.appendCoverPage();
  countPages();
  displayPageNum = maxPages;
  displayPage();
}

void Gui::generateCoverPages()
{
    if (Preferences::generateCoverPages){
        MetaItem mi;
        if (!mi.frontCoverPageExist())
            mi.insertCoverPage();

        if (!mi.backCoverPageExist())
            mi.appendCoverPage();
    }
}

void Gui::insertFinalModel(){
  MetaItem mi;
  static int modelExist = -1;
  // get line to insert final model
  int modelStatus = mi.okToInsertFinalModel();

  if ((Preferences::enableFadeStep || Preferences::enableHighlightStep) && modelStatus != modelExist){
      mi.insertFinalModel(modelStatus);
    } else if ((! Preferences::enableFadeStep && ! Preferences::enableHighlightStep) && modelStatus == modelExist){
      mi.deleteFinalModel();
    }
}

//void Gui::insertCoverPage()
//{
//  MetaItem mi;
//  mi.insertCoverPage();
//}

//void Gui::appendCoverPage()
//{
//  MetaItem mi;
//  mi.appendCoverPage();
//  countPages();
//  ++displayPageNum;
//  displayPage();  // display the page we just added
//}

void Gui::insertNumberedPage()
{
  MetaItem mi;
  mi.insertNumberedPage();
}

void Gui::appendNumberedPage()
{
  MetaItem mi;
  mi.appendNumberedPage();
  //countPages();
  //++displayPageNum;
  //displayPage();  // display the page we just added
}

void Gui::deletePage()
{
  MetaItem mi;
  mi.deletePage();
}

void Gui::addPicture()
{
  MetaItem mi;
  mi.insertPicture();
}

void Gui::addText()
{
  MetaItem mi;
  mi.insertText();
}

void Gui::addBom()
{
  MetaItem mi;
  mi.insertBOM();
}

void Gui::removeLPubFormatting()
{
  MetaItem mi;
  mi.removeLPubFormatting();
  displayPageNum = 1;
  displayPage();
}

void Gui::displayPage()
{
  emit messageSig(true, "Processing page display...");

  if (macroNesting == 0) {
      clearPage(KpageView,KpageScene);
      page.coverPage = false;
      drawPage(KpageView,KpageScene,false);

      enableActions2();
      emit enable3DActionsSig();
    }
  emit messageSig(true,"Page display ready.");
}

/*
void Gui::nextPage()
{
  countPages();
  if (displayPageNum < maxPages) {
    ++displayPageNum;
    displayPage();
  } else {
    statusBarMsg("You're on the last page");
  }
}

void Gui::previousPage()
{
  if (displayPageNum > 1) {
    displayPageNum--;
    displayPage();
  } else {
     statusBarMsg("You're on the first page");
  }
} */

void Gui::nextPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
      bool ok;
      int inputPageNum;
      inputPageNum = rx.cap(1).toInt(&ok);
      if (ok && (inputPageNum != displayPageNum)) {		// numbers are different so jump to page
          countPages();
          if (inputPageNum <= maxPages) {
              if (inputPageNum != displayPageNum) {
                  displayPageNum = inputPageNum;
                  displayPage();
                  return;
                }
            } else {
              statusBarMsg("Page number entered is higher than total pages");
            }
          string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
          setPageLineEdit->setText(string);
          return;
        } else {						// numbers are same so goto next page
          countPages();
          if (displayPageNum < maxPages) {
              ++displayPageNum;
              displayPage();
            } else {
              statusBarMsg("You are on the last page");
            }
        }
    }
}

void Gui::nextPageContinuous()
{
  // This effectively acts like a toggle. If true, it sets false and vice versa.
  nextPageContinuousIsRunning = !nextPageContinuousIsRunning;

  QMessageBox box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Information);
  box.setWindowTitle(tr ("Next page continuous processing"));
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  QString title = "<b>       Next page continuous processing&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
  QString message = QString("Next page continuous processing is running.\n");

  // Request to terminate Next page process while it is still running
  if (!nextPageContinuousIsRunning) {
      box.setIcon (QMessageBox::Warning);
      title = "<b> Stop next page continuous processing ?&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
      message = QString("Next page continuous processing has not completed.\n"
                        "Do you want to terminate this process ?");
      box.setText (title);
      box.setInformativeText (message);

      // Yes, let's terminate
      if (box.exec() == QMessageBox::Yes){
          return;
        }
      // Oops, do not want to terminate
      else {
          nextPageContinuousIsRunning = true;
        }

    }
  // If user clicks Next page process while Previous page process is still running
  else if (previousPageContinuousIsRunning && nextPageContinuousIsRunning) {
      box.setIcon (QMessageBox::Warning);
      title = "<b>       Stop Previous page continuous processing ?&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
      message = QString("Previous page continuous processing is running.\n"
                        "Do you want to stop this process and start Next page processing ?");

      box.setText (title);
      box.setInformativeText (message);

    // User wants to stop Previous page process
    if (box.exec() == QMessageBox::Yes){
        previousPageContinuousIsRunning = false;
        continuousPageDialog(PAGE_NEXT);
      }
    // User wants to continue running Previous page process
    else {
        nextPageContinuousIsRunning = false;
        return;
      }
  // No conflicts, everything looks good, let's go
  } else {
      continuousPageDialog(PAGE_NEXT);
  }
}

void Gui::previousPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
      bool ok;
      int inputPageNum;
      inputPageNum = rx.cap(1).toInt(&ok);
      if (ok && (inputPageNum != displayPageNum)) {		// numbers are different so jump to page
          countPages();
          if (inputPageNum >= 1) {
              if (inputPageNum != displayPageNum) {
                  displayPageNum = inputPageNum;
                  displayPage();
                  return;
                }
            } else {
              statusBarMsg("Page number entered is invalid");
            }
          string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
          setPageLineEdit->setText(string);
          return;
        } else {						// numbers are same so goto previous page
          if (displayPageNum > 1) {
              displayPageNum--;
              displayPage();
            } else {
              statusBarMsg("You are on the first page");
            }
        }
    }
}

void Gui::previousPageContinuous()
{
  // This effectively acts like a toggle. If true, it sets false and vice versa.
  previousPageContinuousIsRunning = !previousPageContinuousIsRunning;

  QMessageBox box;
  box.setTextFormat (Qt::RichText);
  box.setIcon (QMessageBox::Information);
  box.setWindowFlags (Qt::Dialog | Qt::CustomizeWindowHint | Qt::WindowTitleHint);
  box.setWindowTitle(tr ("Previous page continuous processing"));
  box.setStandardButtons (QMessageBox::Yes | QMessageBox::No);
  QString title = "<b> Previous page continuous processing&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
  QString message = QString("Previous page continuous processing is running.\n");

  // Request to terminate Previous page process while it is still running
  if (!previousPageContinuousIsRunning) {
      box.setIcon (QMessageBox::Warning);
      title = "<b> Stop previous page continuous processing ?;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; </b>";
      message = QString("Previous page continuous processing has not completed.\n"
                        "Do you want to terminate this process ?");

      box.setText (title);
      box.setInformativeText (message);

      // Yes, let's terminate
      if (box.exec() == QMessageBox::Yes){
          return;
        }
      // Oops, do not want to terminate
      else {
          previousPageContinuousIsRunning = true;
        }

    }
  // If user clicks Previous page process while Next page process is still running
  else if (nextPageContinuousIsRunning  && previousPageContinuousIsRunning) {
      box.setIcon (QMessageBox::Warning);
      title = "<b> Stop Next page continuous processing ?;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</b>";
      message = QString("Next page continuous processing is running.         \n"
                        "Do you want to terminate this process and start over ?");

      box.setText (title);
      box.setInformativeText (message);

    // User wants to stop Next page process
    if (box.exec() == QMessageBox::Yes){
        nextPageContinuousIsRunning = false;
        continuousPageDialog(PAGE_PREVIOUS);
      }
    // User wants to continue running Next page process
    else {
        previousPageContinuousIsRunning = false;
        return;
      }
    // No conflicts, everything looks good, let's go
    } else {
      continuousPageDialog(PAGE_PREVIOUS);
    }
}

void Gui::firstPage()
{
  displayPageNum = 1;
  displayPage();
}

void Gui::lastPage()
{
  countPages();
  displayPageNum = maxPages;
  displayPage();
}

void Gui::setPage()
{
  QString string = setPageLineEdit->displayText();
  QRegExp rx("^(\\d+).*$");
  if (string.contains(rx)) {
    bool ok;
    int inputPage;
    inputPage = rx.cap(1).toInt(&ok);
    if (ok) {
      countPages();
      if (inputPage <= maxPages) {
        if (inputPage != displayPageNum) {
          displayPageNum = inputPage;
          displayPage();
          return;
        }
      } else {
        statusBarMsg("Page number entered is higher than total pages");
      }
    }
  }
  string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
  setPageLineEdit->setText(string);
}

void Gui::setGoToPage(int index)
{
  int goToPageNum = index+1;
  countPages();
  if (goToPageNum <= maxPages) {
      if (goToPageNum != displayPageNum) {
          displayPageNum = goToPageNum;
          displayPage();
        }
    }
  QString string = QString("%1 of %2") .arg(displayPageNum) .arg(maxPages);
  setPageLineEdit->setText(string);
}

void Gui::fitWidth()
{
  fitWidth(pageview());
}

// Compare two variants - less than.
bool lt(const int &v1, const int &v2){ return v1 < v2; }
// Compare two variants - greater than.
bool gt(const int &v1, const int &v2){ return v1 > v2; }
// Sleeper
class secSleeper : public QThread
{
public:
    static void secSleep(unsigned long Secs){ QThread::sleep(Secs); }
};

bool Gui::continuousPageDialog(Direction d)
{
  pageDirection = d;
  int pageCount = 0;
  int _maxPages    = 0;
  bool terminateProcess = false;

  QString direction = d == PAGE_NEXT ? "Next" : "Previous";

  exportType = PAGE_PROCESS;

  if (Preferences::modeGUI) {
      if (Preferences::doNotShowPageProcessDlg) {
          if (!processPageRange(setPageLineEdit->displayText())) {
              if (d == PAGE_NEXT)
                nextPageContinuousIsRunning = false;
              else
                previousPageContinuousIsRunning = false;
              return false;
            }
        }
      else
        {
            DialogExportPages *dialog = new DialogExportPages();

            if (dialog->exec() == QDialog::Accepted) {

               if(dialog->allPages()){
                 processOption = EXPORT_ALL_PAGES;
               }
               else
               if(dialog->pageRange()){
                  processOption  = EXPORT_PAGE_RANGE;
                  pageRangeText = dialog->pageRangeText();
                }

              resetCache = dialog->resetCache();

              QSettings Settings;
              if (Preferences::doNotShowPageProcessDlg != dialog->doNotShowPageProcessDlg()) {
                 Preferences::doNotShowPageProcessDlg = dialog->doNotShowPageProcessDlg();
                 Settings.setValue(QString("%1/%2").arg(DEFAULTS,"DoNotShowPageProcessDlg"),Preferences::doNotShowPageProcessDlg);
              }

            } else {
               if (d == PAGE_NEXT)
                 nextPageContinuousIsRunning = false;
               else
                 previousPageContinuousIsRunning = false;
               return false;
            }
        }
    }
  else { // command line mode
      // This effectively acts like a toggle. If true, it sets false and vice versa.
      nextPageContinuousIsRunning = !nextPageContinuousIsRunning;
    }

  if (processOption == EXPORT_PAGE_RANGE){
      if (! validatePageRange()){
          if (d == PAGE_NEXT)
            nextPageContinuousIsRunning = false;
          else
            previousPageContinuousIsRunning = false;
          return false;
        }
    }

  if (d == PAGE_NEXT) {
      nextPageContinuousAct->setIcon(QIcon(":/resources/nextpagecontinuousstop.png"));
      nextPageContinuousAct->setStatusTip("Stop continuous next page processing");
      nextPageContinuousAct->setText("Stop Continuous Next Page");
    } else {
      previousPageContinuousAct->setIcon(QIcon(":/resources/prevpagecontinuousstop.png"));
      previousPageContinuousAct->setStatusTip("Stop continuous previous page processing");
      previousPageContinuousAct->setText("Stop Continuous Previous Page");
    }

  if(resetCache){
      clearCustomPartCache(true);
      clearAndRedrawPage();
    }

  // store current display page number
  logStatus() << QString("Continuous %1 page processing start...").arg(direction);

  if (processOption == EXPORT_ALL_PAGES){

      _maxPages = maxPages;

      QApplication::setOverrideCursor(Qt::BusyCursor);

      for (displayPageNum = 1; d == PAGE_NEXT ? displayPageNum <= maxPages : displayPageNum >= 1 ; d == PAGE_NEXT ? displayPageNum++ : displayPageNum--) {

          if (d == PAGE_NEXT) {
              terminateProcess =  !nextPageContinuousIsRunning;
            } else {
              terminateProcess =  !previousPageContinuousIsRunning;
            }

          if (terminateProcess) {
              emit messageSig(true,QString("%1 page processing terminated before completion. %2 pages of %3 processed.")
                              .arg(direction)
                              .arg(displayPageNum - 1)
                              .arg(maxPages));
              if (d == PAGE_NEXT) {
                  nextPageContinuousAct->setIcon(QIcon(":/resources/nextpagecontinuous.png"));
                  nextPageContinuousAct->setStatusTip(tr("Continuously process next document page"));
                  nextPageContinuousAct->setText("Continuous Next Page");
                  nextPageContinuousIsRunning = false;
                } else {
                  previousPageContinuousAct->setIcon(QIcon(":/resources/prevpagecontinuous.png"));
                  previousPageContinuousAct->setStatusTip(tr("Continuously process previous document page"));
                  previousPageContinuousAct->setText("Continuous Previous Page");
                  previousPageContinuousIsRunning = false;
                }
              QApplication::restoreOverrideCursor();
              return false;
            }

          pageCount = displayPageNum;

          displayPage();

          emit messageSig(true,QString("Processed page %1 of %2").arg(displayPageNum).arg(maxPages));

          qApp->processEvents();

          secSleeper::secSleep(Preferences::pageDisplayPause);
        }

      QApplication::restoreOverrideCursor();

    } else if (processOption == EXPORT_PAGE_RANGE) {

      QStringList pageRanges = pageRangeText.split(",");
      QList<int> printPages;
      foreach(QString ranges,pageRanges){
          if (ranges.contains("-")){
              QStringList range = ranges.split("-");
              int minPage = range[0].toInt();
              int maxPage = range[1].toInt();
              for(int i = minPage; i <= maxPage; i++){
                  printPages.append(i);
                }
            } else {
              printPages.append(ranges.toInt());
            }
        }

      if (d == PAGE_NEXT)
        std::sort(printPages.begin(),printPages.end(),lt);    // Next - small to large
      else
        std::sort(printPages.begin(),printPages.end(),gt);    // Previous - large to small

      displayPageNum = printPages.first();

      _maxPages = printPages.count();

      QApplication::setOverrideCursor(Qt::BusyCursor);

      foreach(int printPage,printPages){

          displayPageNum = printPage;

          if (d == PAGE_NEXT) {
              terminateProcess = !nextPageContinuousIsRunning;
            } else {
              terminateProcess = !previousPageContinuousIsRunning;
            }

          if (terminateProcess) {
              emit messageSig(true,QString("%1 page processing terminated before completion. %2 pages of %3 processed.")
                              .arg(direction)
                              .arg(pageCount)
                              .arg(_maxPages));
              if (d == PAGE_NEXT) {
                  nextPageContinuousAct->setIcon(QIcon(":/resources/nextpagecontinuous.png"));
                  nextPageContinuousAct->setStatusTip(tr("Continuously process next document page"));
                  nextPageContinuousAct->setText("Continuous Next Page");
                  nextPageContinuousIsRunning = false;
                } else {
                  previousPageContinuousAct->setIcon(QIcon(":/resources/prevpagecontinuous.png"));
                  previousPageContinuousAct->setStatusTip(tr("Continuously process previous document page"));
                  previousPageContinuousAct->setText("Continuous Previous Page");
                  previousPageContinuousIsRunning = false;
                }
              QApplication::restoreOverrideCursor();
              return false;
            }

          pageCount++;

          displayPage();

          emit messageSig(true,QString("Processed page %1, %2 of %3 for page range %4")
                          .arg(displayPageNum)
                          .arg(pageCount)
                          .arg(_maxPages)
                          .arg(pageRanges.join(" ")));

          qApp->processEvents();

          secSleeper::secSleep(Preferences::pageDisplayPause);
        }

      QApplication::restoreOverrideCursor();
    }

  if (d == PAGE_NEXT) {
      nextPageContinuousAct->setIcon(QIcon(":/resources/nextpagecontinuous.png"));
      nextPageContinuousAct->setStatusTip(tr("Continuously process next document page"));
      nextPageContinuousAct->setText("Continuous Next Page");
      nextPageContinuousIsRunning = false;
    } else {
      previousPageContinuousAct->setIcon(QIcon(":/resources/prevpagecontinuous.png"));
      previousPageContinuousAct->setStatusTip(tr("Continuously process previous document page"));
      previousPageContinuousAct->setText("Continuous Previous Page");
      previousPageContinuousIsRunning = false;
    }

  emit messageSig(true,QString("%1 page processing completed. %2 of %3 pages processed.").arg(direction).arg(pageCount).arg(_maxPages));

  return true;
}

bool Gui::processPageRange(const QString &range)
{
  if (!range.isEmpty()){
      QString lineAllPages = QString("1-%1").arg(gui->maxPages);
      QString linePageRange = QString(range)
          .replace("of","-")
          .replace("to","-")
          .replace(" ","");
      if (lineAllPages == linePageRange) {
          processOption = EXPORT_ALL_PAGES;
        } else {
          processOption = EXPORT_PAGE_RANGE;
          pageRangeText = linePageRange;
        }
      return true;
    } else {
      processOption = EXPORT_ALL_PAGES;
      return true;
    }
  return false;
}

void Gui::fitWidth(
  LGraphicsView *view)
{
  view->scale(1.0,1.0);

  QRectF rect(0,0,pageSize(page.meta.LPub.page, 0),pageSize(page.meta.LPub.page, 1));

  QRectF unity = view->matrix().mapRect(QRectF(0,0,1,1));
  view->scale(1/unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = view->viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = view->matrix().mapRect(rect);
  qreal xratio = viewRect.width() / sceneRect.width();

  view->scale(xratio,xratio);
  view->centerOn(rect.center());
  fitMode = FitWidth;
}

void Gui::fitVisible()
{
  fitVisible(pageview());
}

void Gui::fitVisible(
  LGraphicsView *view)
{
  view->scale(1.0,1.0);

  QRectF rect(0,0,pageSize(page.meta.LPub.page, 0),pageSize(page.meta.LPub.page, 1));

  QRectF unity = view->matrix().mapRect(QRectF(0,0,1,1));
  view->scale(1/unity.width(), 1 / unity.height());

  int margin = 2;
  QRectF viewRect = view->viewport()->rect().adjusted(margin, margin, -margin, -margin);
  QRectF sceneRect = view->matrix().mapRect(rect);
  qreal xratio = viewRect.width() / sceneRect.width();
  qreal yratio = viewRect.height() / sceneRect.height();

  xratio = yratio = qMin(xratio,yratio);
  view->scale(xratio,yratio);
  view->centerOn(rect.center());
  fitMode = FitVisible;
}

void Gui::actualSize()
{
  actualSize(pageview());
}

void Gui::actualSize(
  LGraphicsView *view)
{
  view->resetMatrix();
  fitMode = FitNone;
}
//~~~~~~~~~~~~~~
void Gui::twoPages()
{
  twoPages(pageview());
}

void Gui::twoPages(
  LGraphicsView *view)
{
   view->scale(1.0,1.0);
  fitMode = FitTwoPages;
}

void Gui::continuousScroll()
{
  continuousScroll(pageview());
}
void Gui::continuousScroll(
  LGraphicsView *view)
{
  view->scale(1.0,1.0);
  fitMode = FitContinuousScroll;
}
//~~~~~~~~~~~~~~


void Gui::zoomIn()
{
  zoomIn(pageview());
}

void Gui::zoomIn(
  LGraphicsView *view)
{
  fitMode = FitNone;
  view->scale(1.1,1.1);
}

void Gui::zoomOut()
{
  zoomOut(pageview());
}

void Gui::zoomOut(
  LGraphicsView *view)
{
  fitMode = FitNone;
  view->scale(1.0/1.1,1.0/1.1);
}

void Gui::UpdateStepRotation()
{
    mModelStepRotation = lcVector3(mRotStepAngleX, mRotStepAngleY, mRotStepAngleZ);

    QString rotLabel("Step Rotation %1 %2 %3");
    rotLabel = rotLabel.arg(QString::number(mModelStepRotation[0], 'f', 2),
                            QString::number(mModelStepRotation[1], 'f', 2),
                            QString::number(mModelStepRotation[2], 'f', 2));
    statusBarMsg(rotLabel);
}


void Gui::displayFile(
    LDrawFile     *ldrawFile,
    const QString &modelName)
{
  if (! exporting()) {
      displayFileSig(ldrawFile, modelName);
      curSubFile = modelName;
      int currentIndex = 0;
      for (int i = 0; i < mpdCombo->count(); i++) {
          if (mpdCombo->itemText(i) == modelName) {
              currentIndex = i;
              break;
            }
        }
      mpdCombo->setCurrentIndex(currentIndex);
    }
}

void Gui::displayParmsFile(
  const QString &fileName)
{
    displayParmsFileSig(fileName);
}

void Gui::deployExportBanner(bool b)
{

  if (b) {
      QString exportBanner, imageFile;

#ifdef __APPLE__

      exportBanner = QString("%1/%2").arg(Preferences::lpubDataPath,"extras/printbanner.ldr");
      imageFile = QString("%1/%2").arg(Preferences::lpubDataPath,"extras/PDFPrint.jpg");

#else

      exportBanner = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath,"extras/printbanner.ldr"));
      imageFile = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath,"extras/PDFPrint.jpg"));

#endif

      installExportBanner(exportType, exportBanner,imageFile);
    }
}
/*-----------------------------------------------------------------------------*/

bool Gui::installExportBanner(const int &type, const QString &printFile, const QString &imageFile){

    QList<QString> ldrData;
    ldrData << "0 FILE printbanner.ldr";
    ldrData << "0 Name: printbanner.ldr";
    ldrData << "0 Author: Trevor SANDY";
    ldrData << "0 Unofficial Model";
    ldrData << "0 !LEOCAD MODEL NAME Printbanner";
    ldrData << "0 !LEOCAD MODEL AUTHOR Trevor SANDY";
    ldrData << "0 !LEOCAD MODEL DESCRIPTION Graphic displayed during pdf printing";
    ldrData << "0 !LEOCAD MODEL BACKGROUND IMAGE NAME " + imageFile;
    ldrData << "1 71 0 0 0 1 0 0 0 1 0 0 0 1 3020.dat";
    ldrData << "1 71 30 -8 10 1 0 0 0 1 0 0 0 1 3024.dat";
    ldrData << "1 71 30 -16 10 1 0 0 0 1 0 0 0 1 3024.dat";
    ldrData << "1 71 -30 -8 10 1 0 0 0 1 0 0 0 1 3024.dat";
    ldrData << "1 71 -30 -16 10 1 0 0 0 1 0 0 0 1 3024.dat";
    ldrData << "1 71 -30 -32 10 1 0 0 0 1 0 0 0 1 6091.dat";
    ldrData << "1 71 30 -32 10 1 0 0 0 1 0 0 0 1 6091.dat";
    ldrData << "1 71 30 -32 10 1 0 0 0 1 0 0 0 1 30039.dat";
    ldrData << "1 2 -30 -32 10 1 0 0 0 1 0 0 0 1 30039.dat";
    ldrData << "1 71 0 -24 10 1 0 0 0 1 0 0 0 1 3937.dat";
    ldrData << "1 72 0 -8 -10 1 0 0 0 1 0 0 0 1 3023.dat";
    ldrData << "1 72 0 -8 -10 -1 0 0 0 1 0 0 0 -1 85984.dat";
    ldrData << "1 71 0 -23.272 6.254 -1 0 0 0 0.927 0.375 0 0.375 -0.927 3938.dat";
    ldrData << "1 72 0 -45.524 -2.737 -1 0 0 0 0.927 0.375 0 0.375 -0.927 4865a.dat";
    switch (type) {
      case EXPORT_PNG:
        ldrData << "1 25 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        ldrData << "1 25 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptn.dat";
        ldrData << "1 25 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptg.dat";
        break;
      case EXPORT_JPG:
        ldrData << "1 92 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptj.dat";
        ldrData << "1 92 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        ldrData << "1 92 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptg.dat";
        break;
      case EXPORT_BMP:
        ldrData << "1 73 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptb.dat";
        ldrData << "1 73 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptm.dat";
        ldrData << "1 73 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        break;
      default:
        ldrData << "1 216 -22 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptp.dat";
        ldrData << "1 216 -2 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptd.dat";
        ldrData << "1 216 18 -4 -32 1 0 0 0 0.423 -0.906 0 0.906 0.423 3070bptf.dat";
      }
    ldrData << "0";
    ldrData << "0 NOFILE";

    QFile ldrFile(printFile);
    if ( ! ldrFile.open(QIODevice::ReadWrite | QIODevice::Truncate | QIODevice::Text)) {
        QMessageBox::warning(NULL,
                             tr("LPub3D"),
                             tr("Cannot open Export Banner file %1 for writing:\n%2")
                             .arg(printFile)
                             .arg(ldrFile.errorString()));
        return false;
    }
    QTextStream out(&ldrFile);
    for (int i = 0; i < ldrData.size(); i++) {
        QString fileLine = ldrData[i];
        out << fileLine << endl;
    }
    ldrFile.close();

    //load CSI 3D file into viewer
    if (gMainWindow->OpenProject(ldrFile.fileName())){
        return true;
    } else {return false;}

    return true;
}

void Gui::mpdComboChanged(int index)
{
  QString newSubFile = mpdCombo->currentText();
  if (curSubFile != newSubFile) {
      int modelPageNum = ldrawFile.getModelStartPageNumber(newSubFile);
      logInfo() << "SELECT Model: " << newSubFile << " @ Page: " << modelPageNum;
      countPages();
      if (displayPageNum != modelPageNum && modelPageNum != 0) {
          displayPageNum  = modelPageNum;
          displayPage();
        } else {
          // TODO add status bar message
          Where topOfSteps(newSubFile,0);
          curSubFile = newSubFile;
          displayFileSig(&ldrawFile, curSubFile);
          showLineSig(topOfSteps.lineNumber);
        }
    }
  mpdCombo->setCurrentIndex(index);
}


void Gui::clearAllCaches()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(true,"A model must be open to reset its caches - no action taken.");
        return;
    }

    if (Preferences::enableFadeStep || Preferences::enableHighlightStep) {
       ldrawFile.clearPrevStepPositions();

    }

     clearPLICache();
     clearCSICache();
     clearTempCache();

     //reload current model file
     int savePage = displayPageNum;
     openFile(curFile);
     displayPageNum = savePage;
     displayPage();
     enableActions();

     emit messageSig(true,"All caches reset and model file reloaded.");
}

void Gui::clearAndRedrawPage()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(true,"A model must be open to reset its caches - no action taken.");
        return;
    }

     clearAllCaches();
}

void Gui::clearCustomPartCache(bool silent)
{
  QMessageBox::StandardButton ret;
  QString message = QString("All existing custom part files will be deleted and regenerated.\n"
                            "Warning: Only custom part files for the currently loaded model file will be updated in %1.")
                            .arg(FILE_LPUB3D_UNOFFICIAL_ARCHIVE);
  if (silent) {
      emit messageSig(silent,message);
  } else {
      ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
                                 tr("%1\nDo you want to delete the custom file cache?").arg(message),
                                 QMessageBox::Yes | QMessageBox::Discard | QMessageBox::Cancel);
  }

  if (ret == QMessageBox::Yes || silent) {

      QString dirName = QDir::toNativeSeparators(QString("%1/%2").arg(Preferences::lpubDataPath).arg(Paths::customDir));

      int count = 0;
      if (removeDir(count, dirName)){
          emit messageSig(silent,QMessageBox::tr("Custom parts cache cleaned.  %1 %2 removed.")
                               .arg(count)
                               .arg(count == 1 ? "item": "items"));
      } else {
          emit messageSig(silent,QMessageBox::tr("Unable to remeove custom parts cache directory: %1").arg(dirName));
        return;
      }

      // regenerate custom parts
      if (! getCurFile().isEmpty()) {
        bool overwriteCustomParts = true;
        processFadeColourParts(overwriteCustomParts);       // (re)generate and archive fade parts based on the loaded model file
        processHighlightColourParts(overwriteCustomParts);  // (re)generate and archive highlight parts based on the loaded model file
      }

    } else if (ret == QMessageBox::Cancel) {
      return;
    }
}

void Gui::clearPLICache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(true,"A model must be open to clean its parts cache - no action taken.");
        return;
    }

    QString dirName = QDir::currentPath() + "/" + Paths::partsDir;
    QDir dir(dirName);

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(dirName + "/" + fileInfo.fileName());
        if (!file.remove()) {
            QMessageBox::critical(NULL,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(dirName + "/" + fileInfo.fileName()));
            count--;
          } else
          count++;
      }

    emit messageSig(true,QString("Parts content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearCSICache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(true,"A model must be open to clean its assembly cache - no action taken.");
        return;
    }

    QString dirName = QDir::currentPath() + "/" + Paths::assemDir;
    QDir dir(dirName);

    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = dir.entryInfoList();
    int count = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(dirName + "/" + fileInfo.fileName());
        if (!file.remove()) {
            QMessageBox::critical(NULL,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(dirName + "/" + fileInfo.fileName()));
            count--;
          } else
          count++;
      }

    emit messageSig(true,QString("Assembly content cache cleaned. %1 items removed.").arg(count));
}

void Gui::clearTempCache()
{
    if (getCurFile().isEmpty()) {
        emit messageSig(true,"A model must be open to clean its 3D cache - no action taken.");
        return;
    }

    QString tmpDirName = QDir::currentPath() + "/" + Paths::tmpDir;
    QDir tmpDir(tmpDirName);

    tmpDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList list = tmpDir.entryInfoList();
    int count1 = 0;
    for (int i = 0; i < list.size(); i++) {
        QFileInfo fileInfo = list.at(i);
        QFile     file(tmpDirName + "/" + fileInfo.fileName());
        if (!file.remove()) {
            QMessageBox::critical(NULL,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(tmpDirName + "/" + fileInfo.fileName()));
            count1--;
          } else
          count1++;
      }

    ldrawFile.tempCacheCleared();

    QString viewDirName = QDir::currentPath() + "/" + Paths::viewerDir;
    QDir viewDir(viewDirName);

    viewDir.setFilter(QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot | QDir::NoSymLinks);

    QFileInfoList viewDirList = viewDir.entryInfoList();
    int count2 = 0;
    for (int i = 0; i < viewDirList.size(); i++) {
        QFileInfo fileInfo = viewDirList.at(i);
        QFile     file(viewDirName + "/" + fileInfo.fileName());
        if (!file.remove()) {
            QMessageBox::critical(NULL,
                                  tr("LPub3D"),
                                  tr("Unable to remeove %1")
                                  .arg(viewDirName + "/" + fileInfo.fileName()));
            count2--;
          } else
          count2++;
    }

    emit messageSig(true,QString("3D Viewer content cache cleaned. %1 temp and %2 viewer items removed.").arg(count1).arg(count2));
}

bool Gui::removeDir(int &count, const QString & dirName)
{
    bool result = true;
    QDir dir(dirName);

    if (dir.exists(dirName)) {
        Q_FOREACH(QFileInfo info, dir.entryInfoList(QDir::NoDotAndDotDot | QDir::System | QDir::Hidden  | QDir::AllDirs | QDir::Files, QDir::DirsFirst)) {
            if (info.isDir()) {
                result = removeDir(count,info.absoluteFilePath());
            }
            else {
                result = QFile::remove(info.absoluteFilePath());
                count++;
            }

            if (!result) {
                return result;
            }
        }
        result = dir.rmdir(dirName);
    }
    return result;
}

void Gui::clearStepCSICache(QString &pngName) {
  QString tmpDirName   = QDir::currentPath() + "/" + Paths::tmpDir;
  QString assemDirName = QDir::currentPath() + "/" + Paths::assemDir;
  QFileInfo fileInfo(pngName);
  QFile file(assemDirName + "/" + fileInfo.fileName());
  if (!file.remove()) {
      QMessageBox::critical(NULL,
                            QMessageBox::tr("LPub3D"),
                            QMessageBox::tr("Unable to remove %1")
                            .arg(assemDirName + "/" + fileInfo.fileName()));
    }
  QString ldrName;
  if (renderer->useLDViewSCall()){
      ldrName = tmpDirName + "/" + fileInfo.completeBaseName() + ".ldr";
    } else {
      ldrName = tmpDirName + "/csi.ldr";
    }
  file.setFileName(ldrName);
  if (!file.remove()) {
      QMessageBox::critical(NULL,
                            QMessageBox::tr("LPub3D"),
                            QMessageBox::tr("Unable to remeove %1")
                            .arg(ldrName));
    }
  if (Preferences::enableFadeStep) {
      clearPrevStepPositions();
    }
  displayPage();
}

void Gui::clearPageCSICache(PlacementType relativeType, Page *page) {
  if (page->list.size()) {      
      if (relativeType == SingleStepType) {         // single step page
          Range *range = dynamic_cast<Range *>(page->list[0]);
          if (range->relativeType == RangeType) {
              Step *step = dynamic_cast<Step *>(range->list[0]);
              if (step && step->relativeType == StepType) {
                  clearPageCSIGraphicsItems(step);
              } // validate step (StepType) and process...
          } // validate RangeType - to cast step
      } else if (relativeType == StepGroupType) {  // multi-step page
          for (int i = 0; i < page->list.size(); i++){
              Range *range = dynamic_cast<Range *>(page->list[i]);
              for (int j = 0; j < range->list.size(); j++){
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[j]);
                      if (step && step->relativeType == StepType){
                          clearPageCSIGraphicsItems(step);
                      } // validate step (StepType) and process...
                  } // validate RangeType - to cast step
              } // for each step within divided group...=>list[AbstractRangeElement]->StepType
          } // for each divided group within page...=>list[AbstractStepsElement]->RangeType
      }
      if (Preferences::enableFadeStep) {
          clearPrevStepPositions();
      }
      displayPage();
   }
}

/*
 * Clear step image graphics items
 * This function recurses the step's model to clear images and associated model files.
 * Call only if using LDView Single Call (useLDViewsCall=true)
 */
void Gui::clearPageCSIGraphicsItems(Step *step) {
  // Capture ldr and image names
  QString tmpDirName   = QDir::currentPath() + "/" + Paths::tmpDir;
  QString assemDirName = QDir::currentPath() + "/" + Paths::assemDir;
  QString ldrName;
  bool itemsCleared = false;
  // process step's image(s)
  QFileInfo fileInfo(step->pngName);
  QFile file(assemDirName + "/" + fileInfo.fileName());
  if (!file.remove()) {
      QMessageBox::critical(NULL,
                            QMessageBox::tr("LPub3D"),
                            QMessageBox::tr("Unable to remove %1")
                            .arg(assemDirName + "/" + fileInfo.fileName()));
  }
  if (renderer->useLDViewSCall()){
      ldrName = tmpDirName + "/" + fileInfo.completeBaseName() + ".ldr";
      file.setFileName(ldrName);
      if (!file.remove()) {
          QMessageBox::critical(NULL,
                                QMessageBox::tr("LPub3D"),
                                QMessageBox::tr("Unable to remeove %1")
                                .arg(ldrName));
      }
  } else if (! itemsCleared){
      ldrName = tmpDirName + "/csi.ldr";
      file.setFileName(ldrName);
      if (!file.remove()) {
          QMessageBox::critical(NULL,
                                QMessageBox::tr("LPub3D"),
                                QMessageBox::tr("Unable to remeove %1")
                                .arg(ldrName));
      }
      itemsCleared = true;
  }
  // process callout's step(s) image(s)
  for (int k = 0; k < step->list.size(); k++) {
      if (step->list[k]->relativeType == CalloutType) {
          Callout *callout = dynamic_cast<Callout *>(step->list[k]);
          for (int l = 0; l < callout->list.size(); l++){
              Range *range = dynamic_cast<Range *>(callout->list[l]);
              for (int m = 0; m < range->list.size(); m++){
                  if (range->relativeType == RangeType) {
                      Step *step = dynamic_cast<Step *>(range->list[m]);
                      if (step && step->relativeType == StepType) {
                          clearPageCSIGraphicsItems(step);
                      } // 1.6 validate if Step relativeType is StepType - to clear image, check for Callout
                  } // 1.5 validate if Range relativeType is RangeType - to cast as Step
              } // 1.4 for each Step list-item within a Range...=>list[AbstractRangeElement]->StepType
          } // 1.3 for each Range list-item within a Callout...=>list[AbstractStepsElement]->RangeType
      } // 1.2 validate if relativeType is CalloutType - to cast as Callout...
  } // 1.1 for each Callout list-item within a Step...=>list[Steps]->CalloutType
}

/***************************************************************************
 * These are infrequently used functions for basic environment 
 * configuration stuff
 **************************************************************************/

void Gui::pageSetup()
{
  GlobalPageDialog::getPageGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::assemSetup()
{
  GlobalAssemDialog::getAssemGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::pliSetup()
{
  GlobalPliDialog::getPliGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::bomSetup()
{
  GlobalPliDialog::getBomGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::calloutSetup()
{
  GlobalCalloutDialog::getCalloutGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::multiStepSetup()
{
  GlobalMultiStepDialog::getMultiStepGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::projectSetup()
{
  GlobalProjectDialog::getProjectGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::fadeStepSetup()
{
  GlobalFadeStepDialog::getFadeStepGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::highlightStepSetup()
{
  GlobalHighlightStepDialog::getHighlightStepGlobals(ldrawFile.topLevelFile(),page.meta);
}

void Gui::editTitleAnnotations()
{
    displayParmsFile(Preferences::titleAnnotationsFile);
    parmsWindow->setWindowTitle(tr("Part Title Annotation","Edit/add part title part annotations"));
    parmsWindow->show();
}

void Gui::editFreeFormAnnitations()
{
    displayParmsFile(Preferences::freeformAnnotationsFile);
    parmsWindow->setWindowTitle(tr("Freeform Annotation","Edit/add freeform part annotations"));
    parmsWindow->show();
}

void Gui::editLDrawColourParts()
{
    displayParmsFile(Preferences::ldrawColourPartsFile);
    parmsWindow->setWindowTitle(tr("LDraw Colour Parts","Edit/add LDraw static coulour parts"));
    parmsWindow->show();
}

void Gui::editPliBomSubstituteParts()
{
    displayParmsFile(Preferences::pliSubstitutePartsFile);
    parmsWindow->setWindowTitle(tr("PLI/BOM Substitute Parts","Edit/add PLI/BOM substitute parts"));
    parmsWindow->show();
}

void Gui::editExcludedParts()
{
    displayParmsFile(Preferences::excludedPartsFile);
    parmsWindow->setWindowTitle(tr("Part Count Excluded Parts","Edit/add excluded parts"));
    parmsWindow->show();
}

void Gui::editLdrawIniFile()
{
  if (!Preferences::ldSearchDirs.isEmpty()) {
      displayParmsFile(Preferences::ldrawiniFile);
      parmsWindow->setWindowTitle(tr("LDraw.ini Edit","Edit LDraw.ini search directory entries."));
      parmsWindow->show();
    }
}

void Gui::editLdgliteIni()
{
    displayParmsFile(Preferences::ldgliteIni);
    parmsWindow->setWindowTitle(tr("Edit LDGLite ini","Edit LDGLite ini "));
    parmsWindow->show();
}

void Gui::editLdviewIni()
{
    displayParmsFile(Preferences::ldviewIni);
    parmsWindow->setWindowTitle(tr("Edit LDView ini","Edit LDView ini "));
    parmsWindow->show();
}

void Gui::editLdviewPovIni()
{
    displayParmsFile(Preferences::ldviewPOVIni);
    parmsWindow->setWindowTitle(tr("Edit LDView raytracer ini","Edit LDView raytracer ini"));
    parmsWindow->show();
}

void Gui::editPovrayIni()
{
    displayParmsFile(Preferences::povrayIni);
    parmsWindow->setWindowTitle(tr("Edit Raytracer ini","Edit Raytracer ini"));
    parmsWindow->show();
}

void Gui::editPovrayConf()
{
    displayParmsFile(Preferences::povrayConf);
    parmsWindow->setWindowTitle(tr("Edit Raytracer file access conf","Edit Raytracer file access conf"));
    parmsWindow->show();
}

void Gui::viewLog()
{
    displayParmsFile(Preferences::logPath);
    parmsWindow->setWindowTitle(tr(VER_PRODUCTNAME_STR " log",VER_PRODUCTNAME_STR " logs"));
    parmsWindow->show();
}

void Gui::preferences()
{
    bool useLDViewSCallCompare          = Preferences::useLDViewSingleCall;
    bool displayAllAttributesCompare    = Preferences::displayAllAttributes;
    bool generateCoverPagesCompare      = Preferences::generateCoverPages;
    bool enableFadeStepCompare          = Preferences::enableFadeStep;
    bool fadeStepUseColourCompare       = Preferences::fadeStepUseColour;
    int fadeStepOpacityCompare          = Preferences::fadeStepOpacity;
    bool enableHighlightStepCompare     = Preferences::enableHighlightStep;
    int  highlightStepLineWidthCompare  = Preferences::highlightStepLineWidth;
    bool doNotShowPageProcessDlgCompare = Preferences::doNotShowPageProcessDlg;
    int  pageDisplayPauseCompare        = Preferences::pageDisplayPause;
    QString fadeStepColourCompare       = Preferences::fadeStepColour;
    QString highlightStepColourCompare  = Preferences::highlightStepColour;
    QString ldrawPathCompare            = Preferences::ldrawPath;
    QString lgeoPathCompare             = Preferences::lgeoPath;
    QString preferredRendererCompare    = Preferences::preferredRenderer;

    if (Preferences::getPreferences()) {

        Meta meta;
        page.meta = meta;

        bool rendererChanged               = QString(Preferences::preferredRenderer).toLower()   != preferredRendererCompare.toLower();
        bool enableFadeStepChanged         = Preferences::enableFadeStep                         != enableFadeStepCompare;
        bool fadeStepUseColourChanged      = Preferences::fadeStepUseColour                      != fadeStepUseColourCompare;
        bool fadeStepColorChanged          = QString(Preferences::fadeStepColour).toLower()      != fadeStepColourCompare.toLower();
        bool fadeStepOpacityChanged        = Preferences::fadeStepOpacity                        != fadeStepOpacityCompare;
        bool enableHighlightStepChanged    = Preferences::enableHighlightStep                    != enableHighlightStepCompare;
        bool highlightStepColorChanged     = QString(Preferences::highlightStepColour).toLower() != highlightStepColourCompare.toLower();
        bool highlightStepLineWidthChanged = Preferences::highlightStepLineWidth                 != highlightStepLineWidthCompare;
        bool useLDViewSCallChanged         = Preferences::enableLDViewSingleCall                 != useLDViewSCallCompare;
        bool displayAttributesChanged      = Preferences::displayAllAttributes                   != displayAllAttributesCompare;
        bool generateCoverPagesChanged     = Preferences::generateCoverPages                     != generateCoverPagesCompare;
        bool pageDisplayPauseChanged       = Preferences::pageDisplayPause                       != pageDisplayPauseCompare;
        bool doNotShowPageProcessDlgChanged= Preferences::doNotShowPageProcessDlg                != doNotShowPageProcessDlgCompare;

        bool ldrawPathChanged              = QString(Preferences::ldrawPath).toLower()           != ldrawPathCompare.toLower();
        bool lgeoPathChanged               = QString(Preferences::lgeoPath).toLower()            != lgeoPathCompare.toLower();

        if (enableFadeStepChanged)
            logInfo() << QString("Fade Previous Steps is %1.").arg(Preferences::enableFadeStep ? "ON" : "OFF");

        if (fadeStepUseColourChanged && Preferences::enableFadeStep)
            logInfo() << QString("Use Global Fade Colour is %1").arg(Preferences::fadeStepUseColour ? "ON" : "OFF");

        if (fadeStepOpacityChanged && Preferences::enableFadeStep)
            logInfo() << QString("Fade Step Transparency changed from %1 to %2 percent")
                                 .arg(fadeStepOpacityCompare)
                                 .arg(Preferences::fadeStepOpacity);

        if (fadeStepColorChanged && Preferences::enableFadeStep && Preferences::fadeStepUseColour)
            logInfo() << QString("Fade Step Colour preference changed from %1 to %2")
                                 .arg(fadeStepColourCompare.replace("_"," "))
                                 .arg(QString(Preferences::fadeStepColour).replace("_"," "));

        if (enableHighlightStepChanged)
            logInfo() << QString("Highlight Current Step is %1.").arg(Preferences::enableHighlightStep ? "ON" : "OFF");

        if (highlightStepLineWidthChanged && Preferences::enableHighlightStep)
            logInfo() << QString("Highlight Step line width changed from %1 to %2")
                                 .arg(highlightStepLineWidthCompare)
                                 .arg(Preferences::highlightStepLineWidth);

        if (highlightStepColorChanged && Preferences::enableHighlightStep)
            logInfo() << QString("Highlight Step Colour preference changed from %1 to %2")
                                 .arg(highlightStepColourCompare)
                                 .arg(Preferences::highlightStepColour);

        if (ldrawPathChanged)
            logInfo() << QString("LDraw path preference changed from %1 to %1")
                                 .arg(ldrawPathCompare)
                                 .arg(Preferences::ldrawPath);

        if (lgeoPathChanged && !ldrawPathChanged)
            logInfo() << QString("LGEO path preference changed from %1 to %1")
                                 .arg(lgeoPathCompare)
                                 .arg(Preferences::lgeoPath);

        if (generateCoverPagesChanged)
            logInfo() << QString("Generate Cover Pages preference is %1").arg(Preferences::generateCoverPages ? "ON" : "OFF");


        if (pageDisplayPauseChanged)
            logInfo() << QString("Continuous process page display pause changed from %1 to %2")
                              .arg(highlightStepLineWidthCompare)
                              .arg(Preferences::pageDisplayPause);

        if (doNotShowPageProcessDlgChanged)
            logInfo() << QString("Show continuous page process options dialog is %1.").arg(Preferences::doNotShowPageProcessDlg ? "ON" : "OFF");

        if ((((fadeStepColorChanged && Preferences::fadeStepUseColour) ||
            fadeStepUseColourChanged || fadeStepOpacityChanged) &&
            Preferences::enableFadeStep && !enableFadeStepChanged) ||
           ((highlightStepColorChanged || highlightStepLineWidthChanged) &&
            Preferences::enableHighlightStep && !enableHighlightStepChanged))
           clearCustomPartCache(true);    // true = clear and regenerate custom part files

        if (rendererChanged) {
            logInfo() << QString("Renderer preference changed from %1 to %2")
                                 .arg(preferredRendererCompare)
                                 .arg(Preferences::preferredRenderer);
            Render::setRenderer(Preferences::preferredRenderer);
            if (Preferences::preferredRenderer == RENDERER_LDGLITE)
                partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();
        }

        if (!getCurFile().isEmpty()) {
            if (enableFadeStepChanged         ||
                fadeStepColorChanged          ||
                fadeStepUseColourChanged      ||
                fadeStepOpacityChanged        ||
                enableHighlightStepChanged    ||
                highlightStepColorChanged     ||
                highlightStepLineWidthChanged ||
                rendererChanged               ||
                useLDViewSCallChanged         ||
                displayAttributesChanged      ||
                generateCoverPagesChanged){
                clearAndRedrawPage();
            }
        }

        // set logging options
        using namespace QsLogging;
        Logger& logger = Logger::instance();
        if (Preferences::logging) {
            if (Preferences::logLevels){

                logger.setLoggingLevels();
                logger.setDebugLevel(Preferences::debugLevel);
                logger.setTraceLevel(Preferences::traceLevel);
                logger.setNoticeLevel(Preferences::noticeLevel);
                logger.setInfoLevel(Preferences::infoLevel);
                logger.setStatusLevel(Preferences::statusLevel);
                logger.setErrorLevel(Preferences::errorLevel);
                logger.setFatalLevel(Preferences::fatalLevel);

              } else if (Preferences::logLevel){

                bool ok;
                Level logLevel = logger.fromLevelString(Preferences::loggingLevel,&ok);
                if (!ok)
                  QMessageBox::critical(NULL,QMessageBox::tr(VER_PRODUCTNAME_STR),
                                        QMessageBox::tr("Failed to set log level %1.\n"
                                                        "Logging is off - level set to OffLevel")
                                        .arg(Preferences::loggingLevel));
                logger.setLoggingLevel(logLevel);
              }

            logger.setIncludeLogLevel(Preferences::includeLogLevel);
            logger.setIncludeTimestamp(Preferences::includeTimestamp);
            logger.setIncludeLineNumber(Preferences::includeLineNumber);
            logger.setIncludeFileName(Preferences::includeFileName);
            logger.setIncludeFunctionInfo(Preferences::includeFunction);

            // logging examples
            bool showLogExamples = false;
            if (showLogExamples){
                logStatus() << "[Pref] Uh-oh! - this level is not displayed in the console only the log";
                logInfo()   << "[Pref] LPub3D logging reloaded";
                logInfo()   << "[Pref] Built with Qt" << QT_VERSION_STR << "running on" << qVersion();
                logTrace()  << "[Pref] Here's a" << QString("trace") << "message";
                logDebug()  << "[Pref] Here's a" << static_cast<int>(QsLogging::DebugLevel) << "message";
                logNotice() << "[Pref] Here's a" << QString("Notice") << "message";
                qDebug()    << "[Pref] This message won't be picked up by the logger";
                logError()  << "[Pref] An error has occurred";
                qWarning()  << "[Pref] Neither will this one";
                logFatal()  << "[Pref] Fatal error!";

                Level level = logger.loggingLevel();
                logger.setLoggingLevel(QsLogging::OffLevel);
                for (int i = 0;i < 10;++i) {
                    logError() << QString::fromUtf8("[Pref] This message should not be visible");
                }
                logger.setLoggingLevel(level);
            } // end init logging

          } else {
            logger.setLoggingLevel(OffLevel);
          }
    }
}


/*******************************************************************************
 *
 * This is all the initialization stuff.  It is used once when the program 
 * starts up
 *
 ******************************************************************************/

Gui::Gui()
{
    emit Application::instance()->splashMsgSig(QString("35% - %1 window defaults loading...").arg(VER_PRODUCTNAME_STR));

    Preferences::lgeoPreferences();
    Preferences::rendererPreferences(false);
    Preferences::viewerPreferences();
    Preferences::publishingPreferences();
    Preferences::exportPreferences();

    displayPageNum = 1;

    processOption                   = EXPORT_ALL_PAGES;
    exportType                      = EXPORT_PDF;
    pageRangeText                   = "1";
    resetCache                      = false;
    m_previewDialog                 = false;
    m_exportingContent              = false;
    nextPageContinuousIsRunning     = false;
    previousPageContinuousIsRunning = false;

    editWindow    = new EditWindow(this);  // remove inheritance 'this' to independently manage window
    parmsWindow   = new ParmsWindow();

    KpageScene    = new QGraphicsScene(this);
    KpageScene->setBackgroundBrush(Qt::lightGray);
    KpageView     = new LGraphicsView(KpageScene);
    KpageView->pageBackgroundItem = NULL;
    KpageView->setRenderHints(QPainter::Antialiasing | 
                             QPainter::TextAntialiasing |
                             QPainter::SmoothPixmapTransform);
    setCentralWidget(KpageView);

    mpdCombo = new QComboBox(this);
    mpdCombo->setToolTip(tr("Current Submodel"));
    mpdCombo->setMinimumContentsLength(25);
    mpdCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    mpdCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    mpdCombo->setStatusTip("Use dropdown to select submodel");
    connect(mpdCombo,SIGNAL(activated(int)),
            this,    SLOT(mpdComboChanged(int)));

    setGoToPageCombo = new QComboBox(this);
    setGoToPageCombo->setToolTip(tr("Current Page"));
    setGoToPageCombo->setMinimumContentsLength(10);
    setGoToPageCombo->setInsertPolicy(QComboBox::InsertAtBottom);
    setGoToPageCombo->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    setGoToPageCombo->setStatusTip("Use dropdown to select page");
    connect(setGoToPageCombo,SIGNAL(activated(int)),
            this,            SLOT(setGoToPage(int)));

    progressLabel = new QLabel(this);
    progressLabel->setMinimumWidth(200);
    progressBar = new QProgressBar();
    progressBar->setMaximumWidth(300);

    progressLabelPerm = new QLabel();
    progressLabelPerm->setMinimumWidth(200);
    progressBarPerm = new QProgressBar();
    progressBarPerm->setMaximumWidth(300);

    mExistingRotStep = lcVector3(0.0f, 0.0f, 0.0f);
    mModelStepRotation = lcVector3(0.0f, 0.0f, 0.0f);

    undoStack = new QUndoStack();
    macroNesting = 0;

    connect(this,           SIGNAL(setExportingSig(bool)),
            this,           SLOT(  setExporting(   bool)));

    connect(this,           SIGNAL(displayFileSig(LDrawFile *, const QString &)),
            editWindow,     SLOT(  displayFile   (LDrawFile *, const QString &)));

    connect(this,           SIGNAL(displayParmsFileSig(const QString &)),
            parmsWindow,    SLOT( displayParmsFile   (const QString &)));

    connect(this,           SIGNAL(showLineSig(int)),
            editWindow,     SLOT(  showLine(   int)));

    connect(editWindow,     SIGNAL(redrawSig()),
            this,           SLOT(  clearAndRedrawPage()));

    connect(editWindow,     SIGNAL(contentsChange(const QString &,int,int,const QString &)),
            this,           SLOT(  contentsChange(const QString &,int,int,const QString &)));

    connect(undoStack,      SIGNAL(canRedoChanged(bool)),
            this,           SLOT(  canRedoChanged(bool)));
    connect(undoStack,      SIGNAL(canUndoChanged(bool)),
            this,           SLOT(  canUndoChanged(bool)));
    connect(undoStack,      SIGNAL(cleanChanged(bool)),
            this,           SLOT(  cleanChanged(bool)));

    connect(this, SIGNAL(messageSig(bool,QString)),           this, SLOT(statusMessage(bool,QString)));

    connect(this, SIGNAL(progressBarInitSig()),               this, SLOT(progressBarInit()));
    connect(this, SIGNAL(progressMessageSig(QString)),        this, SLOT(progressBarSetText(QString)));
    connect(this, SIGNAL(progressRangeSig(int,int)),          this, SLOT(progressBarSetRange(int,int)));
    connect(this, SIGNAL(progressSetValueSig(int)),           this, SLOT(progressBarSetValue(int)));
    connect(this, SIGNAL(progressResetSig()),                 this, SLOT(progressBarReset()));
    connect(this, SIGNAL(removeProgressStatusSig()),          this, SLOT(removeProgressStatus()));

    connect(this, SIGNAL(progressBarPermInitSig()),           this, SLOT(progressBarPermInit()));
    connect(this, SIGNAL(progressPermMessageSig(QString)),    this, SLOT(progressBarPermSetText(QString)));
    connect(this, SIGNAL(progressPermRangeSig(int,int)),      this, SLOT(progressBarPermSetRange(int,int)));
    connect(this, SIGNAL(progressPermSetValueSig(int)),       this, SLOT(progressBarPermSetValue(int)));
    connect(this, SIGNAL(progressPermResetSig()),             this, SLOT(progressBarPermReset()));
    connect(this, SIGNAL(removeProgressPermStatusSig()),      this, SLOT(removeProgressPermStatus()));

    m_progressDialog         = new ProgressDialog();
    m_progressDlgProgressBar = m_progressDialog->findChild<QProgressBar*>("progressDlgProgressBar");
    m_progressDlgMessageLbl  = m_progressDialog->findChild<QLabel*>("progressDlgMessageLbl");

    connect (m_progressDialog, SIGNAL (cancelClicked()), this, SLOT (cancelExporting()));

#ifdef WATCHER
    connect(&watcher,       SIGNAL(fileChanged(const QString &)),
             this,          SLOT(  fileChanged(const QString &)));
    changeAccepted = true;
#endif

    gui = this;

    fitMode = FitVisible;
}

Gui::~Gui()
{ 

    delete KpageScene;
    delete KpageView;
    delete editWindow;
    delete parmsWindow;

}

void Gui::closeEvent(QCloseEvent *event)
{

  writeSettings();

  emit requestEndThreadNowSig();

  if (parmsWindow->isVisible())
    parmsWindow->close();

  if (maybeSave()) {
      event->accept();
    } else {
      event->ignore();
    }
}

void Gui::getRequireds(){
    // Check preferred renderer value is set before setting Renderer class
    Preferences::getRequireds();
    Render::setRenderer(Preferences::preferredRenderer);
}

void Gui::initialize()
{

  emit Application::instance()->splashMsgSig(QString("85% - %1 initialization...").arg(VER_PRODUCTNAME_STR));

  connect(this,       SIGNAL(loadFileSig(QString)),      this,        SLOT(loadFile(QString)));
  connect(this,       SIGNAL(processCommandLineSig()),   this,        SLOT(processCommandLine()));
  connect(this,       SIGNAL(setExportingSig(bool)),     this,        SLOT(deployExportBanner(bool)));
  connect(this,       SIGNAL(setExportingSig(bool)),     gMainWindow, SLOT(Halt3DViewer(bool)));
  connect(this,       SIGNAL(enable3DActionsSig()),      gMainWindow, SLOT(Enable3DActions()));
  connect(this,       SIGNAL(disable3DActionsSig()),     gMainWindow, SLOT(Disable3DActions()));
  connect(this,       SIGNAL(updateAllViewsSig()),       gMainWindow, SLOT(UpdateAllViews()));
  connect(this,       SIGNAL(clearViewerWindowSig()),    gMainWindow, SLOT(NewProject()));

  if (Preferences::preferredRenderer == RENDERER_LDGLITE)
      partWorkerLdgLiteSearchDirs.populateLdgLiteSearchDirs();

  emit Application::instance()->splashMsgSig(QString("90% - %1 widgets loading...").arg(VER_PRODUCTNAME_STR));

  createActions();
  createMenus();
  createToolBars();
  createStatusBar();
  createDockWindows();
  toggleLCStatusBar();

  emit disable3DActionsSig();
  setCurrentFile("");

  readSettings();

}

void Gui::generateCustomColourPartsList()
{
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr(VER_PRODUCTNAME_STR),
            tr("Generating the colour parts list may take a long time.\n"
                "Are you sure you want to generate this list?"),
            QMessageBox::Yes | QMessageBox::Cancel);
    if (ret == QMessageBox::Yes) {

        QThread *listThread   = new QThread();
        colourPartListWorker  = new ColourPartListWorker();
        colourPartListWorker->moveToThread(listThread);

        connect(listThread,           SIGNAL(started()),                     colourPartListWorker, SLOT(generateCustomColourPartsList()));
        connect(listThread,           SIGNAL(finished()),                              listThread, SLOT(deleteLater()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),             listThread, SLOT(quit()));
        connect(colourPartListWorker, SIGNAL(colourPartListFinishedSig()),   colourPartListWorker, SLOT(deleteLater()));
        connect(this,                 SIGNAL(requestEndThreadNowSig()),      colourPartListWorker, SLOT(requestEndThreadNow()));

        connect(colourPartListWorker, SIGNAL(messageSig(bool,QString)),                      this, SLOT(statusMessage(bool,QString)));

        connect(colourPartListWorker, SIGNAL(progressBarInitSig()),                          this, SLOT(progressBarInit()));
        connect(colourPartListWorker, SIGNAL(progressMessageSig(QString)),                   this, SLOT(progressBarSetText(QString)));
        connect(colourPartListWorker, SIGNAL(progressRangeSig(int,int)),                     this, SLOT(progressBarSetRange(int,int)));
        connect(colourPartListWorker, SIGNAL(progressSetValueSig(int)),                      this, SLOT(progressBarSetValue(int)));
        connect(colourPartListWorker, SIGNAL(progressResetSig()),                            this, SLOT(progressBarReset()));
        connect(colourPartListWorker, SIGNAL(removeProgressStatusSig()),                     this, SLOT(removeProgressStatus()));

        listThread->start();

    } else {
      return;
    }
}

void Gui::processFadeColourParts(bool overwriteCustomParts)
{
  if (gui->page.meta.LPub.fadeStep.fadeStep.value()) {

      QThread *partThread    = new QThread();
      partWorkerCustomColour = new PartWorker();
      partWorkerCustomColour->moveToThread(partThread);

      connect(this,                   SIGNAL(operateFadeParts(bool)),    partWorkerCustomColour, SLOT(processFadeColourParts(bool)));
      connect(partThread,             SIGNAL(finished()),                            partThread, SLOT(deleteLater()));
      connect(partWorkerCustomColour, SIGNAL(customColourFinishedSig()),             partThread, SLOT(quit()));
      connect(partWorkerCustomColour, SIGNAL(customColourFinishedSig()), partWorkerCustomColour, SLOT(deleteLater()));
      connect(partWorkerCustomColour, SIGNAL(requestFinishSig()),                    partThread, SLOT(quit()));
      connect(partWorkerCustomColour, SIGNAL(requestFinishSig()),        partWorkerCustomColour, SLOT(deleteLater()));
      connect(this,                   SIGNAL(requestEndThreadNowSig()),  partWorkerCustomColour, SLOT(requestEndThreadNow()));

      connect(partWorkerCustomColour, SIGNAL(messageSig(bool,QString)),                   this, SLOT(statusMessage(bool,QString)));

      connect(partWorkerCustomColour, SIGNAL(progressBarInitSig()),                       this, SLOT(progressBarInit()));
      connect(partWorkerCustomColour, SIGNAL(progressMessageSig(QString)),                this, SLOT(progressBarSetText(QString)));
      connect(partWorkerCustomColour, SIGNAL(progressRangeSig(int,int)),                  this, SLOT(progressBarSetRange(int,int)));
      connect(partWorkerCustomColour, SIGNAL(progressSetValueSig(int)),                   this, SLOT(progressBarSetValue(int)));
      connect(partWorkerCustomColour, SIGNAL(progressResetSig()),                         this, SLOT(progressBarReset()));
      connect(partWorkerCustomColour, SIGNAL(removeProgressStatusSig()),                  this, SLOT(removeProgressStatus()));

      partThread->start();

      //qDebug() << qPrintable(QString("Sent overwrite fade parts = %1").arg(overwriteCustomParts ? "True" : "False"));
      emit operateFadeParts(overwriteCustomParts);
    }
}

void Gui::processHighlightColourParts(bool overwriteCustomParts)
{
  if (gui->page.meta.LPub.highlightStep.highlightStep.value()) {

      QThread *partThread    = new QThread();
      partWorkerCustomColour = new PartWorker();
      partWorkerCustomColour->moveToThread(partThread);

      connect(this,                   SIGNAL(operateHighlightParts(bool)), partWorkerCustomColour, SLOT(processHighlightColourParts(bool)));
      connect(partThread,             SIGNAL(finished()),                              partThread, SLOT(deleteLater()));
      connect(partWorkerCustomColour, SIGNAL(customColourFinishedSig()),               partThread, SLOT(quit()));
      connect(partWorkerCustomColour, SIGNAL(customColourFinishedSig()),   partWorkerCustomColour, SLOT(deleteLater()));
      connect(partWorkerCustomColour, SIGNAL(requestFinishSig()),                      partThread, SLOT(quit()));
      connect(partWorkerCustomColour, SIGNAL(requestFinishSig()),          partWorkerCustomColour, SLOT(deleteLater()));
      connect(this,                   SIGNAL(requestEndThreadNowSig()),    partWorkerCustomColour, SLOT(requestEndThreadNow()));

      connect(partWorkerCustomColour, SIGNAL(messageSig(bool,QString)),                   this, SLOT(statusMessage(bool,QString)));

      connect(partWorkerCustomColour, SIGNAL(progressBarInitSig()),                       this, SLOT(progressBarInit()));
      connect(partWorkerCustomColour, SIGNAL(progressMessageSig(QString)),                this, SLOT(progressBarSetText(QString)));
      connect(partWorkerCustomColour, SIGNAL(progressRangeSig(int,int)),                  this, SLOT(progressBarSetRange(int,int)));
      connect(partWorkerCustomColour, SIGNAL(progressSetValueSig(int)),                   this, SLOT(progressBarSetValue(int)));
      connect(partWorkerCustomColour, SIGNAL(progressResetSig()),                         this, SLOT(progressBarReset()));
      connect(partWorkerCustomColour, SIGNAL(removeProgressStatusSig()),                  this, SLOT(removeProgressStatus()));

      partThread->start();

      //qDebug() << qPrintable(QString("Sent overwrite highlight parts = %1").arg(overwriteCustomParts ? "True" : "False"));
      emit operateHighlightParts(overwriteCustomParts);
    }
}

// left side progress bar
void Gui::progressBarInit(){
    progressBar->setMaximumHeight(15);
    statusBar()->addWidget(progressLabel);
    statusBar()->addWidget(progressBar);
    progressLabel->show();
    progressBar->show();
}

void Gui::progressBarSetText(const QString &progressText)
{
  progressLabel->setText(progressText);
  QApplication::processEvents();
}
void Gui::progressBarSetRange(int minimum, int maximum)
{
  progressBar->setRange(minimum,maximum);
  QApplication::processEvents();
}
void Gui::progressBarSetValue(int value)
{
  progressBar->setValue(value);
  QApplication::processEvents();
}
void Gui::progressBarReset()
{
  progressBar->reset();
  QApplication::processEvents();
}

// right side progress bar
void Gui::progressBarPermInit(){
  progressBarPerm->setMaximumHeight(15);
  statusBar()->addPermanentWidget(progressLabelPerm);
  statusBar()->addPermanentWidget(progressBarPerm);
  progressLabelPerm->show();
  progressBarPerm->show();
  QApplication::processEvents();
}

void Gui::progressBarPermSetText(const QString &progressText)
{
progressLabelPerm->setText(progressText);
QApplication::processEvents();
}
void Gui::progressBarPermSetRange(int minimum, int maximum)
{
progressBarPerm->setRange(minimum,maximum);
QApplication::processEvents();
}
void Gui::progressBarPermSetValue(int value)
{
progressBarPerm->setValue(value);
QApplication::processEvents();
}
void Gui::progressBarPermReset()
{
progressBarPerm->reset();
QApplication::processEvents();
}

bool Gui::aboutDialog()
{
    AboutDialog Dialog(this);
    return Dialog.exec() == QDialog::Accepted;
}

void Gui::refreshLDrawUnoffParts(){

    // Download unofficial archive
    emit messageSig(true,"Refresh LDraw Unofficial Library archive...");
    UpdateCheck *libraryDownload;
    QEventLoop *wait = new QEventLoop();
    QString archivePath = tr("%1/%2").arg(Preferences::lpubDataPath, "libraries");
    libraryDownload = new UpdateCheck(this, (void*)LDrawUnofficialLibraryDownload);
    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), archivePath);
    wait->exec();

    // Extract archive
    QString archive = tr("%1/%2").arg(archivePath).arg(FILE_LPUB3D_UNOFFICIAL_ARCHIVE);
    QString destination = tr("%1/unofficial").arg(Preferences::ldrawPath);
#ifdef Q_OS_WIN
    QStringList result = JlCompress::extractDir(archive,destination);
    if (result.isEmpty()){
        emit messageSig(false,tr("Failed to extract %1 to %2").arg(archive).arg(destination));
    } else {
        QString message = tr("%1 Unofficial Library files extracted to %2").arg(result.size()).arg(destination);
        emit messageSig(true,message);
    }
#else
    QMessageBox::information(NULL,
    QMessageBox::tr(VER_PRODUCTNAME_STR),
    QMessageBox::tr("It is recommended to extract the updated unofficial archive library "
                    "%1 to your LDraw library %2").arg(archive).arg(destination));
#endif
}

void Gui::refreshLDrawOfficialParts(){

    // Download official archive
    emit messageSig(true,"Refresh LDraw Official Library archive...");
    UpdateCheck *libraryDownload;
    QEventLoop *wait = new QEventLoop();
    QString archivePath = tr("%1/%2").arg(Preferences::lpubDataPath, "libraries");
    libraryDownload = new UpdateCheck(this, (void*)LDrawOfficialLibraryDownload);
    wait->connect(libraryDownload, SIGNAL(downloadFinished(QString,QString)), wait, SLOT(quit()));
    libraryDownload->requestDownload(libraryDownload->getDEFS_URL(), archivePath);
    wait->exec();

    // Extract archive
    QString archive = tr("%1/%2").arg(archivePath).arg(VER_LDRAW_OFFICIAL_ARCHIVE);
    QString destination = Preferences::ldrawPath;
    destination = destination.remove(destination.size() - 6,6);
#ifdef Q_OS_WIN
    QStringList result = JlCompress::extractDir(archive,destination);
    if (result.isEmpty()){
        emit messageSig(false,tr("Failed to extract %1 to %2/ldraw").arg(archive).arg(destination));
    } else {
        QString message = tr("%1 Official Library files extracted to %2/ldraw").arg(result.size()).arg(destination);
        emit messageSig(true,message);
    }
#else
    QMessageBox::information(NULL,
    QMessageBox::tr(VER_PRODUCTNAME_STR),
    QMessageBox::tr("It is recommended to extract the updated Official archive library "
                    "%1 to your LDraw library %2/ldraw").arg(archive).arg(destination));
#endif
}


void Gui::updateCheck()
{
    // Create an instance of update check
    new UpdateCheck(this, (void*)SoftwareUpdate);
}

// Begin Jaco's code
// Danny: web url changed, as it pointed nowhere

#include <QDesktopServices>
#include <QUrl>

void Gui::onlineManual()
{
    QDesktopServices::openUrl(QUrl("http://sites.google.com/site/workingwithlpub/lpub3d"));
}

// End Jaco's code


void Gui::meta()
{
  Meta meta;
  QStringList doc;
  static const QString fmtDateTime("MM-dd-yyyy hh:mm:ss");

  QString fileName = QFileDialog::getSaveFileName(
    this,
    tr("Metacommands Save File Name"),
    QDir::currentPath() + "/metacommands.txt",
    tr("txt (*.txt)"));

  if (fileName == "") {
    return;
  }
  meta.doc(doc);

  QFile file(fileName);
  if (!file.open(QFile::WriteOnly | QFile::Text)) {
    QMessageBox::warning(NULL,
    QMessageBox::tr(VER_PRODUCTNAME_STR),
    QMessageBox::tr("Cannot write file %1:\n%2.")
    .arg(fileName)
    .arg(file.errorString()));
    return;
  }

  QTextStream out(&file);

  doc.prepend(QString());
  doc.prepend(QString("%1 %2 - Generated on %3")
                       .arg(VER_PRODUCTNAME_STR)
                       .arg(QFileInfo(fileName).baseName())
                       .arg(QDateTime::currentDateTime().toString(fmtDateTime)));
  doc.append(QString());
  doc.append(QString("End of file."));
  int n = 0;
  for (int i = 0; i < doc.size(); i++) {
    QString number;
    if (QString(doc[i]).startsWith('0'))
        number = QString("%1. ").arg(++n,3,10,QChar('0'));
    else
        number = QString();
    out << number << doc[i] << endl;
  }

  file.close();
}

void Gui::createActions()
{
    openAct = new QAction(QIcon(":/resources/open.png"), tr("&Open..."), this);
    openAct->setShortcut(tr("Ctrl+O"));
    openAct->setStatusTip(tr("Open an existing file"));
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(QIcon(":/resources/save.png"), tr("&Save"), this);
    saveAct->setShortcut(tr("Ctrl+S"));
    saveAct->setStatusTip(tr("Save the document to disk"));
    saveAct->setEnabled(false);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(QIcon(":/resources/saveas.png"),tr("Save A&s..."), this);
    saveAsAct->setShortcut(tr("Ctrl+Shift+S"));
    saveAsAct->setStatusTip(tr("Save the document under a new name"));
    saveAsAct->setEnabled(false);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

    closeFileAct = new QAction(QIcon(":/resources/closemodelfile.png"), tr("Cl&ose..."), this);
    closeFileAct->setShortcut(tr("Ctrl+O"));
    closeFileAct->setStatusTip(tr("Close an existing file"));
    closeFileAct->setEnabled(false);
    connect(closeFileAct, SIGNAL(triggered()), this, SLOT(closeModelFile()));

    printToFileAct = new QAction(QIcon(":/resources/file_print.png"), tr("&Print..."), this);
    printToFileAct->setShortcut(tr(""));
    printToFileAct->setStatusTip(tr("Print the current document"));
    printToFileAct->setEnabled(false);
    connect(printToFileAct, SIGNAL(triggered()), this, SLOT(ShowPrintDialog()));

    exportAsPdfPreviewAct = new QAction(QIcon(":/resources/pdf_print_preview.png"), tr("PDF Export Preview..."), this);
    exportAsPdfPreviewAct->setShortcut(tr("Alt+P"));
    exportAsPdfPreviewAct->setStatusTip(tr("Preview the current document to be printed"));
    exportAsPdfPreviewAct->setEnabled(false);
    connect(exportAsPdfPreviewAct, SIGNAL(triggered()), this, SLOT(TogglePrintPreview()));

    exportAsPdfAct = new QAction(QIcon(":/resources/pdf_logo.png"), tr("Export to PDF &File"), this);
    exportAsPdfAct->setShortcut(tr("Alt+F"));
    exportAsPdfAct->setStatusTip(tr("Export your document to a pdf file"));
    exportAsPdfAct->setEnabled(false);
    connect(exportAsPdfAct, SIGNAL(triggered()), this, SLOT(exportAsPdfDialog()));

    exportPngAct = new QAction(QIcon(":/resources/exportpng.png"),tr("Export As P&NG Images"), this);
    exportPngAct->setShortcut(tr("Alt+N"));
    exportPngAct->setStatusTip(tr("Export your document as a sequence of PNG images"));
    exportPngAct->setEnabled(false);
    connect(exportPngAct, SIGNAL(triggered()), this, SLOT(exportAsPngDialog()));

    exportJpgAct = new QAction(QIcon(":/resources/exportjpeg.png"),tr("Export As &JPEG Images"), this);
    exportJpgAct->setShortcut(tr("Alt+J"));
    exportJpgAct->setStatusTip(tr("Export your document as a sequence of JPEG images"));
    exportJpgAct->setEnabled(false);
    connect(exportJpgAct, SIGNAL(triggered()), this, SLOT(exportAsJpgDialog()));

    exportBmpAct = new QAction(QIcon(":/resources/exportbmp.png"),tr("Export As &Bitmap Images"), this);
    exportBmpAct->setShortcut(tr("Alt+B"));
    exportBmpAct->setStatusTip(tr("Export your document as a sequence of bitmap images"));
    exportBmpAct->setEnabled(false);
    connect(exportBmpAct, SIGNAL(triggered()), this, SLOT(exportAsBmpDialog()));

    exitAct = new QAction(QIcon(":/resources/exit.png"),tr("E&xit"), this);
    exitAct->setShortcut(tr("Ctrl+Q"));
    exitAct->setStatusTip(tr("Exit the application"));
    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    for (int i = 0; i < MaxRecentFiles; i++) {
      recentFilesActs[i] = new QAction(this);
      recentFilesActs[i]->setVisible(false);
      connect(recentFilesActs[i], SIGNAL(triggered()), this, 
                                 SLOT(openRecentFile()));
    }

    clearRecentAct = new QAction(tr("Clear Files"),this);
    clearRecentAct->setStatusTip(tr("Clear recent files"));
    connect(clearRecentAct, SIGNAL(triggered()), this, SLOT(clearRecentFiles()));
    // undo/redo

    undoAct = new QAction(QIcon(":/resources/editundo.png"), tr("Undo"), this);
    undoAct->setShortcut(tr("Ctrl+Z"));
    undoAct->setStatusTip(tr("Undo last change"));
    undoAct->setEnabled(false);
    connect(undoAct, SIGNAL(triggered()), this, SLOT(undo()));
    redoAct = new QAction(QIcon(":/resources/editredo.png"), tr("Redo"), this);
#ifdef __APPLE__
    redoAct->setShortcut(tr("Ctrl+Shift+Z"));
#else
    redoAct->setShortcut(tr("Ctrl+Y"));
#endif
    redoAct->setStatusTip(tr("Redo last change"));
    redoAct->setEnabled(false);
    connect(redoAct, SIGNAL(triggered()), this, SLOT(redo()));

    insertCoverPageAct = new QAction(QIcon(":/resources/insertcoverpage.png"),tr("Insert Front Cover Page"),this);
    insertCoverPageAct->setStatusTip(tr("Insert a front cover page"));
    insertCoverPageAct->setEnabled(false);
    connect(insertCoverPageAct, SIGNAL(triggered()), this, SLOT(insertCoverPage()));

    appendCoverPageAct = new QAction(QIcon(":/resources/appendcoverpage.png"),tr("Append Back Cover Page"),this);
    appendCoverPageAct->setStatusTip(tr("Append back cover page"));
    appendCoverPageAct->setEnabled(false);
    connect(appendCoverPageAct, SIGNAL(triggered()), this, SLOT(appendCoverPage()));

    insertNumberedPageAct = new QAction(QIcon(":/resources/insertpage.png"),tr("Insert Page"),this);
    insertNumberedPageAct->setStatusTip(tr("Insert a numbered page"));
    insertNumberedPageAct->setEnabled(false);
    connect(insertNumberedPageAct, SIGNAL(triggered()), this, SLOT(insertNumberedPage()));

    appendNumberedPageAct = new QAction(QIcon(":/resources/appendpage.png"),tr("Append Page"),this);
    appendNumberedPageAct->setStatusTip(tr("Append a numbered page"));
    appendNumberedPageAct->setEnabled(false);
    connect(appendNumberedPageAct, SIGNAL(triggered()), this, SLOT(appendNumberedPage()));

    deletePageAct = new QAction(QIcon(":/resources/deletepage.png"),tr("Delete Page"),this);
    deletePageAct->setStatusTip(tr("Delete this page"));
    deletePageAct->setEnabled(false);
    connect(deletePageAct, SIGNAL(triggered()), this, SLOT(deletePage()));

    addPictureAct = new QAction(QIcon(":/resources/addpicture.png"),tr("Add Picture"),this);
    addPictureAct->setStatusTip(tr("Add a picture to this page"));
    addPictureAct->setEnabled(false);
    connect(addPictureAct, SIGNAL(triggered()), this, SLOT(addPicture()));

    addTextAct = new QAction(QIcon(":/resources/addtext.png"),tr("Add Text"),this);
    addTextAct->setStatusTip(tr("Add text to this page"));
    addTextAct->setEnabled(false);
    connect(addTextAct, SIGNAL(triggered()), this, SLOT(addText()));

    addBomAct = new QAction(QIcon(":/resources/addbom.png"),tr("Add Bill of Materials"),this);
    addBomAct->setStatusTip(tr("Add Bill of Materials to this page"));
    addBomAct->setEnabled(false);
    connect(addBomAct, SIGNAL(triggered()), this, SLOT(addBom()));

    removeLPubFormattingAct = new QAction(QIcon(":/resources/removelpubformat.png"),tr("Remove LPub Formatting"),this);
    removeLPubFormattingAct->setStatusTip(tr("Remove all LPub metacommands from all files"));
    removeLPubFormattingAct->setEnabled(false);
    connect(removeLPubFormattingAct, SIGNAL(triggered()), this, SLOT(removeLPubFormatting()));

    // fitWidth,fitVisible,actualSize

    fitWidthAct = new QAction(QIcon(":/resources/fitWidth.png"), tr("Fit &Width"), this);
    fitWidthAct->setShortcut(tr("Alt+W"));
    fitWidthAct->setStatusTip(tr("Fit document to width"));
    fitWidthAct->setEnabled(false);
    connect(fitWidthAct, SIGNAL(triggered()), this, SLOT(fitWidth()));

    fitVisibleAct = new QAction(QIcon(":/resources/fitVisible.png"), tr("Fit &Visible"), this);
    fitVisibleAct->setShortcut(tr("Alt+V"));
    fitVisibleAct->setStatusTip(tr("Fit document so whole page is visible"));
    fitVisibleAct->setEnabled(false);
    connect(fitVisibleAct, SIGNAL(triggered()), this, SLOT(fitVisible()));

    actualSizeAct = new QAction(QIcon(":/resources/actual.png"),tr("&Actual Size"), this);
    actualSizeAct->setShortcut(tr("Alt+A"));
    actualSizeAct->setStatusTip(tr("Show document actual size"));
    actualSizeAct->setEnabled(false);
    connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(actualSize()));

    // TESTING ONLY
    //connect(actualSizeAct, SIGNAL(triggered()), this, SLOT(twoPages()));
    
    // zoomIn,zoomOut

    zoomInAct = new QAction(QIcon(":/resources/zoomin.png"), tr("&Zoom In"), this);
    zoomInAct->setShortcut(tr("Ctrl++"));
    zoomInAct->setStatusTip(tr("Zoom in"));
    zoomInAct->setEnabled(false);
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));

    zoomOutAct = new QAction(QIcon(":/resources/zoomout.png"),tr("Zoom &Out"),this);
    zoomOutAct->setShortcut(tr("Ctrl+-"));
    zoomOutAct->setStatusTip(tr("Zoom out"));
    zoomOutAct->setEnabled(false);
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));

    // firstPage,lastPage,nextPage,previousPage

    firstPageAct = new QAction(QIcon(":/resources/first.png"),tr("First Page"), this);
    firstPageAct->setShortcut(tr("Ctrl+P"));
    firstPageAct->setStatusTip(tr("Go to first page of document"));
    firstPageAct->setEnabled(false);
    connect(firstPageAct, SIGNAL(triggered()), this, SLOT(firstPage()));

    lastPageAct = new QAction(QIcon(":/resources/last.png"),tr("Last Page"), this);
    lastPageAct->setShortcut(tr("Ctrl+L"));
    lastPageAct->setStatusTip(tr("Go to last page of document"));
    lastPageAct->setEnabled(false);
    connect(lastPageAct, SIGNAL(triggered()), this, SLOT(lastPage()));

    nextPageAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),this);
    nextPageAct->setShortcut(tr("Ctrl+N"));
    nextPageAct->setStatusTip(tr("Go to next page of document"));
    nextPageAct->setEnabled(false);
    connect(nextPageAct, SIGNAL(triggered()), this, SLOT(nextPage()));

    previousPageAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),this);
    previousPageAct->setShortcut(tr("Ctrl+E"));
    previousPageAct->setStatusTip(tr("Go to previous page of document"));
    previousPageAct->setEnabled(false);
    connect(previousPageAct, SIGNAL(triggered()), this, SLOT(previousPage()));

    // nextContinuousPage,previousContinuousPage
    nextPageComboAct = new QAction(QIcon(":/resources/next.png"),tr("&Next Page"),this);
    nextPageComboAct->setStatusTip(tr("Go to next page of document"));
    nextPageComboAct->setEnabled(false);
    connect(nextPageComboAct, SIGNAL(triggered()), this, SLOT(nextPage()));

    previousPageComboAct = new QAction(QIcon(":/resources/prev.png"),tr("&Previous Page"),this);
    previousPageComboAct->setStatusTip(tr("Go to previous page of document"));
    previousPageComboAct->setEnabled(false);
    connect(previousPageComboAct, SIGNAL(triggered()), this, SLOT(previousPage()));

    nextPageContinuousAct = new QAction(QIcon(":/resources/nextpagecontinuous.png"),tr("Continuous Next Page"),this);
    nextPageContinuousAct->setShortcut(tr("Ctrl+Shift+N"));
    nextPageContinuousAct->setStatusTip(tr("Continuously process next document page"));
    nextPageContinuousAct->setEnabled(false);
    connect(nextPageContinuousAct, SIGNAL(triggered()), this, SLOT(nextPageContinuous()));

    previousPageContinuousAct = new QAction(QIcon(":/resources/prevpagecontinuous.png"),tr("Continuous Previous Page"),this);
    previousPageContinuousAct->setShortcut(tr("Ctrl+Shift+E"));
    previousPageContinuousAct->setStatusTip(tr("Continuously process previous document page"));
    previousPageContinuousAct->setEnabled(false);
    connect(previousPageContinuousAct, SIGNAL(triggered()), this, SLOT(previousPageContinuous()));

    QString pageString = "";
    setPageLineEdit = new QLineEdit(pageString,this);
    QSize size = setPageLineEdit->sizeHint();
    size.setWidth(size.width()/3);
    setPageLineEdit->setMinimumSize(size);
    setPageLineEdit->setToolTip("Current Page Index");
    setPageLineEdit->setStatusTip("Enter index and hit enter to go to page");
    setPageLineEdit->setEnabled(false);
    connect(setPageLineEdit, SIGNAL(returnPressed()), this, SLOT(setPage()));

    clearPLICacheAct = new QAction(QIcon(":/resources/clearplicache.png"),tr("Reset Parts Image Cache"), this);
    clearPLICacheAct->setStatusTip(tr("Reset the parts list image cache"));
    connect(clearPLICacheAct, SIGNAL(triggered()), this, SLOT(clearPLICache()));

    clearCSICacheAct = new QAction(QIcon(":/resources/clearcsicache.png"),tr("Reset Assembly Image Cache"), this);
    clearCSICacheAct->setStatusTip(tr("Reset the assembly image cache"));
    connect(clearCSICacheAct, SIGNAL(triggered()), this, SLOT(clearCSICache()));

    clearTempCacheAct = new QAction(QIcon(":/resources/clearcsi3dcache.png"),tr("Reset Temp File Cache"), this);
    clearTempCacheAct->setStatusTip(tr("Reset the Temp file and 3D viewer image cache"));
    connect(clearTempCacheAct, SIGNAL(triggered()), this, SLOT(clearTempCache()));

    clearAllCachesAct = new QAction(QIcon(":/resources/clearimagemodelcache.png"),tr("Reset Model Caches"), this);
    clearAllCachesAct->setStatusTip(tr("Reset model file Parts, Assembly and Temp file caches"));
    connect(clearAllCachesAct, SIGNAL(triggered()), this, SLOT(clearAllCaches()));

    clearFadeCacheAct = new QAction(QIcon(":/resources/clearcustompartcache.png"),tr("Reset Custom Files Cache"), this);
    clearFadeCacheAct->setStatusTip(tr("Reset fade and highlight part files cache"));
    connect(clearFadeCacheAct, SIGNAL(triggered()), this, SLOT(clearCustomPartCache()));

    refreshLDrawUnoffPartsAct = new QAction(QIcon(":/resources/refreshunoffarchive.png"),tr("Refresh LDraw Unofficial Parts"), this);
    refreshLDrawUnoffPartsAct->setStatusTip(tr("Download and replace LDraw Unofficial parts archive file"));
    connect(refreshLDrawUnoffPartsAct, SIGNAL(triggered()), this, SLOT(refreshLDrawUnoffParts()));

    refreshLDrawOfficialPartsAct = new QAction(QIcon(":/resources/refreshoffarchive.png"),tr("Refresh LDraw Official Parts"), this);
    refreshLDrawOfficialPartsAct->setStatusTip(tr("Download and replace LDraw Official parts archive file"));
    connect(refreshLDrawOfficialPartsAct, SIGNAL(triggered()), this, SLOT(refreshLDrawOfficialParts()));

    // Config menu

    pageSetupAct = new QAction(QIcon(":/resources/pagesetup.png"),tr("Page Setup"), this);
    pageSetupAct->setEnabled(false);
    pageSetupAct->setStatusTip(tr("Default values for your project's pages"));
    connect(pageSetupAct, SIGNAL(triggered()), this, SLOT(pageSetup()));

    assemSetupAct = new QAction(QIcon(":/resources/assemblysetup.png"),tr("Assembly Setup"), this);
    assemSetupAct->setEnabled(false);
    assemSetupAct->setStatusTip(tr("Default values for your project's assembly images"));
    connect(assemSetupAct, SIGNAL(triggered()), this, SLOT(assemSetup()));

    pliSetupAct = new QAction(QIcon(":/resources/partslistsetup.png"),tr("Parts List Setup"), this);
    pliSetupAct->setEnabled(false);
    pliSetupAct->setStatusTip(tr("Default values for your project's parts lists"));
    connect(pliSetupAct, SIGNAL(triggered()), this, SLOT(pliSetup()));

    bomSetupAct = new QAction(QIcon(":/resources/bomsetup.png"),tr("Bill of Materials Setup"), this);
    bomSetupAct->setEnabled(false);
    bomSetupAct->setStatusTip(tr("Default values for your project's bill of materials"));
    connect(bomSetupAct, SIGNAL(triggered()), this, SLOT(bomSetup()));

    calloutSetupAct = new QAction(QIcon(":/resources/calloutsetup.png"),tr("Callout Setup"), this);
    calloutSetupAct->setEnabled(false);
    calloutSetupAct->setStatusTip(tr("Default values for your project's callouts"));
    connect(calloutSetupAct, SIGNAL(triggered()), this, SLOT(calloutSetup()));

    multiStepSetupAct = new QAction(QIcon(":/resources/stepgroupsetup.png"),tr("Step Group Setup"), this);
    multiStepSetupAct->setEnabled(false);
    multiStepSetupAct->setStatusTip(tr("Default values for your project's step groups"));
    connect(multiStepSetupAct, SIGNAL(triggered()), this, SLOT(multiStepSetup()));

    projectSetupAct = new QAction(QIcon(":/resources/projectsetup.png"),tr("Project Setup"), this);
    projectSetupAct->setEnabled(false);
    projectSetupAct->setStatusTip(tr("Default values for your project"));
    connect(projectSetupAct, SIGNAL(triggered()), this, SLOT(projectSetup()));

    fadeStepSetupAct = new QAction(QIcon(":/resources/fadestepsetup.png"),tr("Fade Step Setup"), this);
    fadeStepSetupAct->setEnabled(false);
    fadeStepSetupAct->setStatusTip(tr("Fade parts in previous step step"));
    connect(fadeStepSetupAct, SIGNAL(triggered()), this, SLOT(fadeStepSetup()));

    highlightStepSetupAct = new QAction(QIcon(":/resources/highlightstepsetup.png"),tr("Highlight Step Setup"), this);
    highlightStepSetupAct->setEnabled(false);
    highlightStepSetupAct->setStatusTip(tr("Highlight parts in current step"));
    connect(highlightStepSetupAct, SIGNAL(triggered()), this, SLOT(highlightStepSetup()));

    preferencesAct = new QAction(QIcon(":/resources/preferences.png"),tr("Preferences"), this);
    preferencesAct->setStatusTip(tr("Set your preferences for LPub3D"));
    connect(preferencesAct, SIGNAL(triggered()), this, SLOT(preferences()));

    editTitleAnnotationsAct = new QAction(QIcon(":/resources/edittitleannotations.png"),tr("Edit Part Title PLI Annotations"), this);
    editTitleAnnotationsAct->setStatusTip(tr("Add/Edit part title PLI part annotatons"));
    connect(editTitleAnnotationsAct, SIGNAL(triggered()), this, SLOT(editTitleAnnotations()));

    editFreeFormAnnitationsAct = new QAction(QIcon(":/resources/editfreeformannotations.png"),tr("Edit Freeform PLI Annotations"), this);
    editFreeFormAnnitationsAct->setStatusTip(tr("Add/Edit freeform PLI part annotations"));
    connect(editFreeFormAnnitationsAct, SIGNAL(triggered()), this, SLOT(editFreeFormAnnitations()));

    editLDrawColourPartsAct = new QAction(QIcon(":/resources/editldrawcolourparts.png"),tr("Edit LDraw Static Colour Parts List"), this);
    editLDrawColourPartsAct->setStatusTip(tr("Add/Edit the list of LDraw static colour parts used to process fade and highlight steps"));
    connect(editLDrawColourPartsAct, SIGNAL(triggered()), this, SLOT(editLDrawColourParts()));

    editPliBomSubstitutePartsAct = new QAction(QIcon(":/resources/editplisubstituteparts.png"),tr("Edit PLI/BOM Substitute Parts List"), this);
    editPliBomSubstitutePartsAct->setStatusTip(tr("Add/Edit the list of PLI/BOM substitute parts"));
    connect(editPliBomSubstitutePartsAct, SIGNAL(triggered()), this, SLOT(editPliBomSubstituteParts()));

    editExcludedPartsAct = new QAction(QIcon(":/resources/editexcludedparts.png"),tr("Edit Part Count Excluded Parts List"), this);
    editExcludedPartsAct->setStatusTip(tr("Add/Edit the list of part count excluded parts"));
    connect(editExcludedPartsAct, SIGNAL(triggered()), this, SLOT(editExcludedParts()));

    editLdrawIniFileAct = new QAction(QIcon(":/resources/editinifile.png"),tr("Edit LDraw.ini"), this);
    editLdrawIniFileAct->setStatusTip(tr("Add/Edit LDraw.ini search directory entries"));
    connect(editLdrawIniFileAct, SIGNAL(triggered()), this, SLOT(editLdrawIniFile()));

    editLdgliteIniAct = new QAction(QIcon(":/resources/editldgliteconf.png"),tr("Edit LDGLite ini configuration file"), this);
    editLdgliteIniAct->setStatusTip(tr("Edit LDGLite ini configuration file"));
    connect(editLdgliteIniAct, SIGNAL(triggered()), this, SLOT(editLdgliteIni()));

    editLdviewIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("Edit LDView ini configuration file"), this);
    editLdviewIniAct->setStatusTip(tr("Edit LDView ini configuration file"));
    connect(editLdviewIniAct, SIGNAL(triggered()), this, SLOT(editLdviewIni()));

    editLdviewPovIniAct = new QAction(QIcon(":/resources/editldviewconf.png"),tr("Edit LDView POV generation configuration file"), this);
    editLdviewPovIniAct->setStatusTip(tr("Edit LDView POV generation configuration file"));
    connect(editLdviewPovIniAct, SIGNAL(triggered()), this, SLOT(editLdviewPovIni()));

    editPovrayIniAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("Edit Raytracer (POV-Ray) ini configuration file"), this);
    editPovrayIniAct->setStatusTip(tr("Edit Raytracer (POV-Ray) ini configuration file"));
    connect(editPovrayIniAct, SIGNAL(triggered()), this, SLOT(editPovrayIni()));

    editPovrayConfAct = new QAction(QIcon(":/resources/editpovrayconf.png"),tr("Edit Raytracer (POV-Ray) file access configuration file"), this);
    editPovrayConfAct->setStatusTip(tr("Edit Raytracer (POV-Ray) file access configuration file"));
    connect(editPovrayConfAct, SIGNAL(triggered()), this, SLOT(editPovrayConf()));

    generateCustomColourPartsAct = new QAction(QIcon(":/resources/generatecolourparts.png"),tr("Generate Static Colour Parts List"), this);
    generateCustomColourPartsAct->setStatusTip(tr("Generate list of all static coloured parts"));
    connect(generateCustomColourPartsAct, SIGNAL(triggered()), this, SLOT(generateCustomColourPartsList()));

    // Help

    aboutAct = new QAction(QIcon(":/resources/LPub32.png"),tr("&About %1...").arg(VER_PRODUCTNAME_STR), this);
    aboutAct->setStatusTip(tr("Show the application's About box"));
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    // Begin Jaco's code

    onlineManualAct = new QAction(QIcon(":/resources/onlinemanual.png"),tr("&Online manual"), this);
    onlineManualAct->setStatusTip(tr("Visit the Online Manual Website."));
    connect(onlineManualAct, SIGNAL(triggered()), this, SLOT(onlineManual()));

    // End Jaco's code

    metaAct = new QAction(QIcon(":/resources/savemetacommands.png"),tr("&Save LPub Metacommands to File"), this);
    metaAct->setStatusTip(tr("Save a list of the known LPub meta commands to a file"));
    connect(metaAct, SIGNAL(triggered()), this, SLOT(meta()));

    updateAppAct = new QAction(QIcon(":/resources/softwareupdate.png"),tr("Check for &Updates..."), this);
    updateAppAct->setStatusTip(tr("Check if a newer version of  %1 is available for download").arg(VER_PRODUCTNAME_STR));
    connect(updateAppAct, SIGNAL(triggered()), this, SLOT(updateCheck()));

    viewLogAct = new QAction(QIcon(":/resources/viewlog.png"),tr("View %1 log").arg(VER_PRODUCTNAME_STR), this);
    viewLogAct->setStatusTip(tr("View %1 log").arg(VER_PRODUCTNAME_STR));
    connect(viewLogAct, SIGNAL(triggered()), this, SLOT(viewLog()));
}

void Gui::loadPages(){

  MetaItem mi;
  int pageNum     = 0;
  setGoToPageCombo->setMaxCount(0);
  setGoToPageCombo->setMaxCount(1000);
  bool frontCoverPage = mi.frontCoverPageExist();
  bool backCoverPage  = mi.backCoverPageExist();

  for(int i=1;i <= maxPages;i++){
      QApplication::processEvents();
      pageNum++;
      if (frontCoverPage && i == 1){
          pageNum--;
          setGoToPageCombo->addItem(QString("Front Cover"));
        }
      else if (backCoverPage && i == maxPages){
          setGoToPageCombo->addItem(QString("Back Cover"));
        }
      else
        setGoToPageCombo->addItem(QString("Page %1").arg(QString::number(pageNum)));
    }
  setGoToPageCombo->setCurrentIndex(displayPageNum-1);
}

void Gui::enableActions()
{
  saveAsAct->setEnabled(true);
  closeFileAct->setEnabled(true);

  exportAsPdfAct->setEnabled(true);
  printToFileAct->setEnabled(true);
  exportAsPdfPreviewAct->setEnabled(true);

  exportPngAct->setEnabled(true);
  exportJpgAct->setEnabled(true);
  exportBmpAct->setEnabled(true);

  pageSetupAct->setEnabled(true);
  assemSetupAct->setEnabled(true);
  pliSetupAct->setEnabled(true);
  bomSetupAct->setEnabled(true);
  calloutSetupAct->setEnabled(true);
  multiStepSetupAct->setEnabled(true);
  projectSetupAct->setEnabled(true);
  fadeStepSetupAct->setEnabled(true);
  highlightStepSetupAct->setEnabled(true);

  addPictureAct->setEnabled(true);
  removeLPubFormattingAct->setEnabled(true);

  editTitleAnnotationsAct->setEnabled(true);
  editFreeFormAnnitationsAct->setEnabled(true);
  editLDrawColourPartsAct->setEnabled(true);
  editPliBomSubstitutePartsAct->setEnabled(true);
  editExcludedPartsAct->setEnabled(true);
  editLdgliteIniAct->setEnabled(true);
  editLdviewIniAct->setEnabled(true);
  editLdviewPovIniAct->setEnabled(true);
  editPovrayIniAct->setEnabled(true);
  editPovrayConfAct->setEnabled(true);

  setPageLineEdit->setEnabled(true);

  firstPageAct->setEnabled(true);
  lastPageAct->setEnabled(true);
  nextPageAct->setEnabled(true);
  previousPageAct->setEnabled(true);
  nextPageComboAct->setEnabled(true);
  previousPageComboAct->setEnabled(true);
  nextPageContinuousAct->setEnabled(true);
  previousPageContinuousAct->setEnabled(true);

  fitWidthAct->setEnabled(true);
  fitVisibleAct->setEnabled(true);
  actualSizeAct->setEnabled(true);
  zoomInAct->setEnabled(true);
  zoomOutAct->setEnabled(true);

  cacheMenu->setEnabled(true);
  exportMenu->setEnabled(true);
  ExportMenuViewer->setEnabled(true);

}

void Gui::disableActions()
{
  saveAsAct->setEnabled(false);
  closeFileAct->setEnabled(false);

  exportAsPdfAct->setEnabled(false);
  printToFileAct->setEnabled(false);
  exportAsPdfPreviewAct->setEnabled(false);

  exportPngAct->setEnabled(false);
  exportJpgAct->setEnabled(false);
  exportBmpAct->setEnabled(false);

  pageSetupAct->setEnabled(false);
  assemSetupAct->setEnabled(false);
  pliSetupAct->setEnabled(false);
  bomSetupAct->setEnabled(false);
  calloutSetupAct->setEnabled(false);
  multiStepSetupAct->setEnabled(false);
  projectSetupAct->setEnabled(false);
  fadeStepSetupAct->setEnabled(false);
  highlightStepSetupAct->setEnabled(false);

  addPictureAct->setEnabled(false);
  removeLPubFormattingAct->setEnabled(false);

  editTitleAnnotationsAct->setEnabled(false);
  editFreeFormAnnitationsAct->setEnabled(false);
  editLDrawColourPartsAct->setEnabled(false);
  editPliBomSubstitutePartsAct->setEnabled(false);
  editExcludedPartsAct->setEnabled(false);
  editLdgliteIniAct->setEnabled(false);
  editLdviewIniAct->setEnabled(false);
  editLdviewPovIniAct->setEnabled(false);
  editPovrayIniAct->setEnabled(false);
  editPovrayConfAct->setEnabled(false);

  setPageLineEdit->setEnabled(false);

  firstPageAct->setEnabled(false);
  lastPageAct->setEnabled(false);
  nextPageAct->setEnabled(false);
  previousPageAct->setEnabled(false);
  nextPageComboAct->setEnabled(false);
  previousPageComboAct->setEnabled(false);
  nextPageContinuousAct->setEnabled(false);
  previousPageContinuousAct->setEnabled(false);

  fitWidthAct->setEnabled(false);
  fitVisibleAct->setEnabled(false);
  actualSizeAct->setEnabled(false);
  zoomInAct->setEnabled(false);
  zoomOutAct->setEnabled(false);

  cacheMenu->setEnabled(false);
  exportMenu->setEnabled(false);

  ExportMenuViewer->setEnabled(false);

}

void Gui::enableActions2()
{
    MetaItem mi;
    insertCoverPageAct->setEnabled(mi.okToInsertCoverPage() &&
                                   ! mi.frontCoverPageExist());
    appendCoverPageAct->setEnabled(mi.okToAppendCoverPage() &&
                                   ! mi.backCoverPageExist());
    bool frontCover = mi.okToInsertNumberedPage();
    insertNumberedPageAct->setEnabled(frontCover);
    bool backCover = mi.okToAppendNumberedPage();
    appendNumberedPageAct->setEnabled(backCover);
    deletePageAct->setEnabled(page.list.size() == 0);
    addBomAct->setEnabled(frontCover||backCover);
    addTextAct->setEnabled(true);

    loadPages();
}

void Gui::disableActions2()
{
    insertCoverPageAct->setEnabled(false);
    appendCoverPageAct->setEnabled(false);
    insertNumberedPageAct->setEnabled(false);
    appendNumberedPageAct->setEnabled(false);
    deletePageAct->setEnabled(false);
    addBomAct->setEnabled(false);
    addTextAct->setEnabled(false);
}

void Gui::createMenus()
{
  // Editor Menus

    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAct);
    fileMenu->addAction(saveAct);
    fileMenu->addAction(saveAsAct);
    fileMenu->addAction(closeFileAct);

    exportMenu = fileMenu->addMenu("Export As...");
    exportMenu->setIcon(QIcon(":/resources/exportas.png"));
    exportMenu->addAction(exportPngAct);
    exportMenu->addAction(exportJpgAct);
    exportMenu->addAction(exportBmpAct);
#ifdef Q_OS_WIN
    exportMenu->addAction(exportBmpAct);
#endif
    exportMenu->setDisabled(true);

    //fileMenu->addAction(printToFileAct);
    fileMenu->addAction(exportAsPdfPreviewAct);
    fileMenu->addAction(exportAsPdfAct);
    fileMenu->addSeparator();

    recentFileMenu = fileMenu->addMenu(tr("Recent Files..."));
    recentFileMenu->setIcon(QIcon(":/resources/recentfiles.png"));
    for (int i = 0; i < MaxRecentFiles; i++) {
      recentFileMenu->addAction(recentFilesActs[i]);
    }
    separatorAct = recentFileMenu->addSeparator();
    recentFileMenu->addAction(clearRecentAct);

    fileMenu->addSeparator();
    fileMenu->addAction(exitAct);

    menuBar()->addSeparator();

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);

    editMenu->addSeparator();

    editMenu->addAction(insertCoverPageAct);
    editMenu->addAction(appendCoverPageAct);
    editMenu->addAction(insertNumberedPageAct);
    editMenu->addAction(appendNumberedPageAct);
    editMenu->addAction(deletePageAct);
    editMenu->addAction(addPictureAct);
    editMenu->addAction(addTextAct);
    editMenu->addAction(addBomAct);
    editMenu->addAction(removeLPubFormattingAct);

    editMenu->addSeparator();

    QMenu* ToolBarEditorMenu = editMenu->addMenu(tr("LDraw File Editor T&oolbar"));
    ToolBarEditorMenu->addAction(editWindow->editToolBar->toggleViewAction());

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(fitWidthAct);
    viewMenu->addAction(fitVisibleAct);
    viewMenu->addAction(actualSizeAct);
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);

    viewMenu->addSeparator();

    toolsMenu = menuBar()->addMenu(tr("&Tools"));
    toolsMenu->addAction(firstPageAct);

    toolsMenu->addAction(nextPageAct);
    toolsMenu->addAction(nextPageContinuousAct);

    toolsMenu->addAction(previousPageAct);
    toolsMenu->addAction(previousPageContinuousAct);

    toolsMenu->addAction(lastPageAct);

    toolsMenu->addSeparator();

    cacheMenu = toolsMenu->addMenu("Reset Cache");
    cacheMenu->setIcon(QIcon(":/resources/resetcache.png"));
    toolsMenu->addAction(refreshLDrawUnoffPartsAct);
    toolsMenu->addAction(refreshLDrawOfficialPartsAct);
    cacheMenu->addAction(clearAllCachesAct);
    cacheMenu->addAction(clearPLICacheAct);
    cacheMenu->addAction(clearCSICacheAct);
    cacheMenu->addAction(clearTempCacheAct);
    cacheMenu->addAction(clearFadeCacheAct);
    cacheMenu->setDisabled(true);

    configMenu = menuBar()->addMenu(tr("&Configuration"));
    configMenu->addAction(pageSetupAct);
    configMenu->addAction(assemSetupAct);
    configMenu->addAction(pliSetupAct);
    configMenu->addAction(bomSetupAct);
    configMenu->addAction(calloutSetupAct);
    configMenu->addAction(multiStepSetupAct);
    configMenu->addAction(projectSetupAct);
    configMenu->addAction(fadeStepSetupAct);
    configMenu->addAction(highlightStepSetupAct);
    configMenu->addSeparator();
    editorMenu = configMenu->addMenu("Edit Parameter Files");
    editorMenu->setIcon(QIcon(":/resources/editparameterfiles.png"));
    editorMenu->addAction(editLDrawColourPartsAct);
    editorMenu->addAction(editTitleAnnotationsAct);
    editorMenu->addAction(editFreeFormAnnitationsAct);
    editorMenu->addAction(editPliBomSubstitutePartsAct);
    editorMenu->addAction(editExcludedPartsAct);
    if (Preferences::ldrawiniFound){
      editorMenu->addAction(editLdrawIniFileAct);
    }
    editorMenu->addSeparator();
    editorMenu->addAction(editLdgliteIniAct);
    editorMenu->addAction(editLdviewIniAct);
    editorMenu->addAction(editLdviewPovIniAct);
    editorMenu->addAction(editPovrayIniAct);
    editorMenu->addAction(editPovrayConfAct);
    configMenu->addAction(generateCustomColourPartsAct);
    configMenu->addSeparator();
    configMenu->addAction(preferencesAct);

   // 3DViewer Menus
    ViewerMenu = menuBar()->addMenu(tr("&3DViewer"));
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_PREFERENCES]);
    ViewerMenu->addSeparator();
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_ZOOM_EXTENTS]);
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_LOOK_AT]);

    ViewerMenu->addMenu(gMainWindow->GetViewpointMenu());
    ViewerMenu->addMenu(gMainWindow->GetCameraMenu());
    ViewerMenu->addMenu(gMainWindow->GetProjectionMenu());
    ViewerMenu->addMenu(gMainWindow->GetShadingMenu());

    ViewerMenu->addSeparator();
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_HORIZONTAL]);
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_SPLIT_VERTICAL]);
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_REMOVE_VIEW]);
    ViewerMenu->addAction(gMainWindow->mActions[LC_VIEW_RESET_VIEWS]);
    ViewerMenu->addSeparator();

    QMenu* ToolBarViewerMenu = ViewerMenu->addMenu(tr("3DViewer Too&lbar"));
    ToolBarViewerMenu->addAction(gMainWindow->mToolsToolBar->toggleViewAction());

    FileMenuViewer = menuBar()->addMenu(tr("&Step"));
    FileMenuViewer->addAction(gMainWindow->mActions[LC_FILE_SAVEAS]);
    FileMenuViewer->addAction(gMainWindow->mActions[LC_FILE_SAVE_IMAGE]);

    ExportMenuViewer = FileMenuViewer->addMenu(tr("&Export Step As"));
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_3DS]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_BRICKLINK]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_COLLADA]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_CSV]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_HTML]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_POVRAY]);
    ExportMenuViewer->addAction(gMainWindow->mActions[LC_FILE_EXPORT_WAVEFRONT]);
    ExportMenuViewer->setDisabled(true);
    // 3D Viewer Menus End

    // Help Menus

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(viewLogAct);
#ifndef DISABLE_UPDATE_CHECK
    helpMenu->addAction(updateAppAct);
#endif
    // Begin Jaco's code
    helpMenu->addAction(onlineManualAct);
    // End Jaco's code
    helpMenu->addAction(metaAct);
    helpMenu->addSeparator();
    // About Editor
    helpMenu->addAction(aboutAct);
    // About 3D Viewer
    helpMenu->addAction(gMainWindow->mActions[LC_HELP_ABOUT]);
}

void Gui::createToolBars()
{
    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName("FileToolbar");
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(saveAct);

    //fileToolBar->addAction(printToFileAct);
    fileToolBar->addAction(exportAsPdfPreviewAct);
    fileToolBar->addAction(exportAsPdfAct);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->setObjectName("EditToolbar");
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);

    navigationToolBar = addToolBar(tr("Navigation"));
    navigationToolBar->setObjectName("NavigationToolbar");
    navigationToolBar->addAction(firstPageAct);
    previousPageContinuousMenu = new QMenu(tr("Continuous Page Previous"), this);
    previousPageContinuousMenu->addAction(previousPageContinuousAct);
    previousPageComboAct->setMenu(previousPageContinuousMenu);
    navigationToolBar->addAction(previousPageComboAct);
    navigationToolBar->addWidget(setPageLineEdit);
    nextPageContinuousMenu = new QMenu(tr("Continuous Page Next"), this);
    nextPageContinuousMenu->addAction(nextPageContinuousAct);
    nextPageComboAct->setMenu(nextPageContinuousMenu);
    navigationToolBar->addAction(nextPageComboAct);
    navigationToolBar->addAction(lastPageAct);
    navigationToolBar->addWidget(setGoToPageCombo);

    mpdToolBar = addToolBar(tr("MPD"));
    mpdToolBar->setObjectName("MPDToolbar");
    mpdToolBar->addWidget(mpdCombo);

    zoomToolBar = addToolBar(tr("Zoom"));
    zoomToolBar->setObjectName("ZoomToolbar");
    zoomToolBar->addAction(fitVisibleAct);
    zoomToolBar->addAction(fitWidthAct);
// Jaco add actual size icon. Was missing.
    zoomToolBar->addAction(actualSizeAct);
    zoomToolBar->addAction(zoomInAct);
    zoomToolBar->addAction(zoomOutAct);
}

void Gui::statusBarMsg(QString msg)
{
  statusBar()->showMessage(msg);
}

void Gui::createStatusBar()
{

  statusBar()->showMessage(tr("Ready"));
  connect(gMainWindow->mLCStatusBar, SIGNAL(messageChanged(QString)), this, SLOT(showLCStatusMessage()));

}

void Gui::showLCStatusMessage(){

    if(!modelDockWindow->isFloating())
    statusBarMsg(gMainWindow->mLCStatusBar->currentMessage());
}

void Gui::createDockWindows()
{
    fileEditDockWindow = new QDockWidget(trUtf8(wCharToUtf8("LDraw\u2122 File Editor")), this);
    modelDockWindow = new QDockWidget(trUtf8(wCharToUtf8("3DViewer - by LeoCAD\u00A9")), this);
    fileEditDockWindow->setObjectName("LDrawFileDockWindow");
    fileEditDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    fileEditDockWindow->setWidget(editWindow);
    addDockWidget(Qt::RightDockWidgetArea, fileEditDockWindow);
    viewMenu->addAction(fileEditDockWindow->toggleViewAction());
//** 3DViewer Window
    modelDockWindow->setObjectName("ModelDockWindow");
    modelDockWindow->setAllowedAreas(
                Qt::TopDockWidgetArea  | Qt::BottomDockWidgetArea |
                Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    modelDockWindow->setWidget(gMainWindow);
    addDockWidget(Qt::RightDockWidgetArea, modelDockWindow);
    viewMenu->addAction(modelDockWindow->toggleViewAction());
//**
    tabifyDockWidget(modelDockWindow, fileEditDockWindow);
#ifdef Q_OS_MAC
    // modelDock window Hack is not stable on macOS so start with fileEdit Window until I figure out what's wrong.
    fileEditDockWindow->raise();
#else
    modelDockWindow->raise();
#endif

    connect(modelDockWindow, SIGNAL (topLevelChanged(bool)), this, SLOT (toggleLCStatusBar()));
}

void Gui::toggleLCStatusBar(){

    if(modelDockWindow->isFloating())
        gMainWindow->statusBar()->show();
    else
        gMainWindow->statusBar()->hide();
}

void Gui::readSettings()
{
    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    restoreState(Settings.value("State").toByteArray());
    QSize size = Settings.value("Size", QDesktopWidget().availableGeometry(this).size()*0.6).toSize();
    const QByteArray geometry = Settings.value("Geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty()) {
        const QRect availableGeometry = QApplication::desktop()->availableGeometry(this);
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2,
             (availableGeometry.height() - height()) / 2);
    } else {
        restoreGeometry(geometry);
    }
    resize(size);
    Settings.endGroup();
}

void Gui::writeSettings()
{
    QSettings Settings;
    Settings.beginGroup(MAINWINDOW);
    Settings.setValue("Geometry", saveGeometry());
    Settings.setValue("State", saveState());
    Settings.setValue("Size", size());
    Settings.endGroup();
}
