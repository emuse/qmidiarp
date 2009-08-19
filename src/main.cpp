#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>
#include <getopt.h>  
#include <QApplication>
#include <QMainWindow>
#include <QMenuBar>   
#include <QMenu>
#include <QString>
#include <QWidget>
#include <QSpinBox>
#include <QPixmap>
#include <QToolButton>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>

#include "gui.h"
#include "main.h"
#include "pixmaps/qmidiarp2.xpm"


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


    QMainWindow *top = new QMainWindow();
    Gui *gui = new Gui(portCount, top);
    QMenuBar *menuBar = new QMenuBar; 
    QMenu *filePopup = new QMenu(QMenu::tr("&File"),top); 
    QMenu *aboutMenu = new QMenu(QMenu::tr("&Help"),top);

    filePopup->addAction(QMenu::tr("&Open..."), gui, SLOT(load()));
    filePopup->addAction(QMenu::tr("&Save"), gui, SLOT(save()));
    filePopup->addAction(QMenu::tr("&Quit"), &app, SLOT(quit()));
    aboutMenu->addAction(QMenu::tr("&About %1...").arg(PACKAGE), gui,
            SLOT(displayAbout())); 
    menuBar->addMenu(filePopup);
    menuBar->addMenu(aboutMenu);


    top->setWindowTitle(PACKAGE);
    top->setWindowIcon(QPixmap(qmidiarp2_xpm));
    top->setMenuBar(menuBar);
    top->setCentralWidget(gui); 
    top->show();

    if (havePreset) {
        gui->load(fileName);
    }
    return app.exec(); 
}
