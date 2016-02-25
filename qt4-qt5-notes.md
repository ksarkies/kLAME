The change from QT4 to QT5 was not as difficult as that from QT3 to QT4.
The following was needed:

* to klame.pro add QT += widgets

* Qt::WFlags is now Qt::WindowFlags

* #include <QtGui> is now #include <QtWidgets> (not used in this project)

* QFileDialog::setFilter(QDir::Filters filters) no longer will accept character
strings. Change to QFileDialog::setNameFilter(const QString & filter)
(former was deprecated in QT4.8).

* qAction needs to change signals "activated" to "triggered" (former was
deprecated in QT3).

* valueChanged() for qspinbox must now be valueChanged(int) even though no
integer value was passed, as there is now a valueChanged(QString). There were
four instances in the Lame options dialogue window. This only gave rise to some
execution time warnings.

