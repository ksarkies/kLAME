//          kLAME GUI frontend wrapper for LAME - conversion control wav to mp3
//              Help Dialogue Subclass

/***************************************************************************
 *   Copyright (C) 2006 by Ken Sarkies                                     *
 *   ksarkies@trinity.asn.au                                               *
 *                                                                         *
 *   This file is part of kLAME.                                       *
 *                                                                         *
 *   kLAME is free software; you can redistribute it and/or modify     *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   kLAME is distributed in the hope that it will be useful,          *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with kLAME if not, write to the                            *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.             *
 ***************************************************************************/

#include "ui_helpbase.h"
#include <QDialog>
#include <QWidget>

//-----------------------------------------------------------------------------
/** @brief kLAME help window

@todo Add some control buttons to the help dialogue.
*/

class Help : public QDialog
{
    Q_OBJECT
public:
    Help(QWidget* parent = 0);
    ~Help();
private:
    Ui::HelpBase helpUi;
};
