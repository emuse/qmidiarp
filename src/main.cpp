#include <stdio.h>      
#include <stdlib.h>     
#include <unistd.h>
#include <getopt.h>  
#include <qapplication.h>
#include <qmainwindow.h>
#include <qmenubar.h>   
#include <qpopupmenu.h>
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
  QApplication *qApp = new QApplication(argc, argv);
  QMainWindow *top = new QMainWindow();
  top->setCaption("QMidiArp");
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
        printf("\n%s\n", aboutText.latin1());
        printf("--file <name>         Load file\n\n");
        printf("--portCount <num>     Number of Output Ports [2]\n\n");
        exit(EXIT_SUCCESS);
    }
  }
  Gui *gui = new Gui(portCount, top);
  QPopupMenu *filePopup = new QPopupMenu(top); 
  QPopupMenu *aboutMenu = new QPopupMenu(top);
  top->menuBar()->insertItem("&File", filePopup);
  top->menuBar()->insertSeparator(); top->menuBar()->insertItem("&About", aboutMenu); 
  filePopup->insertItem("&Load", gui, SLOT(load()));
  filePopup->insertItem("&Save", gui, SLOT(save()));
  filePopup->insertItem("&Quit", qApp, SLOT(quit()));
  aboutMenu->insertItem("About QMidiArp", gui, SLOT(displayAbout())); 
  top->setCentralWidget(gui); 
  top->show();
  if (havePreset) {
    gui->load(fileName);
  }
  qApp->setMainWidget(top); 
  return qApp->exec(); 
}
