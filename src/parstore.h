/**
 * @file parstore.h
 * @brief Member definitions for the ParStore class.
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011, 2012 <qmidiarp-devel@lists.sourceforge.net>
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

#ifndef PARSTORE_H
#define PARSTORE_H

#include <QObject>
#include <QWidget>
#include <QList>
#include <QMenu>
#include <QString>
#include <QToolButton>
#include <QVector>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "globstore.h"
#include "indicator.h"

#ifndef SAMPLE_H
#define SAMPLE_H

/*! @brief Structure holding elements of a MIDI note or controller representing
 * one point of a waveform
 */
    struct Sample {
        int value;
        int tick;
        bool muted;
    };
#endif

/*! @brief This array holds the currently available LFO resolution values.
 */
const int lfoResValues[9] = {1, 2, 4, 8, 16, 32, 64, 96, 192};

/*! @brief This array holds the currently available Seq resolution values.
 */
const int seqResValues[5] = {1, 2, 4, 8, 16};

/*!
 * @brief Provides a parameter storage for QMidiArp module widgets.
 *
 * ParStore is used by each of QMidiArp's module widgets.
 */
class ParStore : public QWidget
{
  Q_OBJECT

  public:
    ParStore(GlobStore *p_globStore, const QString &name = "", QWidget* parent = 0);
    ~ParStore();
    GlobStore *globStore;
    QToolButton *topButton;
    Indicator *ndc;
    QList<int> jumpToList;
    QMenu *locContextMenu;
    QMenu *jumpToIndexMenu;
    QActionGroup *jumpToGroup;
    int activeStore;
    int currentRequest;

    struct TempStore {
        bool empty;
        bool muteOut;
        int res;
        int size;
        int loopMode;
        int waveForm;
        int portOut;
        int channelOut;
        int chIn;
        QVector<Sample> wave;
        QVector<bool> muteMask;
        /* LFO Modules */
        int ccnumber;
        int ccnumberIn;
        int freq;
        int ampl;
        int offs;
        /* Seq Modules */
        int loopMarker;
        int notelen;
        int vel;
        int transp;
        int dispVertical;
        /* Arp Modules */
        int indexIn0;
        int indexIn1;
        int rangeIn0;
        int rangeIn1;
        int attack;
        int release;
        int repeatMode;
        int rndTick;
        int rndLen;
        int rndVel;
        QString pattern;
    };
    TempStore temp; /**< Structure to which all module parameters are copied
                        before being appended to the TempStore::list*/
    QList<TempStore> list; /**< List of TempStore::temp structures for
                        parameter storage*/

    int restoreRequest; /**< When this variable is greater than -1,
                        ParStore::updateDisplay() will cause
                        ParStore::restoreParams(restoreRequest) at pattern
                        end*/
    int oldRestoreRequest; /**< Contains the last active location for jumping back*/
    bool restoreRunOnce; /**< Signals to ParStore::updateDisplay() that only one
                        run is done and then a restore is required */
    bool needsGUIUpdate; /**< Signals to ParStore::updateDisplay() that the location
                        button color has to be updated*/
    int dispReqIx, dispReqSelected; /**< When ParStore::needsGUIUpdate is true,
                        ParStore::updateDisplay() sets colors to these values*/

/*!
* @brief stores all module parameters to TempStore::temp and stores temp
* in TempStore::list at index ix. If the given index is greater than the list size,
* temp is appended to TempStore::list.
*
* @param ix Index at which the parameters are stored.
*/
    void tempToList(int ix);
/*!
* @brief reads the ParStore::list from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief writes the ParStore::list to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);

/*!
* @brief sets ParStore::restoreRequest and ParStore::restoreRunOnce to the
* location specified
*
* @param ix Location index to be restored at pattern end
*/
    void setRestoreRequest(int ix);
/*!
* @brief sets the color of location button row to the specified color index
*
* @param row Location index at which button color is to be set
* @param color Location color (0: no color, 1: active color, 2: pending color)
*/
    void setBGColorAt(int row, int color);
/*!
* @brief is called by the parent widget and part of the display timer event loop.

* sets the indicator position and handles storage requests as a function of the
* GUI requests and series parameters
*
* @param frame Current frame position of the parent module
* @param reverse Set to true if the parent module currently plays backward
*/
    void updateDisplay(int frame, bool reverse);

  signals:
/*!
* @brief connected to the parent widget and causing parameter storage to the
* location ix
*
* @param ix Storage location
* @param empty True if no parameters are stored and only the template is added
*/
    void store(int ix, bool empty);
/*!
* @brief connected to the parent widget and causing immediate parameter restore from
* location ix
*
* @param ix Storage location
*/
    void restore(int ix);

  public slots:

/*!
* @brief a signal mapper for routing the ParStore::jumpToIndexMenu signals
* to the ParStore::updateRunOnce() slot
*
* @param *action Pointer to the action that has been activated in the menu
*/
    void mapJumpToGroup(QAction *action);
/*!
* @brief called by ParStore::mapJumpToGroup(). Configures the location
* behavior at pattern end
*
* The choices are -2 for "Stay here" (no jumps at pattern end), -1 for
* returning to the previous location (ParStore::oldRestoreRequest) or (if
* zero or above) the location to jump to at pattern end.
*
* @param location Location to be configured
* @param choice -2 (no jumps), -1 (return to previous),
* >=0 (next location to jump to)
*/
    void updateRunOnce(int location, int choice);
/*!
* @brief adds all storage location GUI elements to the widget
*
* It also adds a new element to the ParStore::jumpToList
*/
    void addLocation();
/*!
* @brief removes and deletes all storage location GUI elements
*
* It also removes the parameter storage structure from ParStore::list and
* the element from ParStore::jumpToList
*
* @param ix Location index to be removed
*/
    void removeLocation(int ix);
    void mapRestoreSignal();
    void mapStoreSignal();
    void requestDispState(int ix, int selected);
    void setDispState(int ix, int selected);
    void showLocContextMenu(const QPoint &pos);
};
#endif
