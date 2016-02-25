//          kLAME GUI frontend wrapper for LAME - conversion control wav to mp3
//              Options Dialogue Subclass

// Author:   Ken Sarkies ksarkies at trinity.asn.au
// File:     $Id: klameoptionsdialog.h, v 2.0 16/6/2007 $
// Software: gcc 4.1.1, QT4 4.3.0-2
// Tested:   x86_64 Linux Fedora 6

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

#ifndef KLAMEOPTIONSDIALOGUE_H
#define KLAMEOPTIONSDIALOGUE_H

#include "ui_klameoptionsdialoguebase.h"
#include <QWidget>

//-----------------------------------------------------------------------------
/** @brief kLAME Options Dialogue window

This is a tabbed window in which a range of options are displayed and set.
- The first tab allows general non-LAME options, output directory, filename
tags and column title. Buttons allow commonly used LAME settings to be saved and
loaded.
- The second tab provides some basic LAME settings that define broad
characteristics of the mp3 conversion, basically the bitrate types and their
quality settings.
- The third tab provides a range of advanced options.

The options are set in an option string that mimics the option string passed to
LAME in a command-line call. This derives from a time when kLAME called LAME in
this way. Retaining this form has advantages, namely that advanced LAME settings
not provided in the GUI can be set in the "more options" edit box.

@todo Add more LAME options to additional tabs in the options dialogue.
*/
class KLameOptionsDialogue : public QDialog
{
    Q_OBJECT
public:
    KLameOptionsDialogue(QWidget* parent = 0);
    ~KLameOptionsDialogue();
public:
    void setOptionString(QString& options);
    QString getOptionString() const;
    void setConversionDirectory(QString& dir);
    QString& getConversionDirectory();
    void setLameSettingsDirectory(QString& dir);
    QString getLameSettingsDirectory() const;
    void setFileTag(QString& tag);
    QString& getFileTag();
    void setColumnHeading(QString& name);
    QString& getColumnHeading();
    void setColumnNumber(int column);
    void setupDisplay();
    void setupInitialDefault();
    void wipeSettings();
    void setDefaultOptions(QString& options);
    void buildOptions();
    void initializeValidKeywords();
    void stripBadOptions(QString& options);
private slots:
    void on_browseDirectory_clicked();
    void on_loadSettingsSelect_clicked();
    void on_saveSettingsSelect_clicked();
    void on_qualityPref_toggled();
    void on_bitratePref_toggled();
    void on_useCbr_stateChanged();
    void on_useMaxBitrate_stateChanged();
    void on_changeQualityMeasure_stateChanged();
    void on_useResampleFrequency_stateChanged();
    void on_qualitySelect_valueChanged();
    void on_qualityDisplay_valueChanged(int);
    void on_bitrateSelect_valueChanged();
    void on_bitrateDisplay_valueChanged(int);
    void on_maxBitrateVbr_valueChanged();
    void on_maxBitrateDisplay_valueChanged(int);
    void on_abrBitrateSelect_valueChanged();
    void on_abrBitrateDisplay_valueChanged(int);
    void on_buttonOk_clicked();
    void on_buttonCancel_clicked();
private:
    QString columnName_;                  //!< Header for column.
    QString lameSettingsDirectory_;       //!< Directory for LAME settings file.
    QString lameOptions_;                 //!< LAME Options from user input.
    QString fileTag_;                     //!< Tag appended to filename.
    QString conversionDirectory_;         //!< Directory for mp3 output files.
    QStringList validKeywords;            //!< Table of valid option keywords.
    Ui::KLameOptionsDialogueBase optionsDialogueUi; // User Interface object
};

//-----------------------------------------------------------------------------
// General functions
//-----------------------------------------------------------------------------
QString parseOptions(QString& options, short n);
//-----------------------------------------------------------------------------

#endif
