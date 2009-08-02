#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>
#include <getopt.h>  
#include <qapplication.h>
#include <qmainwindow.h>
#include <qmenubar.h>   
#include <qmenu.h>

#include <qstring.h>
#include "gui.h"
#include "main.h"

static struct option options[] =
        {{"help", 0, 0, 'h'},
         {"portCount", 1, 0, 'p'},
         {"file", 1, 0, 'f'},
         {0, 0, 0, 0}};

int main(int argc, char *argv[])  
{
  QApplication *app = new QApplication(argc, argv);
  QMainWindow *top = new QMainWindow();
  int getopt_return;
  int option_index; 
  int portCount = 2;
  bool havePreset = false;
  QString fileName; 

  while((getopt_return = getopt_long(argc, argv, "hp:f:", options, &option_index)) >= 0) {
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
  Gui *gui = new Gui(portCount, top);
  QMenuBar *menuBar = new QMenuBar; 
  QMenu *filePopup = new QMenu("&File",top); 
  QMenu *aboutMenu = new QMenu("&About",top);
 
  filePopup->addAction("&Load", gui, SLOT(load()));
  filePopup->addAction("&Save", gui, SLOT(save()));
  filePopup->addAction("&Quit", app, SLOT(quit()));
  aboutMenu->addAction("About QMidiArp", gui, SLOT(displayAbout())); 
  menuBar->addMenu(filePopup);
  menuBar->addMenu(aboutMenu);
  
  top->setWindowTitle("QMidiArp");
  top->setMenuBar(menuBar);
  top->setCentralWidget(gui); 
  top->show();
  
  if (havePreset) {
    gui->load(fileName);
  }
  return app->exec(); 
}
