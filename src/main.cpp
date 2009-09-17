#include <getopt.h>  
#include <QApplication>
#include <QFileInfo>
#include <QString>
#include <QTextStream>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include "mainwindow.h"
#include "main.h"


static struct option options[] = {
    {"version", 0, 0, 'v'},
    {"help", 0, 0, 'h'},
    {"portCount", 1, 0, 'p'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[])  
{
    int getopt_return;
    int option_index; 
    int portCount = 2;
    QTextStream out(stdout);

    while ((getopt_return = getopt_long(argc, argv, "vhp:", options,
                    &option_index)) >= 0) {
        switch(getopt_return) {
            case 'v':
                out << ABOUTMSG;
                out.flush();
                exit(EXIT_SUCCESS);
            case 'h':
                out << ABOUTMSG << endl;
				out << endl;
				out << "Usage" << endl;
				out << PACKAGE " [--portCount <num>] [<filename>]" << endl;
				out << PACKAGE " [-v]" << endl;
				out << PACKAGE " [-h]" << endl;
				out << endl;
				out << "Options" << endl;
				out << QString("--version      -v            "
						"Print application version") << endl;
				out << QString("--help         -h            "
						"Print this message") << endl;
                out << QString("--portCount    -p  <num>     "
                        "Number of output ports [%1]\n").arg(portCount) << endl;
                out.flush();
                exit(EXIT_SUCCESS);
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

    // translator for Qt library messages
    QTranslator qtTr;

    if (qtTr.load(QString("qt_") + loc.name(),
                QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTr);

    // translator for qmidiarp strings       
    QTranslator qmidiarpTr;

    if (qmidiarpTr.load(QString(PACKAGE "_") + loc.name(), TRANSLATIONSDIR))
        app.installTranslator(&qmidiarpTr);

    MainWindow* qmidiarp = new MainWindow(portCount);
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
