kLAME 3.0.0
===========

A GUI wrapper converter of multiple wav to mp3 files using LAME.

This application provides a convenient gui for LAME, allowing a number of
conversions to be made in bulk for different LAME options (eg archival
compression and compression for transmission or website storage.

It was written back in 2006 partly as a means to develop familiarity with QT3
(hence the crappy code and obsolete structures), then QT4, and now QT5, and to
fill a gap left by the departure of a similar application in Windows, the name
of which I have forgotten. It has bugs and limitations but will probably not be
developed further unless the need for its features resurfaces.

The main window provides a table with a row for each file and a column for each
conversion option set that includes LAME options, a destination directory and a
filename tag used to describe and distinguish the converted files.

Build
-----

Refer to INSTALL-QT5

Main Form
---------

The window provides a toolbar with a set of buttons described below. The
window also shows a table in which a filename for conversion is stored in each
row and options for conversion are provided in each column. Columns are
provided with checkboxes that can be used to control if the filename will
be converted according to the options set for the particular column.

New Project. This clears the table of all settings and inserts defaults.

Open/Save Project. The full set of columns, options and LAME options can be
	saved for later restoration.

Open Files. This dialogue allows the user to select a list of filenames and
	have them added to a table for later conversion. Additional selections
	will result in additional files being added.

Remove File. When a row is selected, this button will cause the row to be
	deleted.

Add Columns. This will cause an additional column to be added after the last
	column, with default options.

Delete Column. Selection of a column of the table and clicking delete will
	remove the entries.

LAME Options. LAME options generally must be set for each column. These can
	be set for any selected column, or if no column is selected, for
	the last column. LAME are options described separately below.

Convert. Begin the conversion operation. This performs all conversions
	requested, and informs with a message box when complete. Interrupting
	the process will abort all conversions. Any file that has been
	partly converted will leave behind a dud mp3 file.

LAME Options
------------

This tabbed form provides for any option to be passed to LAME. You will need
to be familiar with LAME usage to use these effectively.

Tab 1: General Output options.
Column number. This refers to the column for which options are being set.
	Defaults to a selected column if present, or else to the last column.
	The column header name can be changed here for convenience.

Output directory. Allows selection of a different output directory for each
	column. Checks that this exists and suggests creating it if not.
	Reverts to the old directory name if unable to create one.

Filename Tag. Allows for a tag to be added to the end of a filename (before the
	first dot) to distinguish the different conversion products.

Save settings. This saves a simple one-line string identical to that passed to
	LAME as options. These options can be used for creating new projects
	with standard option settings.

Load settings. When loaded, the options string is parsed and the settings are
	made directly in the settings form.

Tab 2: Basic LAME Settings
Preference. This chooses between VBR (quality) and ABR/CBR (bitrate).

Relative Quality. This is a slider between 10 and 100, and provides the VBR
	quality setting associated with -v. The option associated with -v is
	determined by subtracting the slider value from 100 and dividing by 10.

VBR Algorithm. Chooses between --vbr-old and --vbr-new.

Bitrate and "use CBR". If CBR is checked, this sets --cbr and -b n. If not
	checked this sets --abr n. The slider provides the value of n.

Tab 3: Advanced LAME settings.
Mode (-m) Various mode settings.

Quality (-q). Setting from a spin box 0-9 with default 5.

Full Bandwidth (-k). No filtering applied.

High Pass Frequency (kHz).

High Pass Width (kHz).

Low Pass Frequency (kHz).

Low Pass Width (kHz).

Tonality Limit (kHz). Limit to which audible frequencies are processed.

Use Minimum VBR Bitrate (-b). If checked, a VBR minimum bitrate of 115kHz is
	used. This can cause problems with LAME if the value is too low
	compared to the average bitrate.

Use Maximum VBR Bitrate (-B). If checked, a maximum bitrate can be set on the
	slider. This can also cause problems with LAME.

Resample Frequency. If checked, the selected resample frequency (allowed values
	given in the spinbox) is used.

Change Quality Measure (-X). Measures 0 to 9 given in a spinbox.

Additional options. This allows any other options to be specified. These are
	parsed and invalid options are discarded if the checkbox is selected
	(default). Any options added that clash with those set on the form will
	take precedence. Any LAME option can be specified (beware that LAME may
	change options over time). Options relating to stdout outputs are
	ignored.

Project Save file structure
---------------------------

This is a binary file (QT format) with an ASCII header with the program name
and date, followed by:

1. Number of columns
2. Directory for the wav files
3. Comment strings
4. Filename tags
5. Output directories
6. LAME option strings
7. Column Header labels

Changes
-------

kLAME 3.0.0
1. Update to QT5, replacing some obsolete code hung over from QT3. Currently
   still suffering from some annoying bugs, but works.

kLAME 2.0.6
1. Added Doxygen documentation.
3. Fixed a bug that would not allow columns to be selected if no rows were
   shown. A trick had to be used, two rows inserted, one hidden and the other
   deleted allowed columns to be selected with no rows showing. QT4 sure is
   odd.

kLAME 2.0.5
1. Refactoring. Got rid of global variables and followed conventions
   suggested in http://geosoft.no/development/cppstyle.html.
2. Cleaned up filename processing code to make use of QFileInfo and QDir
   class functions to extract directory and build absolute filepaths.
3. Fixed two bugs that didn't save the directory if a file dialogue was
   cancelled.
4. Used signals and slot feature of QT to communicate progress between
   the threads and the progress dialogue. Beautiful!
5. Cleaned up the help page.
6. Added some code to allow correct working under QT4.2. When adding
   files for conversion, each row in the TableWidget is now explicitly
   made visible as the hideRow function seems to be doing odd things.
7. Added some code to check output directory existence when loading a
   project file.

kLAME 2.0-4
1. Added a check for an undefined directory in the LAME options dialogue,
   and an offer to create it.

kLAME 2.0-3
1. Fixed a problem accessing an array beyond its bounds that caused a
   segfault. This had an odd effect in that the program segfaulted when
   starting conversion only after a project save was done.
2. Fixed a problem where initial settings for main window size and location
   put it off screen under Windows. Changed to the QT recommended method.
3. Fixed a bug where kLAME would hang waiting for non-existent threads to
   finish. This happened when a box was unchecked. The threads were not
   started but the code still waited for them. Added a check on the
   checkboxes.
4. Fixed a bug where some directory paths did not have a trailing separator
   under windows. Not sure that this is elegant. A better approach would be
   to add the separator to create the filepath, then run cleaPath() on the
   result.
5. Minor change to lift a 512MB limitation on wav file size by changing from
   int to long.


kLAME 2.0-1
1. Added threaded operation to the conversions. This should speed up the
   conversions on multiple CPU machines.

kLAME 1.2-0
1. Port of the entire application across to QT4.
2. Added Menus - should have had these from the start
3. Added button to allow deletion of a selected row as QT4 does not have a
   straightforward means of testing if a column or row is selected.
4. Delete columns/rows button now deletes only the selected column (probably a
   better idea anyway).
5. Fixed a problem with sliders and spinboxes not properly synchronised.
6. Had to change the save project file format as QT4 has a problem with
   reading string constants back into a QString.
7. Added checks on file open problems.
8. Added a LAME flush call at the end of each conversion that was overlooked.

kLAME 1.1-1
1. Changed labels to appear in boxes to overcome variation of location with
   different distros.
2. Fixed a couple of annoying bugs, namely open files had two actions causing
   it to open twice.

kLAME 1.1-0
1. Changed the system call to the LAME frontend to a direct access of the API.
   This requires reading and interpreting the WAVE file format, and also
   parsing the LAME options to set them directly in LAME. At present only 1
   or 2 channels, and 8 or 16 bit samples are allowed. Any WAVE header
   parameters that do not look right cause the program to abort the conversion.
2. Added an informative progress bar that looks quite natty.
3. Pulled some functions out of the KLameOptionsDialogue class to make them
   more generally accessible to other modules. These relate to parsing of
   options.

kLAME 1.0-5
1. Added the New Project icon and code to clear the table of all settings.

kLAME 1.0-4
1. Added code to test checkbox in table and enable/disable conversion.

kLAME 1.0-3
1. Added some text identifiers for the toolbar buttons.
2. Allow column headers to be editable.
3. Fixed problem where new columns had an incorrect header after loading a
   project
4. Delete columns on loading a project if number of columns is larger than
   those in the project.
5. Added Help page.

kLAME 1.0-2
1. Fixed a problem where columns could not be deleted if no rows were present.
2. Made all settings persistent when re-entering options dialogue. All
   filenames and directories also persistent.
3. Added save settings on exit and load settings on startup.

K. Sarkies, 9 February 2016
