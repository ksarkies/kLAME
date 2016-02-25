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

#include "klamemainform.h"
#include "klameoptionsdialog.h"
#include "help.h"
#include <QApplication>
#include <QFileDialog>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QProgressDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QTextEdit>
#include <QCloseEvent>
#include <QDebug>
#include <QFile>
#include <QTimer>
#include <cstdlib>
#include <iostream>

const QString VERSION = "2.0.6";
const QString VERSION_DATE = "22 September 2007";

//-----------------------------------------------------------------------------
/** @brief Constructor.

Build the User Interface display from the Ui class in ui_mainwindowform.h. The
project is set to a cleared state and the saved settings from the last time
kLAME was used are applied.
 */

KLameMainForm::KLameMainForm(QWidget* parent) : QMainWindow(parent)
{
    mainFormUi.setupUi(this);

    on_actionNewProject_triggered();   // Set the project to a cleared state
    loadSettings();                    // Settings saved from last time, if any
}

KLameMainForm::~KLameMainForm() {}

//-----------------------------------------------------------------------------
/** @brief Create a new blank project
*/

void KLameMainForm::on_actionNewProject_triggered()
{
//! Clear out the arrays of file settings.
    lameOptionsList_.clear();
    filenameTagList_.clear();
    outputDirectoryList_.clear();
//! Create blank options for the first column.
    lameOptionsList_.append("");
    filenameTagList_.append("");
    outputDirectoryList_.append("");
/* If this code looks bizarre, it is. However this is the only way that seemed
to allow kLAME to start up with a blank table view, and yet allow columns to be
selected to set the LAME options.*/
/**Set a dummy first row to trick QT into giving us access to the display
columns. Without any rows, QT would not return important information about
selected columns and it would not be possible to set LAME options.*/
    mainFormUi.mainTable->insertRow(0);
    mainFormUi.mainTable->insertRow(0);
    mainFormUi.mainTable->hideRow(1);
    mainFormUi.mainTable->removeRow(0);
//! The first column containing file paths is hidden.
    mainFormUi.mainTable->setColumnCount(3);
    mainFormUi.mainTable->hideColumn(0);
    mainFormUi.mainTable->setShowGrid(false);
    headerLabels_ = (QStringList() << "Dummy" << "Filename" << "Convert 1");
//! Default labels are set.
    mainFormUi.mainTable->setHorizontalHeaderLabels(headerLabels_);
}
//-----------------------------------------------------------------------------
/** @brief  Load all settings as a Project

The project file contains various settings for a specific task. It uses a
binary format according to QDataStream.
- streamHeader: A string giving version and author information (not used).
- noCols: number of columns in the conversion matrix.
- wavDirectory_: Location of the WAV files to convert.
- commentList_: List of comments for each column.
- filenameTagList_: List of filename tags for each column
- outputDirectoryList_ List of output directories for each column.
- lameOptionsList_: List of LAME options for each column.
- headerLabels_: List of labels for each column.

If one of the output directories is not found, a default is used. This can
occur if the project file is used on another machine where the directory
structure is different, or if a directory is moved. The directory can be reset
in the options dialogue.
*/

void KLameMainForm::on_actionOpenProject_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                "kLAME Load Project Settings",
                                projectsDirectory_,
                                "kLAME Project (*.qlp)");
    if (! filename.isEmpty())
    {
        QFileInfo fileInfo(filename);
        projectsDirectory_ = fileInfo.absolutePath();
        int ans = 0;
        if (! QFile::exists(filename))
            ans = QMessageBox::warning(
                    this, "kLAME Settings -- File Problem",
                    QString( "File not found" ),
                    "&Retry", "&Cancel", 0, 0, 1 );
        if (ans == 0)
        {
            projectFile_ = filename;
            QFile file(projectFile_);
            if (file.open(QIODevice::ReadOnly))
            {
                QDataStream stream(&file);
                int noCols;
                QString streamHeader;
                stream >> streamHeader;
                stream >> noCols;
                stream >> wavDirectory_;
                stream >> commentList_;
                stream >> filenameTagList_;
                stream >> outputDirectoryList_;
                stream >> lameOptionsList_;
                stream >> headerLabels_;
                file.close();
                headerLabels_.prepend("Dummy");
                int existingCols = mainFormUi.mainTable->columnCount()-2;
                if (existingCols < noCols-2)
                {
                    for (int ncol = existingCols; ncol < noCols-2; ncol++)
                        on_actionAddColumn_triggered();
                }
                if (existingCols > noCols-2)
                {
                    for (int ncol = noCols-2; ncol < existingCols; ncol++)
                        mainFormUi.mainTable->removeColumn(ncol);
                }
                for (QStringList::Iterator it = outputDirectoryList_.begin();
                        it != outputDirectoryList_.end(); it++)
                {
                    QDir directory(*it);
                    if (! directory.exists())
                    {
                        QMessageBox::information(this,
                                "kLAME","An output directory does not exist.\n"
                                "Some conversions will not take place");
                        break;
                    }
                }
                mainFormUi.mainTable->setHorizontalHeaderLabels(headerLabels_);
            }
        }
    }
}
//-----------------------------------------------------------------------------
/** @brief Save all settings as a Project

The settings that define the displayed matrix, including the LAME settings,
filename tags, comments, column headings, are saved in the binary QDataStream
format.
*/

void KLameMainForm::on_actionSaveProject_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
                        "kLAME Save Project Settings",projectsDirectory_,
                        "kLAME Project (*.qlp)",0,
                        QFileDialog::DontConfirmOverwrite);
    if (! filename.isEmpty())
    {
        QFileInfo fileInfo(filename);
        projectsDirectory_ = fileInfo.absolutePath();
        if (! filename.endsWith(".qlp")) filename.append(".qlp");
        int ans = 0;
        if (QFile::exists(filename))
            ans = QMessageBox::warning(
                    this, "kLAME Settings -- Overwrite File",
                    QString( "Overwrite\n'%1'?" ).arg(filename),
                    "&Yes", "&No", QString::null, 1, 1 );
        if (ans == 0)
        {
            projectFile_ = filename;
            QFile file(projectFile_);
            if (file.open(QIODevice::WriteOnly))
            {
                int numberColumns = mainFormUi.mainTable->columnCount();
                headerLabels_ = QStringList();
                for (int col = 1; col < numberColumns; col++)
                {
                    headerLabels_ << mainFormUi.mainTable->
                                    horizontalHeaderItem(col)->text();
                }
                QDataStream stream(&file);
                QString streamHeader = "kLAME "+
                                        VERSION+
                                        " - copyright K Sarkies, 2006 "+
                                        VERSION_DATE;
                stream << streamHeader;
                stream << numberColumns;
                stream << wavDirectory_;
                stream << commentList_;
                stream << filenameTagList_;
                stream << outputDirectoryList_;
                stream << lameOptionsList_;
                stream << headerLabels_;
                file.close();
            }
        }
    }
}
//-----------------------------------------------------------------------------
/** @brief Add Files to the Display

Call up a dialogue to allow a number of WAV files to be selected and added for
conversion. These are stored in the QStringList filenames and are displayed in
each row. The function can be called multiple times and additional files added.
These are inserted at the start of the list.
*/

void KLameMainForm::on_actionAddFiles_triggered()
{
    QFileDialog* fd = new QFileDialog(this,"Select Files to Convert");
    fd->setFileMode(QFileDialog::ExistingFiles);
    fd->setNameFilter("Sound Files (*.wav)");
    fd->setDirectory(wavDirectory_);
    fd->setViewMode(QFileDialog::Detail);
    QStringList filenames;
    if (fd->exec() == QDialog::Accepted)
        filenames = fd->selectedFiles();
//! The WAV directory is saved for posterity.
    wavDirectory_ = fd->directory().absolutePath();
    if (! filenames.isEmpty())
    {
        QStringList filenameList = filenames;
        for (QStringList::Iterator it = filenameList.begin();
                it != filenameList.end(); it++)
        {
            QString fname = *it;      // Filename to insert
            mainFormUi.mainTable->insertRow(0);
//! Table item widgets are created to insert into the new rows in the table.
// First column has the full filename
            QTableWidgetItem* fnameWidget = new QTableWidgetItem(fname);
            mainFormUi.mainTable->setItem(0,0,fnameWidget);
            QTableWidgetItem* fnameSectionWidget =
                    new QTableWidgetItem(fname.section("/",-1,-1));
// Second column has the cut down displayed version (the path is stripped off)..
            mainFormUi.mainTable->setItem(0,1,fnameSectionWidget);
// Then all the checkboxes for each row/column intersection are added.
            for (int ncol=2; ncol<mainFormUi.mainTable->columnCount(); ncol++)
            {
                QTableWidgetItem* checkSelected = new QTableWidgetItem();
                checkSelected->setFlags(Qt::ItemIsUserCheckable |
                                        Qt::ItemIsEnabled |
                                        Qt::ItemIsEditable |
                                        Qt::ItemIsSelectable);
                checkSelected->setCheckState(Qt::Checked);
                mainFormUi.mainTable->setItem(0,ncol,checkSelected);
            }
        }
    }
//! All rows are shown, then the last (dummy) row is hidden.
    uint rowNumber = mainFormUi.mainTable->rowCount()-1;
    for (int nrow=0; nrow < rowNumber; nrow++)
        mainFormUi.mainTable->showRow(nrow);
    mainFormUi.mainTable->hideRow(rowNumber);
}

//-----------------------------------------------------------------------------
/** @brief Delete a selected row
*/

void KLameMainForm::on_actionRemoveFile_triggered()
{
    mainFormUi.mainTable->removeRow(mainFormUi.mainTable->currentRow());
}

//-----------------------------------------------------------------------------
/** @brief Call up the Options dialogue

The column number of the selected column (or last one if none selected) is
determined, and all the parameters are passed to the options dialogue object
by calling set methods. These are the column number, lame options, filename
tag, header label and output and save directories. When the dialogue is ended,
these parameters are recovered by calling the object get methods.
*/

void KLameMainForm::on_actionOptions_triggered()
{
    KLameOptionsDialogue* lameOptionsForm = new KLameOptionsDialogue(this);
    short selectedColumn = mainFormUi.mainTable->currentColumn()-1;
//    qDebug() << selectedColumn << mainFormUi.mainTable->rowCount();
    if (selectedColumn <= 0)
            selectedColumn = mainFormUi.mainTable->columnCount()-2;
    lameOptionsForm->setLameSettingsDirectory(settingsDirectory_);
    lameOptionsForm->setOptionString(lameOptionsList_[selectedColumn-1]);
    lameOptionsForm->setFileTag(filenameTagList_[selectedColumn-1]);
    lameOptionsForm->setColumnHeading(headerLabels_[selectedColumn+1]);
    QString saveDirectory = outputDirectoryList_[selectedColumn-1];
    if (saveDirectory.isEmpty())
        saveDirectory = QDir::currentPath();
    lameOptionsForm->setConversionDirectory(saveDirectory);
    lameOptionsForm->setColumnNumber(selectedColumn);
    if (lameOptionsForm->exec())
    {
        lameOptionsList_[selectedColumn-1] =
                    lameOptionsForm->getOptionString();
        filenameTagList_[selectedColumn-1] =
                    lameOptionsForm->getFileTag();
        headerLabels_[selectedColumn+1] =
                    lameOptionsForm->getColumnHeading();
        mainFormUi.mainTable->setHorizontalHeaderLabels(headerLabels_);
        outputDirectoryList_[selectedColumn-1] =
                    lameOptionsForm->getConversionDirectory();
        settingsDirectory_ =
                    lameOptionsForm->getLameSettingsDirectory();
    }
}

//-----------------------------------------------------------------------------
/** @brief Add a new column to the end of the table of conversions

This inserts a new column with a default heading.
*/

void KLameMainForm::on_actionAddColumn_triggered()
{
    int numColumns = mainFormUi.mainTable->columnCount();
    mainFormUi.mainTable->insertColumn(numColumns);     // New column at end
    QString headerLabel = "New";
    headerLabels_ << headerLabel;
    mainFormUi.mainTable->setHorizontalHeaderLabels(headerLabels_);
/** If any rows are present, then checkboxes are added to the row/column
intersections.*/
    for (int nrow = 0; nrow < mainFormUi.mainTable->rowCount()-1; nrow++)
    {
        QTableWidgetItem* checkSelected = new QTableWidgetItem();
        checkSelected->setFlags(Qt::ItemIsUserCheckable);
//! The default setting is to check the checkboxes.
        checkSelected->setCheckState(Qt::Checked);
        mainFormUi.mainTable->setItem(nrow,numColumns,checkSelected);
    }
//! Blank options, tags and output directories are added.
    lameOptionsList_.append("");
    filenameTagList_.append("");
    outputDirectoryList_.append("");
}
//-----------------------------------------------------------------------------
/** @brief Delete the current selected column

The column is removed, as well as the header for the column.
*/

void KLameMainForm::on_actionDeleteColumn_triggered()
{
    int selectedColumn = mainFormUi.mainTable->currentColumn();
    mainFormUi.mainTable->removeColumn(selectedColumn);
    headerLabels_.removeAt(selectedColumn);
    mainFormUi.mainTable->setHorizontalHeaderLabels(headerLabels_);
}
//-----------------------------------------------------------------------------
/** @brief Perform the conversion of the selected WAV files to MP3

Each row is examined and a separate thread is launched to convert each file with
different settings for each column. LAME is called through the API as described
in the API document and in lame.h. Each file selected is given a different
thread. Note that conversions all occur in parallel. Cancelling the operation
will leave all conversions unfinished.

To allow the progress dialogue to run, a wait() blocking method is not used,
rather a loop until isFinished() is true is executed for each thread, and 
"qApp->processEvents()" is used to give control to the progress dialogue (and
other GUI processes).

QT's signals and slots are used to communicate progress between the GUI progress
display and the conversion threads.
*/

void KLameMainForm::on_actionConvertFiles_triggered()
{
    uint numberColumns = mainFormUi.mainTable->columnCount()-2;
    uint numberRows = mainFormUi.mainTable->rowCount()-1;
    uint numberFiles = numberColumns*numberRows; // Output files to create.
    QString returnCode_ = "OK";
    QString totalConversions = QString::number(numberFiles);
// Generate a progress dialogue
    ProgressDisplay progress("Conversion to mp3", "Abort", 0, 100, this);
// LAME setup
    lame_global_flags* gfp[numberColumns][numberRows];  // Setup flags array.
// Each conversion needs a converter object. These are setup as an array.
    Converter f[numberColumns][numberRows];
// Note: each column has different options.
    for (uint ncol = 2; ncol < numberColumns+2; ncol++)
    {
        QString headerLabel = headerLabels_[ncol-1];
/** Initialise LAME - this returns a pointer to its global flags if successful.
Do this for each column and row, but not for the last row as it is a dummy. If
LAME initialisation fails, the whole process is aborted as it may indicate a
major problem.*/
        for (uint nrow = 0; nrow < numberRows; nrow++)
        {
            if ((gfp[ncol-2][nrow] = lame_init()) == NULL)
            {
                returnCode_ = "LAME Initialise Fail";
                break;                          // Abort the whole conversion
            }
/** Set the error message handlers to direct error messages away from the
console (which probably doesn't exist). This could be changed to direct
messages to a log file for example.*/
            lame_set_errorf(gfp[ncol-2][nrow],errorHandler);
            lame_set_debugf(gfp[ncol-2][nrow],errorHandler);
            lame_set_msgf(gfp[ncol-2][nrow],errorHandler);
// Now set the parameters for LAME from the column options
            QString lameOptions = lameOptionsList_[ncol-2];
            QString lameOption="";
            uint n=0;
/** The option string parser is called to pull out each option and call the
appropriate LAME function to set it. kLAME uses the command line option
syntax for LAME although this is not strictly necessary; historically it is
useful as the same option syntax can be used in the settings dialogue. It allows
the functionality of the LAME calls to be extended using the existing LAME
code.*/
            do
            {
                lameOption = parseOptions(lameOptions, n);  // Get nth option
                returnCode_ = setLameSetting(gfp[ncol-2][nrow],lameOption);
// Skip out of this loop but do other columns
                if (returnCode_ != "OK") break;
                n++;
            }
            while (lameOption != "");       // loop until all options done
            if (returnCode_ == "OK")        // If errors, skip this column only
            {
// Complete LAME initialisation and final check of option validity
                if (lame_init_params(gfp[ncol-2][nrow]) < 0)
                    returnCode_ = "Parameter Error";
            }
            if (returnCode_ == "OK")        // If errors, skip this column only
            {
                QString inputFilePath =     // filename from first column
                        mainFormUi.mainTable->item(nrow,0)->text();
                QString filenameStub =      // filename without extension
                        mainFormUi.mainTable->item(nrow,1)->
                            text().section(".",0,0,QString::SectionSkipEmpty);
                QDir outputDirectory(outputDirectoryList_[ncol-2]);
                QString outputFileName = filenameStub +
                        filenameTagList_[ncol-2] + ".mp3";
                QString outputFilePath =    // Build the output filename
                        outputDirectory.filePath(outputFileName);
                if (mainFormUi.mainTable->item(nrow,ncol)->checkState() ==
                                Qt::Checked)
                {
// ** This is where the threads are initiated. **
// Connect the progress cancelled signal to the thread slot to set cancel flag
                    QObject::connect(&progress,
                                     SIGNAL(canceled()),
                                     &f[ncol-2][nrow],
                                     SLOT(setCancelled()));
// Connect the total increment signal to the progress total incrementer slot
                    QObject::connect(&f[ncol-2][nrow],
                                     SIGNAL(progressTotalIncrement(uint)),
                                     &progress,
                                     SLOT(bumpProgressTotal(uint)));
// Connect the count increment signal to the progress count incrementer slot
                    QObject::connect(&f[ncol-2][nrow],
                                     SIGNAL(progressCountIncrement(uint)),
                                     &progress,
                                     SLOT(bumpProgressCount(uint)));
// Pass necessary parameters, the internal LAME data block, input and output
// filenames, and launch the thread
                    f[ncol-2][nrow].setLameFlags(gfp[ncol-2][nrow]);
                    f[ncol-2][nrow].setInputFileName(inputFilePath);
                    f[ncol-2][nrow].setOutputFileName(outputFilePath);
                    f[ncol-2][nrow].start();
                }
            }
        }
        if (returnCode_ != "OK") break;     // Skip out if an error
    }
/** Each row/column entry is tested to see if its thread has finished. If not,
qApp->processEvents() is called to allow other processes, notably the GUI and
the progress dialogue, to get a chance to do their stuff.*/
    for (uint ncol = 2; ncol < numberColumns+2; ncol++)
    {
        for (uint nrow = 0; nrow < numberRows; nrow++)
        {
            if (mainFormUi.mainTable->item(nrow,ncol)->checkState() == 
                            Qt::Checked)
            {
// We'll just hang around until they're done
                while (! f[ncol-2][nrow].isFinished())
                    qApp->processEvents();      // Let other processes in
            }
        }
    }
/** Close down LAME, flushing all the global flag memory, and terminate the
progress dialogue.*/
    for (uint ncol = 2; ncol < numberColumns+2; ncol++)
    {
        for (uint nrow = 0; nrow < numberRows; nrow++)
            lame_close(gfp[ncol-2][nrow]);          // Free global flags memory
    }
    if (returnCode_ == "OK") QMessageBox::information(this,
                                "kLAME","Conversions Complete");
    else QMessageBox::critical(this,"LAME Conversion Failure",
                         QString("A problem occurred during conversion\n%1")
                         .arg(returnCode_));
// Force the progress dialogue to terminate
    progress.cancel();
}
//-----------------------------------------------------------------------------
/** @brief Open the Help dialogue
*/

void KLameMainForm::on_actionInstructions_triggered()
{
    Help* help = new Help(this);
    help->exec();
}

//-----------------------------------------------------------------------------
/** @brief Open the About dialogue
*/

void KLameMainForm::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About kLAME",
        "kLAME is a wav to mp3 bulk sound file conversion utility\n"
        "using the LAME mp3 engine\n\n"
        "Copyright 2006 Ken Sarkies.\n"
        "Version "+VERSION+" Date "+VERSION_DATE+"\n"
        "http://www.jiggerjuice.net/klame\n\n"
        "kLAME is free software; you can redistribute it and/or modify it\n"
        "under the terms of the GNU General Public License as published by\n"
        "the Free Software Foundation; either version 2 of the License, or\n"
        "(at your option) any later version.\n"
        "kLAME is distributed in the hope that it will be useful,\n"
        "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
        "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
        "See the GNU General Public License for more details.");
}
//-----------------------------------------------------------------------------
/** @brief Quit

Save the settings and exit the application.
*/

void KLameMainForm::on_actionQuit_triggered()
{
    saveSettings();
    QApplication::exit( 0 );
}

//-----------------------------------------------------------------------------
/** @brief Exit application
 
Exit from the same location regardless of where it is closed so that settings
can be saved.
*/

void KLameMainForm::closeEvent(QCloseEvent*)
{
    on_actionQuit_triggered();
}

//-----------------------------------------------------------------------------
/** @brief Save settings on exit

This uses the Qt QSettings class to place the settings in a default location
(registry under Windows and a settings file under Linux - see the Qt
documentation).

The settings saved are:
- Main Window position and size.
- Directory with project files.
- Directory to find the LAME settings (which can be saved in the options
dialogue).
- Directory to find the WAV files.
If these are changed during use of kLAME, the new values will be saved for tghe
next session.
*/

void KLameMainForm::saveSettings()
{
    QSettings settings("Jiggerjuice","kLAME");
    settings.setValue("/kLAME/WindowPosition", pos());
    settings.setValue("/kLAME/WindowSize", size());
    settings.setValue("/kLAME/ProjectsDir",projectsDirectory_);
    settings.setValue("/kLAME/SettingsDir",settingsDirectory_);
    settings.setValue("/kLAME/WavDir",wavDirectory_);
}
//-----------------------------------------------------------------------------
/** @brief Load settings on init
*/

void KLameMainForm::loadSettings()
{
    QSettings settings("Jiggerjuice","kLAME");
    QPoint windowPosition = settings.value("/kLAME/WindowPosition",
            QPoint(300, 60)).toPoint();
    QSize windowSize = settings.value("/kLAME/WindowSize",
            QSize(500, 360)).toSize();
    resize(windowSize);
    move(windowPosition);
    projectsDirectory_ = settings.value("/kLAME/ProjectsDir",
            QDir::currentPath()).toString();
    settingsDirectory_ = settings.value("/kLAME/SettingsDir",
            QDir::currentPath()).toString();
    wavDirectory_ = settings.value("/kLAME/WavDir",
            QDir::currentPath()).toString();
}
//-----------------------------------------------------------------------------
/** @brief Check that the wav file has the expected header format

Read the header of the WAV file and check that it has all required attributes
that will make it a valid sound file for this program. See for example
http://www.sonicspot.com/guide/wavefiles.html. A valid WAV file is uncompressed.
The stream is moved along past the header and will finally point to the start of
the samples.
@param[in] stream QDataStream I/O stream defined on input file.
@param[out] numberChannels Number of channels (1,2).
@param[out] bitsPerSample bits per sample (8,16).
@param[out] chunkSize size of data blocks (chunks).
@returns true if no error occurred in checking.
*/

bool Converter::isValidWavHeader(QDataStream& stream, uint& numberChannels,
                        uint& bitsPerSample, uint& chunkSize)
{
    QString dummyString;
    char dummy[16];
    stream.readRawData(dummy,4);            // RIFF should be present
    dummyString = dummy;                    // Cast to a string and truncate
    dummyString.truncate(4);
    if (dummyString != "RIFF") return false;
    stream.readRawData(dummy,4);            // file size
// Cast to give int value of bytes
    uint filesize = ((((unsigned int) (unsigned char) dummy[3])*256
                    + ((unsigned int) (unsigned char) dummy[2]))*256
                    + ((unsigned int) (unsigned char) dummy[1]))*256
                    +  (unsigned int) (unsigned char) dummy[0];
    stream.readRawData(dummy,4);            // WAVE should be present
    dummyString = dummy;                    // Cast to a string and truncate
    dummyString.truncate(4);
    if (dummyString != "WAVE") return false;
    stream.readRawData(dummy,4);            // fmt
    dummyString = dummy;                    // Cast to a string and truncate
    dummyString.truncate(4);
    if (dummyString != "fmt ") return false;
    stream.readRawData(dummy,4);            // fmt data size
    uint fmtSize = (((((unsigned int) (unsigned char) dummy[3])*256
                    + ((unsigned int) (unsigned char) dummy[2]))*256
                    + ((unsigned int) (unsigned char) dummy[1])))*256
                    +  (unsigned int) (unsigned char) dummy[0];
    if (fmtSize != 16) return false;
    stream.readRawData(dummy,fmtSize);      // fmt data
// Compression code (1=uncompressed)
    uint compressionCode = ((unsigned int) (unsigned char) dummy[1])*256
                          + (unsigned int) (unsigned char) dummy[0];
    if (compressionCode != 1) return false;
// Number of channels, 1 or 2
    numberChannels =      ((unsigned int) (unsigned char) dummy[3])*256
                         + (unsigned int) (unsigned char) dummy[2];
    uint sampleRate =   ((((unsigned int) (unsigned char) dummy[7])*256
                        + ((unsigned int) (unsigned char) dummy[6]))*256
                        + ((unsigned int) (unsigned char) dummy[5]))*256
                        +  (unsigned int) (unsigned char) dummy[4];
    uint bytesPerSec =  ((((unsigned int) (unsigned char) dummy[11])*256
                        + ((unsigned int) (unsigned char) dummy[10]))*256
                        + ((unsigned int) (unsigned char) dummy[9]))*256
                        +  (unsigned int) (unsigned char) dummy[8];
    uint byteAlign =      ((unsigned int) (unsigned char) dummy[13])*256
                        +  (unsigned int) (unsigned char) dummy[12];
    bitsPerSample =       ((unsigned int) (unsigned char) dummy[15])*256
                        +  (unsigned int) (unsigned char) dummy[14];
// Only allow these two for now
    if ((numberChannels != 1) && (numberChannels != 2)) return false;
// Only allow these two for now
    if ((bitsPerSample != 8) && (bitsPerSample != 16)) return false;
    stream.readRawData(dummy,4);            // data
    dummyString = dummy;                    // Cast to a string and truncate
    dummyString.truncate(4);
    if (dummyString != "data") return 0;
    stream.readRawData(dummy,4);            // data chunk size (entire file)
    chunkSize =      (((((unsigned int) (unsigned char) dummy[3])*256
                      + ((unsigned int) (unsigned char) dummy[2]))*256
                      + ((unsigned int) (unsigned char) dummy[1])))*256
                      +  (unsigned int) (unsigned char) dummy[0];
    return true;
}
//-----------------------------------------------------------------------------
/** @brief Pull in a buffer full of wav samples

Read a block from the WAV input file, split it into left and right channels, and
return the channels in a two dimensional array wav[2][], with wav[0] being left
channel and wav[1] being right channel. These can be 16 bit signed values or 8
bit unsigned values.

This function relies on isValidWavHeader() being called to position the inout
stream at the start of the WAV samples. It could be used also for a raw PCM
file as long as the number of channels and bits per sample are known.
@param[in] stream input QDataStream of WAV samples.
@param[in] numberChannels Number of channels,
@param[in] bitsPerSample bits per sample.
@param[in] blockSize and block size.
@param[out] inBuffer Buffer full of short integer data representing samples. The
buffer can hold two channels and up to INPUT_BLOCK_SIZE samples each.
@returns boolean indicating that no error occurred.
*/

bool Converter::getWavBuffer(QDataStream& stream,
                    short inBuffer[2][INPUT_BLOCK_SIZE],
                    const uint numberChannels,
                    const uint bitsPerSample, const uint blockSize)
{
    QString dummyString;
    char dummy[4];
    if (bitsPerSample == 8)                 // in this case samples unsigned
    {
        if (numberChannels == 2)
        {
            for (uint n = 0; n<blockSize; n++)
            {
                stream.readRawData(dummy,2);
                inBuffer[0][n] = (unsigned int) (unsigned char) dummy[0];
                inBuffer[1][n] = (unsigned int) (unsigned char) dummy[1];
            }
        }
        else
        {
            for (uint n = 0; n<blockSize; n++)
            {
                stream.readRawData(dummy,1);
                inBuffer[0][n] = (unsigned int) (unsigned char) dummy[0];
            }
        }
    }
    else
    {
        if (numberChannels == 2)
        {
            for (uint n = 0; n<blockSize; n++)
            {
                stream.readRawData(dummy,4);
                inBuffer[0][n] = ((unsigned int) (unsigned char) dummy[1])*256
                                + (unsigned int) (unsigned char) dummy[0];
                inBuffer[1][n] = ((unsigned int) (unsigned char) dummy[3])*256
                                + (unsigned int) (unsigned char) dummy[2];
            }
        }
        else
        {
            for (uint n = 0; n<blockSize; n++)
            {
                stream.readRawData(dummy,2);
                inBuffer[0][n] = ((unsigned int) (unsigned char) dummy[1])*256
                                + (unsigned int) (unsigned char) dummy[0];
            }
        }
    }
    return 1;
}
//-----------------------------------------------------------------------------
/** @brief Thread conversion member function to convert a single file

This is based on the Converter class defined as a subclass of QT's QThread
class. A number of quantities must be setup in the constructor to identify the
file and its LAME options.

Setup the data buffers, open the files, and start conversion block by block.
Though mp3 has a block structure, we don't need to be concerned about it. We
only do conversion in blocks to minimize memory use and to allow the progress to
be monitored and cancelled if necessary.
*/

void Converter::run()
{
    short inputBuffer[2][INPUT_BLOCK_SIZE]; // PCM sample block buffer
    uchar outputBuffer[OUTPUT_BLOCK_SIZE];  // mp3 block output buffer
    QFile outFile(outputFile_);             // Open files for input and output
    QFile inFile(inputFile_);
    if (! inFile.open(QIODevice::ReadOnly))
    {
        returnCode_ = "Could not open an input file.";
        return;
    }
    if (! outFile.open(QIODevice::WriteOnly))
    {
        returnCode_ = "Could not open an output file.";
        return;
    }
    uint inputFile_Size,bitsPerSample,numberChannels;
    QDataStream instream(&inFile);
    QDataStream outstream(&outFile);
/** The WAVE file header is checked and only certain parameters are allowed,
namely 1 or 2 channels, and 8 or 16 bit samples. Compute the number of input
blocks (file size in bytes divided by number of samples and by number of bytes
per sample) for the loop, and pass the blocksize in samples to the mp3
conversion, ensuring that the last blocksize is computed correctly as the
leftover part of a full block. */
    if (isValidWavHeader(instream,numberChannels,bitsPerSample,inputFile_Size))
    {
        ulong inputBlocks = inputFile_Size/(numberChannels*bitsPerSample)*8;
// Split input into blocks
        uint numBlocks = (inputBlocks/INPUT_BLOCK_SIZE)+1;
// Update the total number of blocks with manageable sizes
//! Emits a signal to let the Progress Display know of the new finish point
        emit progressTotalIncrement(numBlocks);
        for (uint call=0; call<numBlocks; call++)
        {
            uint blockSize = INPUT_BLOCK_SIZE;  // Last block may be smaller
            if (call == numBlocks-1) blockSize=
                                        inputBlocks-blockSize*(numBlocks-1);
            if (! getWavBuffer(instream, inputBuffer, numberChannels, 
                                bitsPerSample, blockSize))
            {
                returnCode_ = "Corrupted WAV File. Premature EOF";
                break;                          // Premature end
            }
            else
            {                                   // Convert the block
                int buffSize = lame_encode_buffer(gfp_,inputBuffer[0],
                                            inputBuffer[1],
                                            blockSize,outputBuffer,
                                            OUTPUT_BLOCK_SIZE);
                if (buffSize < 0)
                {
                    returnCode_ = "mp3 Conversion Error Occurred";
                    break;
                }
                else if (buffSize > 0)
                {                           // Dump converted block to output
                    outstream.writeRawData((const char*) outputBuffer,buffSize);
                }
            }
/** After every 100 blocks have been converted, a signal is emitted to update
the progress counter. At this point the conversion cancelled variable can be
checked.*/
            if (call % 100 == 99) emit progressCountIncrement(100);
            if (isConversionCancelled_) break;  // Signal to abort conversion
        }
        int buffSize = lame_encode_flush(gfp_,outputBuffer,OUTPUT_BLOCK_SIZE);
        if (buffSize < 0)
        {
            returnCode_ = "mp3 Conversion Error Occurred";
        }
        else if (buffSize > 0)
        {
// Dump converted block to output
            outstream.writeRawData((const char*) outputBuffer,buffSize);
        }
    }
    inFile.close();
    outFile.close();
}
//-----------------------------------------------------------------------------
/** @brief Set the cancelled variable

This slot receives the cancelled signal from the progress dialogue and lets
the thread know to stop.
*/

void Converter::setCancelled()
{
    isConversionCancelled_ = true;
}
//-----------------------------------------------------------------------------
/** @brief Return the error code from the conversions
*/

QString Converter::getReturnCode() const
{
    return returnCode_;
}

//-----------------------------------------------------------------------------
/** @brief Set the LAME global flags
*/

void Converter::setLameFlags(lame_global_flags* flags)
{
    gfp_ = flags;
}
//-----------------------------------------------------------------------------
/** @brief Set the input WAV file name
*/

void Converter::setInputFileName(QString inputFile)
{
    inputFile_ = inputFile;
}
//-----------------------------------------------------------------------------
/** @brief Set the output mp3 file name
*/

void Converter::setOutputFileName(QString outputFile)
{
    outputFile_ = outputFile;
}
//-----------------------------------------------------------------------------
/** @brief Progress Dialogue Class Definitions

The total and count for the progress dialogue are cleared.
*/

ProgressDisplay::ProgressDisplay(const QString & labelText,
            const QString & cancelButtonText, int minimum, int maximum,
            QWidget* parent,Qt::WindowFlags f)
            : QProgressDialog(labelText,cancelButtonText,
            minimum,maximum,parent,f)
{
     progressTotal_ = 0;
     progressCount_ = 0;
}
//-----------------------------------------------------------------------------
/** @brief Increment the progress total.

The progress dialogue maximum is updated by each thread once it knows how long
it will need.
*/

void ProgressDisplay::bumpProgressTotal(uint increment)
{
    progressTotal_ += increment;
    if (progressTotal_>0)                   // Start when values reasonable
        setMaximum(progressTotal_);
}
//-----------------------------------------------------------------------------
/** @brief Increment the progress count.
*/

void ProgressDisplay::bumpProgressCount(uint increment)
{
    progressCount_ += increment;
    if (progressTotal_>0)                   // Start when values reasonable
        setValue(progressCount_);
}
//-----------------------------------------------------------------------------
/** @defgroup lame General functions specific to LAME API.
*/
/*@{*/
//-----------------------------------------------------------------------------
/** @brief LAME allows an output function for error messages to be defined.

Error messages must go to the right place in a GUI environment. This error
handler for LAME just returns to give no error output. Write to an error stream
with a print function such as "(void) vfprintf(stderr, format, ap)". This could
be used to write to a log file for example.
*/

void errorHandler(const char* format, va_list ap)
{
    return;
}
//-----------------------------------------------------------------------------
/** @brief Set the LAME setting from the option provided.

Take a single option in string form, and set the corresponding LAME setting by
calling the appropriate API function. The string options recognized are in fact
the options used by the command line form of LAME.
*/

QString setLameSetting(lame_global_flags* gfp,QString& option)
{
    bool IOK,FOK;
    int parmI = 0;
    float parmF = 0;
    QString additionalOpts = "";
    QString keyword = option.section(" ",0,0);      // Pull out option keyword
    QString parameter = option.section(" ",1,1);    // First parameter if any
    if (parameter != "")
    {
// Get numbers where appropriate (no checking done yet)
        parmI = parameter.toInt(&IOK,10);
        parmF = parameter.toFloat(&FOK);
    }
    if (keyword == "-m")                            // MP3 Mode setting
    {
        if (parameter == "m")
            lame_set_mode(gfp,MONO);
        else if (parameter == "s")
            lame_set_mode(gfp,STEREO);
        else if ((parameter == "j") || (parameter == "a"))
            lame_set_mode(gfp,JOINT_STEREO);
        else if (parameter == "f")
        {
            lame_set_force_ms(gfp,1);
            lame_set_mode(gfp,JOINT_STEREO);
        }
        else if (parameter == "d")
            lame_set_mode(gfp,DUAL_CHANNEL);
        else return option;
    }
    else if (keyword == "-V")           // VBR Quality
    {                                   // If VBR not turned on, turn it on now
        if (! IOK) return option;
        if (lame_get_VBR(gfp) == vbr_off) lame_set_VBR(gfp,vbr_default);
        if (parmI < 0)
            parmI = 0;
        if (parmI > 9)
            parmI = 9;
        lame_set_VBR_q(gfp,parmI);
    }
    else if (keyword == "--vbr-new")
    {
        lame_set_VBR(gfp,vbr_mtrh);
    }
    else if (keyword == "--vbr-old")
    {
        lame_set_VBR(gfp,vbr_mtrh);
    }
    else if (keyword == "-v")
    {
        lame_set_VBR(gfp,vbr_default);
    }
    else if (keyword == "-B")
    {
        if (! IOK) return option;
        lame_set_VBR_max_bitrate_kbps(gfp,parmI);
    }
    else if (keyword == "-b")
    {
        if (! IOK) return option;
        lame_set_brate(gfp,parmI);
        lame_set_VBR_min_bitrate_kbps(gfp,lame_get_brate(gfp));
    }
    else if (keyword == "--abr")
    {
        if (! IOK) return option;
        lame_set_VBR(gfp,vbr_abr);
// Convert bps to kbps for values > 8000
        if (parmI >= 8000)
            parmI = (parmI + 500) / 1000;
        if (parmI > 320)
            parmI = 320;
        if (parmI < 8)
            parmI = 8;
        lame_set_VBR_mean_bitrate_kbps(gfp,parmI);
    }
    else if (keyword == "--cbr")
    {
        lame_set_VBR(gfp,vbr_off);
    }
    else if (keyword == "-q")
    {
        if (! IOK) return option;
        if( parmI < 0 )
            parmI = 0;
        if( parmI > 9 )
            parmI = 9;
        (void) lame_set_quality(gfp,parmI);
    }
    else if (keyword == "-k")               // No filtering
    {
        lame_set_lowpassfreq(gfp,-1);
        lame_set_highpassfreq(gfp,-1);
    }
    else if (keyword == "--preset")
    {
        if (parameter == "standard")
            lame_set_VBR_q(gfp, 2);
        else if (parameter == "medium")
            lame_set_VBR_q(gfp, 4);
        else if (parameter == "extreme")
            lame_set_VBR_q(gfp, 0);
        else if (parameter == "insane")
            lame_set_preset(gfp, INSANE);
        else return option;
    }
// Specify in kHz (<16) or Hz, convert to Hz
    else if (keyword == "--highpass")
    {
        if (! FOK) return option;
        if (parmF < 16)
            parmF *= 1000;                  // kHz specifications below 16
        if (parmF > 50000)
            parmF = 50000;
        if (parmF < 1)
            parmF = 1;
        lame_set_highpassfreq(gfp,(int)(parmF));
    }
    else if (keyword == "--highpass-width") // Specify in kHz, convert to Hz
    {
        if (! FOK) return option;
        parmF *= 1000;
        lame_set_highpasswidth(gfp,(int)parmF);
    }
// Specify in kHz (<50) or Hz, convert to Hz
    else if (keyword == "--lowpass")
    {
        if (! FOK) return option;
        if (parmF < 50)
            parmF *= 1000;                  // kHz specifications below 50
        if (parmF > 50000)
            parmF = 50000;
        if (parmF < 1)
            parmF = 1;
        lame_set_lowpassfreq(gfp,(int)(parmF));
    }
    else if (keyword == "--lowpass-width")  // Specify in kHz, convert to Hz
    {
        if (! FOK) return option;
        parmF *= 1000;
        lame_set_lowpasswidth(gfp,(int)parmF);
    }
    else if (keyword == "--cwlimit")
    {
        if (! FOK) return option;
        if (parmF < 50)
            parmF *= 1000;                  // kHz specifications below 50
        if (parmF > 50000)
            parmF = 50000;
        if (parmF < 1)
            parmF = 1;
        lame_set_cwlimit(gfp,(int)(parmF));
    }
    else if (keyword == "--resample")
    {
        if (parameter == "8")
            parmI = 8000;
        else if ((parameter == "11.025") || (parameter == "11"))
            parmI = 11025;
        else if (parameter == "12")
            parmI = 12000;
        else if (parameter == "16")
            parmI = 16000;
        else if ((parameter == "22.05") || (parameter == "22"))
            parmI = 22050;
        else if (parameter == "24")
            parmI = 24000;
        else if (parameter == "32")
            parmI = 32000;
        else if ((parameter == "44.1") || (parameter == "44.1"))
            parmI = 44100;
        else if (parameter == "48")
            parmI = 48000;
        else
            return option;
        (void) lame_set_out_samplerate( gfp,parmI);
    }
    else if (keyword == "-X")
    {
        if (! IOK) return option;
        lame_set_quant_comp(gfp, parmI);
    }
    return "OK";
}
//-----------------------------------------------------------------------------
/*@{*/
