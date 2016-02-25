/*
Title:    kLAME GUI frontend wrapper for LAME - conversion control wav to mp3
*/

/***************************************************************************
 *   Copyright (C) 2006 by Ken Sarkies                                     *
 *   ksarkies@trinity.asn.au                                               *
 *                                                                         *
 *   This file is part of kLAME.                                           *
 *                                                                         *
 *   kLAME is free software; you can redistribute it and/or modify         *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   kLAME is distributed in the hope that it will be useful,              *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with kLAME if not, write to the                                 *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.             *
 ***************************************************************************/

#ifndef KLAMEMAINFORM_H
#define KLAMEMAINFORM_H

#include "ui_klamemainformbase.h"
#include <QMainWindow>
#include <QCloseEvent>
#include <QThread>
#include <QProgressDialog>
#include "lame.h"

const int INPUT_BLOCK_SIZE = 1152;
// Recommended maximum size to hold conversion from wav to mp3
const uint OUTPUT_BLOCK_SIZE = 5*INPUT_BLOCK_SIZE/4+7200;

//-----------------------------------------------------------------------------
/** @brief kLAME Main form window

This class provides the main window. The files to be converted and the
different LAME settings are stored in a display array with files to be
converted in rows and lame settings in columns, resulting in a matrix of output
files. The display is established in a QTableWidget, which is an item based
table view with a default model.

This class manages stored directory paths and filenames, ensuring that they
remain persistent throughout the kLAME session, and are saved and loaded
between sessions.

The project file contains information about the different LAME settings to be
applied, their number and value, as well as the directories in which the input
and output files are placed.

Output filenames are constructed from the input filename. To distinguish between
output files having the same root name but different LAME settings, a tag
suffix can be defined for each set of LAME settings.

@todo Accept other input forms, such as raw PCM.
@todo Provide other output forms such as mp3 back to wav etc.
*/

class KLameMainForm : public QMainWindow
{
    Q_OBJECT
public:
    KLameMainForm(QWidget* parent = 0);
    ~KLameMainForm();
private slots:
    void on_actionNewProject_triggered();
    void on_actionOpenProject_triggered();
    void on_actionSaveProject_triggered();
    void on_actionAddFiles_triggered();
    void on_actionRemoveFile_triggered();
    void on_actionOptions_triggered();
    void on_actionAddColumn_triggered();
    void on_actionDeleteColumn_triggered();
    void on_actionConvertFiles_triggered();
    void on_actionInstructions_triggered();
    void on_actionAbout_triggered();
    void on_actionQuit_triggered();
private:
    void closeEvent(QCloseEvent*);      // subclass: catch any window close
    void saveSettings();                // Saves user's setting on exit
    void loadSettings();                // Load's users settings at start
    QString wavDirectory_;              //!< Directory holding wav files.
    QString settingsDirectory_;         //!< Directory to store settings.
    QString projectsDirectory_;         //!< Directory holding project files.
    QStringList lameOptionsList_;       //!< Options for LAME (each column).
    QString projectFile_;               //!< File with kLAME project details.
    QStringList headerLabels_;          //!< Headers for the column display.
    QStringList outputDirectoryList_;   //!< Output directories (each column).
    QStringList filenameTagList_;       //!< File name tags (each column).
    QStringList commentList_;           //!< Comments (each column).
    Ui::KLameMainFormBase mainFormUi;   // User Interface object
};

//-----------------------------------------------------------------------------
/** @brief Converter thread class

The Converter class runs the threaded code for LAME conversion of a single WAV
file.
 */

class Converter : public QThread
{
    Q_OBJECT
public:
    Converter(): returnCode_("OK"),isConversionCancelled_(false) {};
    virtual void run();                     // Reimplemented to do the work
    QString getReturnCode() const;          // Access to error messages
    void setLameFlags(lame_global_flags* flags);    // Set thread parameters
    void setInputFileName(QString inputFile);		// WAV file input
    void setOutputFileName(QString outputFile);		// mp3 file output
signals:
    void progressTotalIncrement(uint increment);	// Update progress total
    void progressCountIncrement(uint increment);	// Update current progress
private slots:
    void setCancelled();                            // Prepare to abort thread
private:
    bool isValidWavHeader(QDataStream& stream, uint& numberChannels,
                      uint& bitsPerSample, uint& chunkSize);
    bool getWavBuffer(QDataStream& stream, short inBuffer[2][INPUT_BLOCK_SIZE],
                  const uint numberChannels,
                  const uint bitsPerSample, const uint blockSize);
//! A set of configuration data used by LAME. LAME is re-entrant, but
//! a unique set of flags must be maintained separately for each thread.
    lame_global_flags* gfp_;
    QString inputFile_;               //!< WAV input file.
    QString outputFile_;              //!< Output file for conversion result.
    QString returnCode_;              //!< Error code to send back to caller.
    bool isConversionCancelled_;      //!< used to signal thread to abort.
};

//-----------------------------------------------------------------------------
/** @brief Display of progress

Subclass the QProgressDialogue class to provide additional progress variables
and processing. Slots allows us to update the progress dialogue. The total count
is updated by all threads to get a grand total which is used to set the maximum
time of the dialogue. The current progress count is also updated by all threads.
*/
class ProgressDisplay : public QProgressDialog
{
    Q_OBJECT
public:
    ProgressDisplay(const QString & labelText,
            const QString & cancelButtonText, int minimum, int maximum,
            QWidget* parent = 0,Qt::WindowFlags f = 0);
public slots:
    void bumpProgressTotal(uint increment);
    void bumpProgressCount(uint increment);
private:
    uint progressTotal_;    //! The total time to complete all tasks
    uint progressCount_;    //! The current progress time.
};
//-----------------------------------------------------------------------------
// LAME general functions
//-----------------------------------------------------------------------------
void errorHandler(const char* format, va_list ap);
QString setLameSetting(lame_global_flags* gfp,QString& option);
//-----------------------------------------------------------------------------

#endif
