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

#include "klameoptionsdialog.h"
#include <QFileDialog>
#include <QFile>
#include <QString>
#include <QStringList>
#include <QDir>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QSlider>
#include <QRadioButton>
#include <QMessageBox>
#include <QTextStream>
#include <QDebug>

//-----------------------------------------------------------------------------
/** @brief Setup the default display.

A table of valid keywords is created and the initial defaults are set for the
LAME option variables.
*/

KLameOptionsDialogue::KLameOptionsDialogue(QWidget* parent) : QDialog(parent)
{
    optionsDialogueUi.setupUi(this);

    initializeValidKeywords();
    setupInitialDefault();
}

KLameOptionsDialogue::~KLameOptionsDialogue() {}

//-----------------------------------------------------------------------------
/** @brief Access function to set the LAME option string.

@param[in] options QString of LAME options.
*/

void KLameOptionsDialogue::setOptionString(QString& options)
{
    lameOptions_ = options;
//! The LAME option string is decoded and the option variables are set.
    setupInitialDefault();
}
//-----------------------------------------------------------------------------
/** @brief Access function to get the LAME option string.

@returns QString of LAME options.
*/

QString KLameOptionsDialogue::getOptionString() const
{
    return lameOptions_;
}
//-----------------------------------------------------------------------------
/** @brief Access function to set the mp3 file output directory name
 
The conversion directory name variable is set and is shown in the
dialogue.
@param[in] conversionDirectory QString Directory name.
*/

void KLameOptionsDialogue::setConversionDirectory(QString& conversionDirectory)
{
    conversionDirectory_ = conversionDirectory;
    optionsDialogueUi.outputDirectory->setText(conversionDirectory_);
}
//-----------------------------------------------------------------------------
/** @brief Access function to munge and return the mp3 file output directory.

Also checks that we can access the entered directory, and create it if it is
not there.
@returns QString Directory name.
*/

QString& KLameOptionsDialogue::getConversionDirectory()
{
    QString originalDirectory = conversionDirectory_;
    conversionDirectory_ = optionsDialogueUi.outputDirectory->text();
    QDir saveDir(conversionDirectory_);
    bool isOk(true);
    if (! saveDir.exists())
    {
        int ans = 0;
        if (! saveDir.exists())
            ans = QMessageBox::warning(this,
                    "kLAME Settings -- Create Directory",
                    QString( "Directory\n'%1'\ndoes not exist - Create it?" )
                    .arg(conversionDirectory_),
                    "&Yes", "&No", QString::null, 1, 1 );
        if (ans == 0)
        {
            isOk = saveDir.mkpath(conversionDirectory_);
            if (isOk)
                conversionDirectory_ = saveDir.canonicalPath();
            else
            {
                QMessageBox::information(this,
                    "kLAME","Unable to create the directory");
            }
        }
        else isOk = false;
    }
    if (! isOk)
    {
        conversionDirectory_ = originalDirectory;
    }
    conversionDirectory_ = QDir::cleanPath(conversionDirectory_);
    return conversionDirectory_;
}
//-----------------------------------------------------------------------------
/** @brief Access function to set the LAME option settings directory name.

@param[in] dir QString directory name.
This directory is used to store the option settings files for commonly used LAME
options. 
*/

void KLameOptionsDialogue::setLameSettingsDirectory(QString& dir)
{
    lameSettingsDirectory_ = dir;
}
//-----------------------------------------------------------------------------
/** @brief Access function to get the LAME settings directory name.

@returns QString directory name.
*/

QString KLameOptionsDialogue::getLameSettingsDirectory() const
{
    return lameSettingsDirectory_ ;
}
//-----------------------------------------------------------------------------
/** @brief Access function to set the filename tag and show it.

@param[in] fileTag QString filename tag.
*/

void KLameOptionsDialogue::setFileTag(QString& fileTag)
{
    fileTag_ = fileTag;
    optionsDialogueUi.filenameTag->setText(fileTag_);
}
//-----------------------------------------------------------------------------
/** @brief Access function to get the filename tag.

@returns QString filename tag.
*/

QString& KLameOptionsDialogue::getFileTag()
{
    fileTag_ = optionsDialogueUi.filenameTag->text();
    return fileTag_;
}
//-----------------------------------------------------------------------------
/** @brief Access function to set the column title

@param[in] columnName QString column title.
*/

void KLameOptionsDialogue::setColumnHeading(QString& columnName)
{
    columnName_ = columnName;
    optionsDialogueUi.columnHeaderEdit->setText(columnName_);
}
//-----------------------------------------------------------------------------
/** @brief Access function to get the column name

@returns QString column title.
*/

QString& KLameOptionsDialogue::getColumnHeading()
{
    columnName_ = optionsDialogueUi.columnHeaderEdit->text();
    return columnName_;
}
//-----------------------------------------------------------------------------
/** @brief Access function to set the column number

@param[in] columnNumber int column number.
This is not modifiable, it identifies the column to which the settings apply.
*/

void KLameOptionsDialogue::setColumnNumber(int columnNumber)
{
    optionsDialogueUi.columnNumber->setText(QString::number(columnNumber));
}
//-----------------------------------------------------------------------------
/** @brief Select a directory for destination converted files.
*/

void KLameOptionsDialogue::on_browseDirectory_clicked()
{
    conversionDirectory_ = QFileDialog::getExistingDirectory(this,
                                        "Choose a directory",
                                        QDir::currentPath(),
                                        QFileDialog::ShowDirsOnly |
                                        QFileDialog::DontResolveSymlinks);
    optionsDialogueUi.outputDirectory->setText(conversionDirectory_);
}
//-----------------------------------------------------------------------------
/** @brief OK Button Clicked

On acceptance, check the selected LAME settings and build the final option
string. The window is closed and the aprent object resumes. The options can
be accessed from the options dialogue object which still exists in the context
in which it was called.
*/

void KLameOptionsDialogue::on_buttonOk_clicked()
{
    buildOptions();
    accept();
}
//-----------------------------------------------------------------------------
/** @brief Cancel Button clicked.

Simply close the window and return a reject response. The parent can still
access the object, but has no reason to do so.
*/

void KLameOptionsDialogue::on_buttonCancel_clicked()
{
    reject();
}
//-----------------------------------------------------------------------------
/** @brief Select and Load Settings File.

A file of commonly used LAME settings can be loaded up. These are stored as a
text string of command line options and are read in a QTextStream directly to
the lameOptions variable. They are then interpreted and the LAME options
variables are set.
*/

void KLameOptionsDialogue::on_loadSettingsSelect_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this,
                                        "kLAME Load LAME Settings",
                                        lameSettingsDirectory_,
                                        "LAME Settings (*.txt)");
    if (! filename.isEmpty())
    {
        QFileInfo fileInfo(filename);
        lameSettingsDirectory_ = fileInfo.absolutePath();
        QFile file(filename);
        if (file.open(QIODevice::ReadOnly))
        {
            QTextStream stream(&file);
            QString line;
            while (! stream.atEnd())
            {
                line = stream.readLine();
            }
            lameOptions_ = line;
            file.close();
            setupInitialDefault();
        }
    }
}

//-----------------------------------------------------------------------------
/** @brief Save Settings File
*/

void KLameOptionsDialogue::on_saveSettingsSelect_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,
                                    "kLAME Save LAME Settings",
                                    lameSettingsDirectory_,
                                    "LAME Settings (*.txt)",0,
                                    QFileDialog::DontConfirmOverwrite);
    if (! filename.isEmpty())
    {
        QFileInfo fileInfo(filename);
        lameSettingsDirectory_ = fileInfo.absolutePath();
        if (! filename.endsWith(".txt")) filename.append(".txt");
        int ans = 0;
        if ( QFile::exists( filename ) )
            ans = QMessageBox::warning( 
                    this, "kLAME - Overwrite File",
                    QString( "Overwrite\n'%1'?" ).
                    arg( filename ),
                    "&Yes", "&No", QString::null, 1, 1 );
        if ( ans == 0 ) 
        {
            buildOptions();             // Create the options string
            QFile file( filename );
            if ( file.open( QIODevice::WriteOnly ) )
            {
                QTextStream stream( &file );
                stream << lameOptions_;
                file.close();
            }
        }
    }
}

//-----------------------------------------------------------------------------
/** @brief Quality Preference button is clicked.

Disable certain parts of the form if the Quality Preference button is selected.
This button choses the VBR type.
*/

void KLameOptionsDialogue::on_qualityPref_toggled()
{
    if (optionsDialogueUi.qualityPref->isChecked())     // VBR
    {
        optionsDialogueUi.qualityBox->setEnabled(true);
        optionsDialogueUi.useCbr->setEnabled(false);
        optionsDialogueUi.useMinVbrBitrate->setEnabled(true);
        optionsDialogueUi.abrBitrateBox->setEnabled(false);
    }
}
//-----------------------------------------------------------------------------
/** @brief Bitrate Preference button is clicked.

Enable certain parts of the form if the Bitrate Preference button is selected.
This button choses the CBR/ABR types.
*/

void KLameOptionsDialogue::on_bitratePref_toggled()
{
    if (optionsDialogueUi.bitratePref->isChecked())     // ABR/CBR
    {
        optionsDialogueUi.qualityBox->setEnabled(false);
        optionsDialogueUi.useCbr->setEnabled(true);
        on_useCbr_stateChanged();
    }
}

//-----------------------------------------------------------------------------
/** @brief The CBR checkbox changes state

Disable/enable some elements if the CBR checkbox is changed.
*/

void KLameOptionsDialogue::on_useCbr_stateChanged()
{
    if (optionsDialogueUi.useCbr->isChecked())
    {
        optionsDialogueUi.useMinVbrBitrate->setEnabled(false);
        optionsDialogueUi.maxVbrBitrateBox->setEnabled(false);
        optionsDialogueUi.useMaxBitrate->setChecked(false);
        optionsDialogueUi.useMaxBitrate->setEnabled(false);
        optionsDialogueUi.abrBitrateBox->setEnabled(false);
    }
    else
    {
        optionsDialogueUi.abrBitrateBox->setEnabled(true);
    }
}
//-----------------------------------------------------------------------------
/** @brief The Use Maximum VBR Bitrate checkbox changes state

Enable/disable the maxbitrate control from the checkbox state.
*/

void KLameOptionsDialogue::on_useMaxBitrate_stateChanged()
{
    optionsDialogueUi.maxVbrBitrateBox->
            setEnabled(optionsDialogueUi.useMaxBitrate->isChecked());
}

//-----------------------------------------------------------------------------
/** @brief The Change Quality Measure checkbox changes state

Enable/disable quality measure from the checkbox state.
*/

void KLameOptionsDialogue::on_changeQualityMeasure_stateChanged()
{
    optionsDialogueUi.qualityMeasure->
            setEnabled(optionsDialogueUi.changeQualityMeasure->isChecked());
}
//-----------------------------------------------------------------------------
/** @brief The Use Resample Frequency checkbox changes state

Enable/disable resample frequency from the checkbox state.
*/

void KLameOptionsDialogue::on_useResampleFrequency_stateChanged()
{
    optionsDialogueUi.resampleFrequency->
            setEnabled(optionsDialogueUi.useResampleFrequency->isChecked());
}
//-----------------------------------------------------------------------------
/** @brief The Quality Select slider bar changes state

Display the quality selection in the Quality Display spinbox.
*/

void KLameOptionsDialogue::on_qualitySelect_valueChanged()
{
    optionsDialogueUi.qualityDisplay->
            setValue(optionsDialogueUi.qualitySelect->value());
}
//-----------------------------------------------------------------------------
/** @brief The Quality Display spinbox changes state

Display the quality selection in the Quality Select slider bar.
*/

void KLameOptionsDialogue::on_qualityDisplay_valueChanged(int i)
{
    optionsDialogueUi.qualitySelect->
            setValue(optionsDialogueUi.qualityDisplay->value());
}
//-----------------------------------------------------------------------------
/** @brief The BitRate Select slider bar changes state

Display the bit rate selection in the BitRate Display spinbox.
*/
// Display bit rate selection as line-edit box

void KLameOptionsDialogue::on_bitrateSelect_valueChanged()
{
    optionsDialogueUi.bitrateDisplay->
            setValue(optionsDialogueUi.bitrateSelect->value());
}
//-----------------------------------------------------------------------------
/** @brief The BitRate Display spinbox changes state

Display the bit rate selection in the BitRate Select slider bar.
*/

void KLameOptionsDialogue::on_bitrateDisplay_valueChanged(int i)
{
    optionsDialogueUi.bitrateSelect->
            setValue(optionsDialogueUi.bitrateDisplay->value());
}
//-----------------------------------------------------------------------------
/** @brief The Maximum BitRate Select slider bar changes state

Display the bit rate selection in the Maximum BitRate Display spinbox.
*/

void KLameOptionsDialogue::on_maxBitrateVbr_valueChanged()
{
    optionsDialogueUi.maxBitrateDisplay->
            setValue(optionsDialogueUi.maxBitrateVbr->value());
}
//-----------------------------------------------------------------------------
/** @brief The Maximum BitRate Display spinbox changes state

Display the bit rate selection in the Maximum BitRate Select slider bar.
*/

void KLameOptionsDialogue::on_maxBitrateDisplay_valueChanged(int i)
{
    optionsDialogueUi.maxBitrateVbr->
            setValue(optionsDialogueUi.maxBitrateDisplay->value());
}
//-----------------------------------------------------------------------------
/** @brief The ABR BitRate Select slider bar changes state

Display the bit rate selection in the ABR BitRate Display spinbox.
*/

void KLameOptionsDialogue::on_abrBitrateSelect_valueChanged()
{
    optionsDialogueUi.abrBitrateDisplay->
            setValue(optionsDialogueUi.abrBitrateSelect->value());
}
//-----------------------------------------------------------------------------
/** @brief The ABR BitRate Display spinbox changes state

Display the bit rate selection in the ABR BitRate Select slider bar.
*/

void KLameOptionsDialogue::on_abrBitrateDisplay_valueChanged(int i)
{
    optionsDialogueUi.abrBitrateSelect->
            setValue(optionsDialogueUi.abrBitrateDisplay->value());
}
//-----------------------------------------------------------------------------
/** @brief Set a consistent display
 
Items are appropriately enabled or disabled and linked items are synchronized.
*/

void KLameOptionsDialogue::setupDisplay()
{
    on_useMaxBitrate_stateChanged();        // Enable/disable bitrate selector
    on_changeQualityMeasure_stateChanged(); // Enable/disable quality measure
    on_qualityPref_toggled();
    on_bitratePref_toggled();
    on_useResampleFrequency_stateChanged(); // Enable/disable resample freq
    on_bitrateSelect_valueChanged();        // Synchronize bitrate controls
    on_qualitySelect_valueChanged();        // Synchronize quality controls
    on_abrBitrateSelect_valueChanged();     // Synchronize ABR bitrate controls
    on_maxBitrateVbr_valueChanged();        // Synchronize max bitrate controls
}
//-----------------------------------------------------------------------------
/** @brief Preset LAME options on display initialization.

The settings are cleared and the option string is interpreted and options
variables are set.
*/

void KLameOptionsDialogue::setupInitialDefault()
{
    wipeSettings();
    if (lameOptions_ == "")         // Set basic defaults if no options given
        setupDisplay();
    else setDefaultOptions(lameOptions_);
}
//-----------------------------------------------------------------------------
/** @brief Wipe all settings in dialogue to a default state.

The defaults are defined in this function and need to be changed here if
necessary.
*/

void KLameOptionsDialogue::wipeSettings()
{
    optionsDialogueUi.mode->setCurrentIndex(0);
    optionsDialogueUi.bitratePref->setChecked(false);
    optionsDialogueUi.qualityPref->setChecked(true);
    optionsDialogueUi.qualitySelect->setValue(60);
    on_qualitySelect_valueChanged();
    optionsDialogueUi.bitrateSelect->setValue(128);
    on_bitrateSelect_valueChanged();
    optionsDialogueUi.abrBitrateSelect->setValue(128);
    on_abrBitrateSelect_valueChanged();
    optionsDialogueUi.useCbr->setChecked(false);
    optionsDialogueUi.qualitySetting->setValue(5);
    optionsDialogueUi.noFiltering->setChecked(false);
    optionsDialogueUi.blockSizeDifference->setChecked(false);
    optionsDialogueUi.useMaxBitrate->setChecked(false);
    on_maxBitrateVbr_valueChanged();
    optionsDialogueUi.useMinVbrBitrate->setChecked(false);
    optionsDialogueUi.presetComboBox->setCurrentIndex(1);
    optionsDialogueUi.presetSelect->setChecked(false);
    optionsDialogueUi.highpassFreqSelect->setChecked(false);
    optionsDialogueUi.highpassFreq->setText("");
    optionsDialogueUi.highpassWidthSelect->setChecked(false);
    optionsDialogueUi.highpassWidth->setText("");
    optionsDialogueUi.lowpassFreqSelect->setChecked(false);
    optionsDialogueUi.lowpassFreq->setText("");
    optionsDialogueUi.lowpassWidthSelect->setChecked(false);
    optionsDialogueUi.lowpassWidth->setText("");
    optionsDialogueUi.useTonalityLimit->setChecked(false);
    optionsDialogueUi.tonalityLimit->setText("");
    optionsDialogueUi.useResampleFrequency->setChecked(false);
    optionsDialogueUi.resampleFrequency->setCurrentIndex(0);
    optionsDialogueUi.algorithmVbr->setCurrentIndex(0);
    optionsDialogueUi.changeQualityMeasure->setChecked(false);
    optionsDialogueUi.qualityMeasure->setValue(0);
    optionsDialogueUi.additionalOptions->setText("");
    }
//-----------------------------------------------------------------------------
/** @brief Set the option dialogue settings from the option string.

This sets the display to reflect the options provided in the option string.
Called when the LAME options string is changed (loading and initialization).
Called also to set any options that were entered in the additional option box.
It is possible that these additional options may be reflected in the GUI
options settings, so the display needs to be updated. For that reason, the
display is not cleared and the other selected options are kept unchanged.
@param[in] options QString of command-line LAME options.
*/

void KLameOptionsDialogue::setDefaultOptions(QString& options)
{
    bool isOk;
    int parm = 0;
    short optionNumber = 0;
    QString option = parseOptions(options, optionNumber);
    QString additionalOpts = "";
    while (option != "")
    {
        QString keyword = option.section(" ",0,0);  // Pull out option keyword
        QString parameter = option.section(" ",1,1);// First parameter if any
        if (parameter != "") parm = parameter.toInt(&isOk,10);
        if (keyword == "-m")
        {
            if (parameter == "m")
                optionsDialogueUi.mode->setCurrentIndex(1);
            else if (parameter == "s")
                optionsDialogueUi.mode->setCurrentIndex(2);
            else if (parameter == "j")
                optionsDialogueUi.mode->setCurrentIndex(3);
            else if (parameter == "f")
                optionsDialogueUi.mode->setCurrentIndex(4);
            else if (parameter == "d")
                optionsDialogueUi.mode->setCurrentIndex(5);
        }
        else if (keyword == "-V")
        {
            optionsDialogueUi.qualityPref->setChecked(true);
            optionsDialogueUi.qualitySelect->setValue(100-parm*10);
        }
        else if (keyword == "--vbr-new")
        {
            optionsDialogueUi.qualityPref->setChecked(true);
            optionsDialogueUi.algorithmVbr->setCurrentIndex(1);
        }
        else if ((keyword == "--vbr-old") || (keyword == "-v"))
        {
            optionsDialogueUi.qualityPref->setChecked(true);
            optionsDialogueUi.algorithmVbr->setCurrentIndex(0);
        }
        else if (keyword == "-B")
        {
            optionsDialogueUi.useMaxBitrate->setChecked(true);
            optionsDialogueUi.maxBitrateVbr->setValue(parm);
        }
        else if (keyword == "-b")
        {
            optionsDialogueUi.bitrateSelect->setValue(parm);
            optionsDialogueUi.useMinVbrBitrate->setChecked(true);
        }
        else if (keyword == "--abr")
        {
            optionsDialogueUi.bitratePref->setChecked(true);
            optionsDialogueUi.abrBitrateSelect->setValue(parm);
        }
        else if (keyword == "--cbr")
        {
            optionsDialogueUi.bitratePref->setChecked(true);
            optionsDialogueUi.useCbr->setChecked(true);
        }
        else if (keyword == "-q")
            optionsDialogueUi.qualitySetting->setValue(parm);
        else if (keyword == "-k")
            optionsDialogueUi.noFiltering->setChecked(true);
// Obsolete keyword
//      else if (keyword == "-d")
//          blockSizeDifference->setChecked(true);
        else if (keyword == "--preset")
        {
            optionsDialogueUi.presetSelect->setChecked(true);
            if (parameter == "standard")
                optionsDialogueUi.presetComboBox->setCurrentIndex(0);
            if (parameter == "medium")
                optionsDialogueUi.presetComboBox->setCurrentIndex(1);
            if (parameter == "extreme")
                optionsDialogueUi.presetComboBox->setCurrentIndex(2);
            if (parameter == "insane")
                optionsDialogueUi.presetComboBox->setCurrentIndex(3);
        }
        else if (keyword == "--highpass")
        {
            optionsDialogueUi.highpassFreqSelect->setChecked(true);
            optionsDialogueUi.highpassFreq->setText(parameter);
        }
        else if (keyword == "--highpass-width")
        {
            optionsDialogueUi.highpassWidthSelect->setChecked(true);
            optionsDialogueUi.highpassWidth->setText(parameter);
        }
        else if (keyword == "--lowpass")
        {
            optionsDialogueUi.lowpassFreqSelect->setChecked(true);
            optionsDialogueUi.lowpassFreq->setText(parameter);
        }
        else if (keyword == "--lowpass-width")
        {
            optionsDialogueUi.lowpassWidthSelect->setChecked(true);
            optionsDialogueUi.lowpassWidth->setText(parameter);
        }
        else if (keyword == "--cwlimit")
        {
            optionsDialogueUi.useTonalityLimit->setChecked(true);
            optionsDialogueUi.tonalityLimit->setText(parameter);
        }
        else if (keyword == "--resample")
        {
            optionsDialogueUi.useResampleFrequency->setChecked(true);
            if (parameter == "8")
                optionsDialogueUi.resampleFrequency->setCurrentIndex(0);
            else if (parameter == "11.025")
                optionsDialogueUi.resampleFrequency->setCurrentIndex(1);
            else if (parameter == "12")
                optionsDialogueUi.resampleFrequency->setCurrentIndex(2);
            else if (parameter == "16")
                optionsDialogueUi.resampleFrequency->setCurrentIndex(3);
            else if (parameter == "22.05")
                optionsDialogueUi.resampleFrequency->setCurrentIndex(4);
            else if (parameter == "24")
                optionsDialogueUi.resampleFrequency->setCurrentIndex(5);
            else if (parameter == "32")
                optionsDialogueUi.resampleFrequency->setCurrentIndex(6);
            else if (parameter == "44.1")
                optionsDialogueUi.resampleFrequency->setCurrentIndex(7);
            else
                optionsDialogueUi.resampleFrequency->setCurrentIndex(8);
        }
        else if (keyword == "-X")
        {
            optionsDialogueUi.changeQualityMeasure->setChecked(true);
            optionsDialogueUi.qualityMeasure->setValue(parm);
        }
        else if (keyword != "--silent")
            additionalOpts += option + " ";
        optionNumber++;
        option = parseOptions(options, optionNumber);
    }
    setupDisplay();
    optionsDialogueUi.additionalOptions->setText(additionalOpts);
    on_qualityPref_toggled();
    on_bitratePref_toggled();
    on_useMaxBitrate_stateChanged();
    on_changeQualityMeasure_stateChanged();
    on_useResampleFrequency_stateChanged();
    on_useCbr_stateChanged();
}
//-----------------------------------------------------------------------------
/** @brief Check the selected LAME settings and build the option string.
*/

void KLameOptionsDialogue::buildOptions()
{
// Clear the options and rebuild them. Set Silent operation.
    lameOptions_ = " --silent";
// Additional options - we may need to parse these
    QString additionalOpt = optionsDialogueUi.additionalOptions->text();
    if (optionsDialogueUi.discardInvalidOptions->isChecked())
        stripBadOptions(additionalOpt );        // Pull out invalid options
    setDefaultOptions(additionalOpt);   // Set any display options from this
    lameOptions_ += additionalOpt;
// Check for VBR or CBR/ABR. These are all the basic settings on second tab.
    if (optionsDialogueUi.qualityPref->isChecked()) //VBR
    {
// Set vbr quality, maximum bitrate and algorithm
        lameOptions_ += " -V " +
                        QString::number((100-optionsDialogueUi.qualityDisplay->
                        value())/10);
        if (optionsDialogueUi.useMaxBitrate->isChecked())
            lameOptions_ += " -B " +
                        QString::number(optionsDialogueUi.maxBitrateDisplay->
                        value());
        if (optionsDialogueUi.algorithmVbr->currentIndex() == 1)
            lameOptions_ += " --vbr-new";
        else
            lameOptions_ += " --vbr-old";
        if (optionsDialogueUi.useMinVbrBitrate->isChecked())
            lameOptions_ += " -b " +
                        QString::number(optionsDialogueUi.bitrateDisplay->
                        value());
    }
    else                                            // CBR/ABR
    {
// Set CBR or ABR bitrates
        if (! optionsDialogueUi.useCbr->isChecked())
        {
            lameOptions_ += " --abr " +
                        QString::number(optionsDialogueUi.abrBitrateDisplay->
                        value());
            if (optionsDialogueUi.useMaxBitrate->isChecked())
                lameOptions_ += " -B " +
                        QString::number(optionsDialogueUi.maxBitrateDisplay->
                        value());
            if (optionsDialogueUi.useMinVbrBitrate->isChecked())
                lameOptions_ += " -b " +
                        QString::number(optionsDialogueUi.bitrateDisplay->
                        value());
        }
        else
        {
            lameOptions_ += " --cbr ";
            lameOptions_ += "-b " +
                        QString::number(optionsDialogueUi.bitrateDisplay->
                        value());
        }
    }
// These are the advanced settings on the third tab
// Set mono or stereo modes
    switch (optionsDialogueUi.mode->currentIndex())
    {
    case 1: lameOptions_ += " -m m"; break;
    case 2: lameOptions_ += " -m s"; break;
    case 3: lameOptions_ += " -m j"; break;
    case 4: lameOptions_ += " -m f"; break;
    case 5: lameOptions_ += " -m d"; break;
    }
// Quality setting
    lameOptions_ += " -q " +
                    QString::number(optionsDialogueUi.qualitySetting->value());
// No filtering (use all bandwidths) advanced option
    if (optionsDialogueUi.noFiltering->isChecked())
        lameOptions_ += " -k";
// Allow Block sizes to differ between channels setting
//  if (blockSizeDifference->isChecked())
//      lameOptions_ += " -d";
// Preset option
    if (optionsDialogueUi.presetSelect->isChecked())
        lameOptions_ += " --preset " +
                        optionsDialogueUi.presetComboBox->currentText();
// Frequency measures
    if (optionsDialogueUi.highpassFreqSelect->isChecked())
        lameOptions_ += " --highpass " +
                        optionsDialogueUi.highpassFreq->text();
    if (optionsDialogueUi.highpassWidthSelect->isChecked())
        lameOptions_ += " --highpass-width " +
                        optionsDialogueUi.highpassWidth->text();
    if (optionsDialogueUi.lowpassFreqSelect->isChecked())
        lameOptions_ += " --lowpass " +
                        optionsDialogueUi.lowpassFreq->text();
    if (optionsDialogueUi.lowpassWidthSelect->isChecked())
        lameOptions_ += " --lowpass-width " +
                        optionsDialogueUi.lowpassWidth->text();
// Change measure used for determining quality (0-9)
    if (optionsDialogueUi.changeQualityMeasure->isChecked())
        lameOptions_ += " -X " +
                        QString::number(optionsDialogueUi.qualityMeasure->
                        value());
// Resample if checked
    if (optionsDialogueUi.useResampleFrequency->isChecked())
    {
        QString resampleFrequencyText =
                        optionsDialogueUi.resampleFrequency->currentText();
        lameOptions_ += " --resample " +
                        resampleFrequencyText.
                        left(resampleFrequencyText.indexOf(" kHz"));
    }
// Tonality Limit
    if (optionsDialogueUi.useTonalityLimit->isChecked())
        lameOptions_ += " --cwlimit " +
                        optionsDialogueUi.tonalityLimit->text();
}
//-----------------------------------------------------------------------------
/** @brief Check the option string and strip it of any that are not valid here.

Use the QStringList of valid options set in the constructor to verify that an
option is valid. The option list is sent back stripped of any invalid options.
@param options QString of command-line LAME options.
*/

void KLameOptionsDialogue::stripBadOptions(QString& options)
{
    short optionNumber = 0;
    QString option = parseOptions(options, optionNumber);
    QString goodOptions = "";
    while (option != "")
    {
        QString keyword = option.section(" ",0,0);// Pull out option keyword
        for ( QStringList::Iterator it = validKeywords.begin();
                it != validKeywords.end(); ++it)
        {
            if (keyword == *it)
            {
                goodOptions += option + " ";
                break;
            }
        }
        optionNumber++;
        option = parseOptions(options, optionNumber);
    }
    options = goodOptions;                  // Send back stripped option string
}
//-----------------------------------------------------------------------------
/** @brief Setup a list of valid keywords recognised by LAME
*/

void KLameOptionsDialogue::initializeValidKeywords()
{
    validKeywords << "-a";
    validKeywords << "--add-id3v2";
    validKeywords << "--allshort";
    validKeywords << "--alt-preset";
    validKeywords << "--athaa-type";
    validKeywords << "--athaa-sensitivity";
    validKeywords << "--athonly";
    validKeywords << "--athlower";
    validKeywords << "--athtype";
    validKeywords << "-b";
    validKeywords << "-B";
    validKeywords << "--big-endian";
    validKeywords << "--bitwidth";
    validKeywords << "-c";
    validKeywords << "--cbr";
    validKeywords << "--clip-detect";
    validKeywords << "--comp";
    validKeywords << "--cwlimit";
//  validKeywords << "-d";              // deprecated? - worse, just not there
    validKeywords << "--decode";
    validKeywords << "--disptime";
    validKeywords << "-e";
    validKeywords << "-f";
    validKeywords << "-F";
    validKeywords << "--freeformat";
    validKeywords << "--interch";
    validKeywords << "-g";
    validKeywords << "--genre-list";
    validKeywords << "-h";
    validKeywords << "--highpass";
    validKeywords << "--highpass-width";
    validKeywords << "--id3v1-only";
    validKeywords << "--id3v2-only";
    validKeywords << "--ignore-tag-errors";
    validKeywords << "-k";
    validKeywords << "--little-endian";
    validKeywords << "--lowpass";
    validKeywords << "--lowpass-width";
    validKeywords << "-m";
    validKeywords << "--mp1input";
    validKeywords << "--mp2input";
    validKeywords << "--mp3input";
    validKeywords << "--noasm";
    validKeywords << "--nohist";
    validKeywords << "--nogap";
    validKeywords << "--nogapout";
    validKeywords << "--noreplaygain";
    validKeywords << "--nores";
    validKeywords << "--noshort";
    validKeywords << "--notemp";
    validKeywords << "--ns-alto";
    validKeywords << "--ns-bass";
    validKeywords << "--nspsytune";
    validKeywords << "--nsmsfix";
    validKeywords << "--nssafejoint";
    validKeywords << "--nssfb21";
    validKeywords << "--ns-treble";
    validKeywords << "-o";
    validKeywords << "-p";
    validKeywords << "--pad-id3v2";
    validKeywords << "--preset";
    validKeywords << "-q";
    validKeywords << "--quiet";
    validKeywords << "-r";
    validKeywords << "--r3mix";
    validKeywords << "--replaygain-accurate";
    validKeywords << "--replaygain-fast";
    validKeywords << "-s";
    validKeywords << "-S";
    validKeywords << "--scale";
    validKeywords << "--scale-l";
    validKeywords << "--scale-r";
    validKeywords << "--short";
    validKeywords << "--silent";
    validKeywords << "--signed";
    validKeywords << "--space-id3v1";
    validKeywords << "--strictly-enforce-ISO";
    validKeywords << "--substep";
    validKeywords << "-t";
    validKeywords << "-T";
    validKeywords << "--ta";
    validKeywords << "--tc";
    validKeywords << "--tg";
    validKeywords << "--tl";
    validKeywords << "--tn";
    validKeywords << "--tt";
    validKeywords << "--ty";
    validKeywords << "--unsigned";
    validKeywords << "-v";
    validKeywords << "-V";
    validKeywords << "--vbr-old";
    validKeywords << "--vbr-new";
    validKeywords << "-x";
    validKeywords << "-X";
    validKeywords << "-Y";
    validKeywords << "-Z";
}
//-----------------------------------------------------------------------------
// NON-MEMBER FUNCTIONS useable in other parts of the program
//-----------------------------------------------------------------------------
/** @brief Pull out a command-line option from a string
@ingroup lame

The string is examined for the presence of Unix style command-line options
(starting with a "-" or "--" symbol and preceded by white space). One of the
options is separated out and
returned.
@param[in] QString of command-line LAME options.
@param[in] index of the option to check.
@returns a QString of the nth option (n>=0).
*/

QString parseOptions(QString& options, short n)
{
    short tokenNumber = 0;
    short optNumber = 0;
    QString option;
    options = options.simplified();
    QString token;
    bool isOptionFound = false;
    do
    {
        token = options.section(" ",tokenNumber,tokenNumber);
        tokenNumber++;
        if (isOptionFound)
        {
            if (token[0] == '-') break;
            option += " " + token;
        }
        else if (token[0] == '-')       // token is start of an option string
        {
            if (optNumber == n)         // this is the one we want
            {
                isOptionFound = true;
                option = token;         // start our returned option string
            }
            optNumber++;
        }
    }
    while (token != "");
    return option;
}
//-----------------------------------------------------------------------------
