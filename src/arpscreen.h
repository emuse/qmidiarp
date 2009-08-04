#ifndef ARPSCREEN_H
#define ARPSCREEN_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qsizepolicy.h>
#include <qsize.h>


#define ARPSCREEN_MINIMUM_WIDTH   250
#define ARPSCREEN_MINIMUM_HEIGHT   80
#define ARPSCREEN_VMARGIN          10
#define ARPSCREEN_HMARGIN          16




class ArpScreen : public QWidget
{
  Q_OBJECT

  private:
    int maxRef;
    int globalMax, globalMaxResetCount;
    //QTimer *timer;
	QString pattern;
	QString a_pattern;

  protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void resizeEvent (QResizeEvent* );            
    
  public:
    ArpScreen(int p_maxRef, QWidget* parent=0);
    ~ArpScreen();
   virtual QSize sizeHint() const;
   virtual QSizePolicy sizePolicy() const;
   
  public slots: 
    void updateArpScreen(QString);
};
  
#endif
