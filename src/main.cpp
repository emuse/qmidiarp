/**  @file main.cpp
 *   @brief Main program file. Instantiates the Application MainWindow.
 *
 *      Handles commandline arguments and options before MainWindow
 *      construction.
 *   @mainpage A MIDI Arpeggiator, LFO and Step Sequencer
 *   @section Description
 *      This attempts to give an overview of the architecture of this
 *      software.
 *
 *   @section LICENSE
 *
 *      Copyright 2009, 2010, 2011 <qmidiarp-devel@lists.sourceforge.net>
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; either version 2 of the License, or
 *      (at your option) any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with this program; if not, write to the Free Software
 *      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 *      MA 02110-1301, USA.
 *
 */
#include <getopt.h>
#include <QApplication>
#include <QFileInfo>
#include <QString>
#include <QTextStream>
#if defined(TRANSLATIONSDIR)
#include <QTranslator>
#endif
#include <QLocale>
#include <QLibraryInfo>

#include "mainwindow.h"
#include "main.h"


static struct option options[] = {
    {"version", 0, 0, 'v'},
    {"help", 0, 0, 'h'},
    {"alsa", 0, 0, 'a'},
    {"jack", 0, 0, 'j'},
    {"jack_session_uuid", required_argument, 0, 'U' },
    {"portCount", 1, 0, 'p'},
    {0, 0, 0, 0}
};

QString global_jack_session_uuid = "";

int main(int argc, char *argv[])
{
    int getopt_return;
    int option_index;
    int portCount = 2;
    bool alsamidi = false;

    QTextStream out(stdout);

    while ((getopt_return = getopt_long(argc, argv, "vhajUp:", options,
                    &option_index)) >= 0) {
        switch(getopt_return) {
            case 'v':
                out << ABOUTMSG;
                out.flush();
                exit(EXIT_SUCCESS);

            case 'h':
                out << "Usage: " PACKAGE " [OPTION] [FILENAME]" << endl;
                out << endl;
                out << "Options:" << endl;
                out << "  -v, --version            "
                    "Print application version" << endl;
                out << "  -h, --help               "
                    "Print this message" << endl;
                out << "  -a, --alsa               "
                    "Use ALSA MIDI interface" << endl;
                out << "  -j, --jack               "
                    "Use JACK MIDI interface (default)" << endl;
                out << QString("  -p, --portCount <num>    "
                        "Number of output ports [%1]").arg(portCount) << endl;
                out.flush();
                exit(EXIT_SUCCESS);

            case 'a':
                alsamidi = true;
                break;

            case 'j':
                alsamidi = false;
                break;
            case 'U':
                global_jack_session_uuid = QString(optarg);
                break;
            case 'p':
                portCount = atoi(optarg);
                if (portCount > MAX_PORTS)
                    portCount = MAX_PORTS;
                else if (portCount < 1)
                    portCount = 2;
                break;
        }
    }

    QApplication app(argc, argv);
    QLocale loc = QLocale::system();

#if defined(TRANSLATIONSDIR)
    // translator for Qt library messages
    QTranslator qtTr;

    if (qtTr.load(QString("qt_") + loc.name(),
                QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTr);

    // translator for qmidiarp messages
    QTranslator qmidiarpTr;

    if (qmidiarpTr.load(QString(PACKAGE "_") + loc.name(), TRANSLATIONSDIR))
        app.installTranslator(&qmidiarpTr);
#endif

    MainWindow* qmidiarp = new MainWindow(portCount, alsamidi);
    if (optind < argc) {
        QFileInfo fi(argv[optind]);
        if (fi.exists())
            qmidiarp->openFile(fi.absoluteFilePath());
        else
            qWarning("File not found: %s", argv[optind]);
    }

    int result = app.exec();
    delete qmidiarp;
    return result;
}
