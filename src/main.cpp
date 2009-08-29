#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>
#include <getopt.h>  
#include <QApplication>
#include <QString>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include "mainwindow.h"
#include "main.h"


static struct option options[] = {
    {"help", 0, 0, 'h'},
    {"portCount", 1, 0, 'p'},
    {"file", 1, 0, 'f'},
    {0, 0, 0, 0}
};

int main(int argc, char *argv[])  
{
    int getopt_return;
    int option_index; 
    int portCount = 2;
    bool havePreset = false;
    QString fileName; 


    while((getopt_return = getopt_long(argc, argv, "hp:f:", options,
                    &option_index)) >= 0) {

        switch(getopt_return) {
            case 'p':
                portCount = atoi(optarg);
                if (portCount > MAX_PORTS) portCount = MAX_PORTS;
                break;
            case 'f':
                havePreset = true;
                fileName = optarg;
                break;
            case 'h':
                printf("\n%s\n", qPrintable(aboutText));
                printf("--file <name>         Load file\n\n");
                printf("--portCount <num>     Number of Output Ports [2]\n\n");
                exit(EXIT_SUCCESS);
        }
    }

    QApplication app(argc, argv);

    // translator for Qt library messages
    QTranslator qtTr;
    QLocale loc = QLocale::system();

    if (qtTr.load(QString("qt_") + loc.name(),
                QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
        app.installTranslator(&qtTr);

    // translator for qmidiarp strings       
    QTranslator qmidiarpTr;

    if (qmidiarpTr.load(QString(PACKAGE "_") + loc.name(), TRANSLATIONSDIR))
        app.installTranslator(&qmidiarpTr);

    new MainWindow(fileName, portCount);
 
    return app.exec(); 
}
