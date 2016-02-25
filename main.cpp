/**
@mainpage kLAME - Conversion of WAV Audio files to MP3
@version 2.0.5
@author Ken Sarkies (www.jiggerjuice.net)
@date 23 September 2007

This describes the program structure and detailed design for the kLAME MP3
conversion utility.

kLAME provides a GUI interface for conversion of WAV files to MP3 files using
the LAME MP3 conversion libraries. kLAME provides a graphical means to set and
store the various LAME parameters, and allows construction of a matrix of
conversions with a list of different WAV files to convert, and a set of
different MP3 compression ratios for each file.

kLAME uses threads to perform the conversions, giving a significant advantage
for multiprocessor architectures.
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

#include <qapplication.h>
#include "klamemainform.h"

int main(int argc,char ** argv)
{
    QApplication a(argc,argv);
    KLameMainForm w;
    w.show();
   return a.exec();
}
