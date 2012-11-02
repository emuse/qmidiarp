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
#include <QList>
#include <QString>
#include <QVector>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

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
class ParStore : public QObject
{
  Q_OBJECT

  public:
    ParStore();
    ~ParStore();
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

    int restoreRequest;
    int oldRestoreRequest;
    bool restoreRunOnce;
/*!
* @brief stores all module parameters to TempStore::temp and stores temp
* in TempStore::list at index ix. If the given index is greater than the list size,
* temp is appended to TempStore::list.
*
* @param ix index at which the parameters are stored.
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

    void setRestoreRequest(int ix, bool runOnce);
};
#endif
