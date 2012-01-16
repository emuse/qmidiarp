/**
 * @file parstore.h
 * @brief Member definitions for the ParStore class.
 *
 * @section LICENSE
 *
 *      Copyright 2009, 2010, 2011 <qmidiarp-devel@lists.sourceforge.net>
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
    TempStore temp;
    QList<TempStore> list;

    void tempToList(int ix);
/*!
* @brief This function reads all parameters of this LFO from an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to read from
*/
    void readData(QXmlStreamReader& xml);
/*!
* @brief This function writes all parameters of this LFO to an XML stream
* passed by the caller, i.e. MainWindow.
*
* @param xml QXmlStreamWriter to write to
*/
    void writeData(QXmlStreamWriter& xml);
/*!
* @brief This function allows ignoring one XML element in the XML stream
* passed by the caller.
*
* It also advances the stream read-in. It is used to
* ignore unknown elements for both-ways-compatibility
*
* @param xml reference to QXmlStreamReader containing the open XML stream
*/
    void skipXmlElement(QXmlStreamReader& xml);
};
#endif
