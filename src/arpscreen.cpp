/*
 *      arpscreen.cpp
 *      
 *      This program is part of QMidiArp.
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
 */


#include <stdio.h>
#include <qpolygon.h>
#include <stdlib.h>
#include <unistd.h>
#include <qwidget.h>
#include <qstring.h>
#include <qpainter.h>
#include <qpaintdevice.h>
#include <qpen.h>
#include <qpixmap.h>
#include <qbrush.h>
#include <qtimer.h>
#include <qsizepolicy.h>
#include <qsize.h>
#include <qboxlayout.h>
#include <math.h>
#include <qtabwidget.h>
#include "arpscreen.h"
#include "midiarp.h"
#include "arpwidget.h"




// Constructor.
ArpScreen::ArpScreen(int p_maxRef, QWidget* parent) 
             : QWidget (parent)
{
  maxRef = p_maxRef;
  globalMaxResetCount = 0;
  globalMax = 0;
  setPalette(QPalette(QColor(0, 20, 100), QColor(0, 20, 100)));
  a_pattern=" ";
  //timer = new QTimer(this);
  //QObject::connect(timer, SIGNAL(timeout()), this, SLOT(updateArpScreen()));
  //timer->start(200);

  }
// Destructor.

ArpScreen::~ArpScreen()
{
}
// Paint event handler.
void ArpScreen::paintEvent(QPaintEvent * ) {

//  QPixmap pm(width(), height());  
  QPainter p(this);
  QPolygon points(7);
  QPen pen;
  int l1, l2;
  int beat = 4;
  double nsteps = 0.0;
  double tempo = 1.0;
  double curstep = 0.0;
  int nlines = 0; 
  int notelen;
  double ypos = 0.;
  int octYoffset;
  int w = QWidget::width();
  int h = QWidget::height();
  
  double len, xscale, yscale, ofs;
  int x, x1;
  int patternMaxIndex, patternLen;
  bool chordMode = false;
  int octave = 0;
  int maxOctave = 0;
  int minOctave = 0;
  double minTempo = 1.0;
  int noctaves=1;
  double vel =1.0;
  QChar c;
   
  patternLen = a_pattern.length();
  patternMaxIndex = 0;
  
  for (l1 = 0; l1 < patternLen; l1++) {
    c = a_pattern.at(l1);
 	 
	if (c.isDigit()) {
		if (!chordMode) {
			nsteps = nsteps + tempo; 
		}
		if (c.digitValue() > patternMaxIndex) patternMaxIndex = c.digitValue();
		 
		}
	switch(c.toAscii()) {
          case '(': {
			  chordMode = true;
			  nsteps = nsteps + tempo; 
		  }
            break;
          case ')': {
			  chordMode = false;
		  }
		  break;
		  case '>': {
			  tempo /= 2.0;
  			  if (tempo < minTempo) minTempo /= 2.0;
			  	    }
            break;
          case '<': {
			  tempo *= 2.0;
			  }
            break;
          case '.': {
			  if (!chordMode) nsteps = nsteps + tempo;
			  tempo = 1.0;
		  }
          case 'p': {
			  if (!chordMode) nsteps = nsteps + tempo;
			  }
            break;
		  case '+': {
			 octave++;
			 if (octave > maxOctave) maxOctave++;
		 }
            break;
          case '-': {
 			 octave--;
			 if (octave < minOctave) minOctave--;
		 }
           break;
          case '=': octave=0;
            break;
          case '/': vel += 0.2;
            break;
          case '\\': vel -= 0.2;
            break; 
          case 'd': notelen *= 2;
            break;
          case 'h': notelen /= 2;
            break;
		  default: ;
	}
	
  }
//printf("nsteps : %d",(int)nsteps);

	
 //Green Filled Frame
  p.fillRect(0, 0, w, h, QColor(10, 50, 10));
  p.setViewport(0, 0, width(), height());
  p.setWindow(0, 0, width(), height());
  pen.setColor(QColor(20, 160, 20));
  pen.setWidth(1);
  p.setPen(pen);
  p.drawRect(0, 0, width() - 1, height() - 1);
  
 //Grid 
  len = (double)nsteps;  
  xscale = (double)(width() - 2 * ARPSCREEN_HMARGIN) / len;
  yscale = (double)height() - 2 * ARPSCREEN_VMARGIN;
  
    //Beat separators
  for (l1 = 0; l1 < nsteps + 1; l1++) {
         
	if (l1 < 10) {
      ofs = width() / len * .5 - 4 + ARPSCREEN_HMARGIN;
     } else {
	  ofs = width() / len * .5 - 6 + ARPSCREEN_HMARGIN;
}
	if ((bool)(l1%beat)) {
          pen.setColor(QColor(60, 180, 60));
		        } else {
		  pen.setColor(QColor(60, 180, 150));	  
	  }
    p.setPen(pen);
    x = (int)((double)l1 * xscale);
    p.drawLine(ARPSCREEN_HMARGIN + x, ARPSCREEN_VMARGIN, ARPSCREEN_HMARGIN + x, height()-ARPSCREEN_VMARGIN);
	
	 
    if (l1 < nsteps) {
		//Beat numbers
	
		p.setFont(QFont("Helvetica", 8));
		p.drawText(ofs + x, ARPSCREEN_VMARGIN, QString::number(l1+1));
	
        // Beat divisor separators
	    pen.setColor(QColor(20, 80, 20));
        p.setPen(pen);
	
	    for (l2 = 1; l2 < 1.0/minTempo; l2++) {
		    x1 = x + l2 * xscale * minTempo;
			if (x1 < xscale*len)
	        p.drawLine(ARPSCREEN_HMARGIN + x1, ARPSCREEN_VMARGIN, ARPSCREEN_HMARGIN + x1, height()-ARPSCREEN_VMARGIN);
	    } 
  }
}

  //Octave separators and numbers
		
    pen.setWidth(1);
	pen.setColor(QColor(40, 120, 40));
    p.setPen(pen);
    noctaves = maxOctave - minOctave + 1;
	 
	for (l1 = 0; l1 < noctaves + 1; l1++) {
             p.drawLine(ARPSCREEN_HMARGIN, yscale * l1 / noctaves + ARPSCREEN_VMARGIN, 
             width() - ARPSCREEN_HMARGIN, 
			 yscale * l1 / noctaves + ARPSCREEN_VMARGIN);		 
		     p.drawText(ARPSCREEN_HMARGIN / 2 - 3, 
		     yscale * (l1 + 0.5) / noctaves + ARPSCREEN_VMARGIN + 4, 
		     QString::number(noctaves - l1 + minOctave - 1));

		 }	 
		 
  //Draw arpTicks
  
   curstep=0.0;
   notelen=xscale/8;
   tempo =1.0;
   vel=0.8;
   octave=0;
   chordMode = false;
 
   pen.setWidth(1);
   p.setPen(pen);
for (l1 = 0; l1 < patternLen; l1++) {
	c = a_pattern.at(l1);
	if (c.isDigit()) {
		 nlines=c.digitValue()+1;
		 if (!chordMode) {
			 curstep = curstep + tempo; 
		 }
		 //printf("c was digit,   "); 
		 } else { switch(c.toAscii()) {
		 //printf("c was not digit,    "); 
          case '(': {
			  chordMode = true;
			  curstep = curstep + tempo; 
		  }
            break;
          case ')': {
			  chordMode = false;
		  }
		  break;
		  case '>': {
			  tempo /= 2.0;
			  }
            break;
          case '<': {
			  tempo *= 2.0;
			  }
            break;
          case '.': {
			  curstep = curstep + tempo;
			  tempo = 1.0;
		  }
            break;           
          case 'p': {
			  if (!chordMode) curstep = curstep + tempo;
			  }
            break;
          case '+': octave++;
            break;
          case '-': octave--;
            break;
          case '=': octave=0;
            break;         
          case '/': vel += 0.2;
            break;
          case '\\': vel -= 0.2;
            break; 
          case 'd': notelen *= 2;
            break;
          case 'h': notelen /= 2;
            break;
		  default: ;//printf("c: no match\n");
        }   
     }
		if (c.isDigit()) {
			//printf("curstep: %f     nlines: %d\n",curstep, (int)nlines); 
		
		x = (int)((curstep - tempo) * xscale);
		octYoffset = (octave - minOctave) * (patternMaxIndex+1);

			if (nlines > 0) {
				   ypos = nlines-1;
				   p.fillRect(ARPSCREEN_HMARGIN + x, 
				   yscale * (1. - (ypos + octYoffset) / (patternMaxIndex+1) / noctaves) + ARPSCREEN_VMARGIN - 3, 
				   notelen, 3, QColor(80+60*(vel-.8), 160+40*(vel-.8), 80+60*(vel-.8)));
					  }
		}
   }
  
  }

void ArpScreen::updateArpScreen(QString b_pattern) {
	
  a_pattern = b_pattern;
  ArpScreen::update();
 // timer->start(200);
}
QSize ArpScreen::sizeHint() const {

  return QSize(ARPSCREEN_MINIMUM_WIDTH, ARPSCREEN_MINIMUM_HEIGHT); 
}

QSizePolicy ArpScreen::sizePolicy() const {

  return QSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

void ArpScreen::resizeEvent (QResizeEvent *pResizeEvent )
{

	QWidget::resizeEvent(pResizeEvent);
}

