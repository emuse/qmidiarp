/**  @file main.cpp
 *   @brief Main program file. Instantiates the Application MainWindow.
 *
 *      Handles commandline arguments and options before MainWindow
 *      construction.
 *   @mainpage A MIDI Arpeggiator, LFO and Step Sequencer
 *   @section Description
 * This functional documentation attempts to give an overview of the
 * architecture of QMidiArp.
 * QMidiArp is an advanced MIDI arpeggiator, programmable step sequencer
 * and LFO for Linux. It can hold any number of arpeggiator, sequencer,
 * or LFO modules running in parallel. It has support for
 * JACK MIDI and ALSA MIDI backends. The modules are also available as
 * LV2 MIDI plugins. QMidiArp runs on Linux and uses the Qt toolkit.
 * Arpeggiator modules produce
 * sequences depending on the notes sent to their input port, which is
 * typically connected to a keyboard or another sequencer.
 * Step sequencer modules allow you to create simple linear, monophonic
 * and globally transposable sequences similar to the first analog
 * sequencers. MIDI LFO modules independently produce MIDI controller
 * data of adjustable waveform, time resolution, amplitude and duration.
 * A Global Storage Tool can store different setups and switch between
 * them at a given time. It allows you to dynamically combine patterns
 * and LFO wave forms. For each module, an input note filter is
 * available, and the output port and channel can be set independently.
 * QMidiArp works with an internal tick resolution of 192 ticks per beat.
 * It can be synchronized to an incoming MIDI realtime clock or as a
 * JACK transport client. Most of the relevant control elements are
 * accessible via MIDI controller through a MIDI-learn infrastructure.
 * QMidiArp also has a log tool displaying the history of incoming MIDI
 * events in colors depending on their type. 
 *
 *   @section AUTHORS
 * Frank Kober 2009 - 2017 <BR> 
 * Nedko Arnaudov 2011 <BR> 
 * Guido Scholz 2009 <BR> 
 * Matthias Nagorni 2003 <BR> 
 * 
 *
 *      Copyright 2009 - 2017 <qmidiarp-devel@lists.sourceforge.net>
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
#ifdef HAVE_ALSA
    {"alsa", 0, 0, 'a'},
    {"jack", 0, 0, 'j'},
#endif
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
    QString s;

    QTextStream out(stdout);
    srand(getpid());
    while ((getopt_return = getopt_long(argc, argv, "vhajUp:", options,
                    &option_index)) >= 0) {
        switch(getopt_return) {
            case 'v':
                s = QString(ABOUTMSG);
                s.replace(QString("<br/>"), QString("\n"));
                s.replace(QString("</p>"), QString("\n"));
                s.remove(QRegExp("<[^>]*>"));
                out << s;
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
#ifdef HAVE_ALSA
                out << "  -a, --alsa               "
                    "Use ALSA MIDI interface" << endl;
                out << "  -j, --jack               "
                    "Use JACK MIDI interface (default)" << endl;
#endif
                out << QString("  -p, --portCount <num>    "
                        "Number of output ports [%1]").arg(portCount) << endl;
                out.flush();
                exit(EXIT_SUCCESS);
#ifdef HAVE_ALSA
            case 'a':
                alsamidi = true;
                break;
            case 'j':
                alsamidi = false;
                break;
#endif
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

    MainWindow* qmidiarp = new MainWindow(portCount, alsamidi, argv[0]);
    if (optind < argc) {
        QFileInfo fi(argv[optind]);
        if (fi.exists())
            qmidiarp->openFile(fi.absoluteFilePath());
        else
            qWarning("File not found: %s", argv[optind]);
    }
    int result = -1;
    if (!qmidiarp->jackFailed)
        result = app.exec();

    delete qmidiarp;
    return result;
}
