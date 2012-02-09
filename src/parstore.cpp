/**
 * @file parstore.cpp
 * @brief Implements the ParStore class.
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

#include "main.h"
#include "parstore.h"

ParStore::ParStore()
{
    // when temp.empty is true, restoring from that set is ignored
    temp.empty = false;
    temp.muteOut = false;
    temp.res = 1;
    temp.size = 0;
    temp.loopMode = 0;
    temp.waveForm = 0;
    temp.portOut = 0;
    temp.channelOut = 0;
    temp.chIn = 0;
    temp.wave.clear();
    temp.muteMask.clear();
    /* LFO Modules */
    temp.ccnumber = 0;
    temp.ccnumberIn = 0;
    temp.freq = 0;
    temp.ampl = 0;
    temp.offs = 0;
    /* Seq Modules */
    temp.loopMarker = 0;
    temp.notelen = 0;
    temp.vel = 0;
    temp.transp = 0;
    temp.dispVertical = 0;
    /* Arp Modules */
    temp.indexIn0 = 0;
    temp.indexIn1 = 0;
    temp.rangeIn0 = 0;
    temp.rangeIn1 = 0;
    temp.attack = 0;
    temp.release = 0;
    temp.repeatMode = 0;
    temp.rndTick = 0;
    temp.rndLen = 0;
    temp.rndVel = 0;
    temp.pattern = "";
    list.clear();
}

ParStore::~ParStore()
{
}

void ParStore::tempToList(int ix)
{
    if (ix >= list.size()) {
        list.append(temp);
    }
    else {
        list.replace(ix, temp);
    }
}

void ParStore::writeData(QXmlStreamWriter& xml)
{
    int l1;
    QByteArray tempArray;

    xml.writeStartElement("globalStores");

    for (int ix = 0; ix < list.size(); ix++) {
        xml.writeStartElement("parStore");
        xml.writeAttribute("ID", QString::number(ix));
            xml.writeTextElement("empty", QString::number(list.at(ix).empty));
            xml.writeTextElement("muteOut", QString::number(list.at(ix).muteOut));
            xml.writeTextElement("res", QString::number(list.at(ix).res));
            xml.writeTextElement("size", QString::number(list.at(ix).size));
            xml.writeTextElement("loopMode", QString::number(list.at(ix).loopMode));
            xml.writeTextElement("waveForm", QString::number(list.at(ix).waveForm));
            xml.writeTextElement("portOut", QString::number(list.at(ix).portOut));
            xml.writeTextElement("channelOut", QString::number(list.at(ix).channelOut));
            xml.writeTextElement("chIn", QString::number(list.at(ix).chIn));
            xml.writeTextElement("ccnumber", QString::number(list.at(ix).ccnumber));
            xml.writeTextElement("ccnumberIn", QString::number(list.at(ix).ccnumberIn));
            xml.writeTextElement("freq", QString::number(list.at(ix).freq));
            xml.writeTextElement("ampl", QString::number(list.at(ix).ampl));
            xml.writeTextElement("offs", QString::number(list.at(ix).offs));
            xml.writeTextElement("loopMarker", QString::number(list.at(ix).loopMarker));
            xml.writeTextElement("notelen", QString::number(list.at(ix).notelen));
            xml.writeTextElement("vel", QString::number(list.at(ix).vel));
            xml.writeTextElement("dispVertical", QString::number(list.at(ix).dispVertical));
            xml.writeTextElement("transp", QString::number(list.at(ix).transp));
            xml.writeTextElement("indexIn0", QString::number(list.at(ix).indexIn0));
            xml.writeTextElement("indexIn1", QString::number(list.at(ix).indexIn1));
            xml.writeTextElement("rangeIn0", QString::number(list.at(ix).rangeIn0));
            xml.writeTextElement("rangeIn1", QString::number(list.at(ix).rangeIn1));
            xml.writeTextElement("attack", QString::number(list.at(ix).attack));
            xml.writeTextElement("release", QString::number(list.at(ix).release));
            xml.writeTextElement("repeatMode", QString::number(list.at(ix).repeatMode));
            xml.writeTextElement("rndTick", QString::number(list.at(ix).rndTick));
            xml.writeTextElement("rndLen", QString::number(list.at(ix).rndLen));
            xml.writeTextElement("rndVel", QString::number(list.at(ix).rndVel));
            xml.writeTextElement("pattern", list.at(ix).pattern);

            tempArray.clear();
            l1 = 0;
            while (l1 < list.at(ix).muteMask.count()) {
                tempArray.append(list.at(ix).muteMask.at(l1));
                l1++;
            }
            xml.writeStartElement("muteMask");
                xml.writeTextElement("data", tempArray.toHex());
            xml.writeEndElement();

            tempArray.clear();
            l1 = 0;
            while (l1 < list.at(ix).wave.count()) {
                tempArray.append(list.at(ix).wave.at(l1).value);
                l1++;
            }
            xml.writeStartElement("wave");
                xml.writeTextElement("data", tempArray.toHex());
            xml.writeEndElement();
        xml.writeEndElement();
    }
    xml.writeEndElement();
}

void ParStore::readData(QXmlStreamReader& xml)
{
    int ix = 0;

    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isEndElement())
            break;

        if (xml.isStartElement() && (xml.name() == "parStore")) {
            while (!xml.atEnd()) {
                xml.readNext();
                if (xml.isEndElement())
                    break;
                if (xml.name() == "empty")
                    temp.empty = xml.readElementText().toInt();
                else if (xml.name() == "muteOut")
                    temp.muteOut = xml.readElementText().toInt();
                else if (xml.name() == "res")
                    temp.res = xml.readElementText().toInt();
                else if (xml.name() == "size")
                    temp.size = xml.readElementText().toInt();
                else if (xml.name() == "loopMode")
                    temp.loopMode = xml.readElementText().toInt();
                else if (xml.name() == "waveForm")
                    temp.waveForm = xml.readElementText().toInt();
                else if (xml.name() == "portOut")
                    temp.portOut = xml.readElementText().toInt();
                else if (xml.name() == "channelOut")
                    temp.channelOut = xml.readElementText().toInt();
                else if (xml.name() == "chIn")
                    temp.chIn = xml.readElementText().toInt();
                else if (xml.name() == "ccnumber")
                    temp.ccnumber = xml.readElementText().toInt();
                else if (xml.name() == "ccnumberIn")
                    temp.ccnumberIn = xml.readElementText().toInt();
                else if (xml.name() == "freq")
                    temp.freq = xml.readElementText().toInt();
                else if (xml.name() == "ampl")
                    temp.ampl = xml.readElementText().toInt();
                else if (xml.name() == "offs")
                    temp.offs = xml.readElementText().toInt();
                else if (xml.name() == "vel")
                    temp.vel = xml.readElementText().toInt();
                else if (xml.name() == "dispVertical")
                    temp.dispVertical = xml.readElementText().toInt();
                else if (xml.name() == "transp")
                    temp.transp = xml.readElementText().toInt();
                else if (xml.name() == "notelen")
                    temp.notelen = xml.readElementText().toInt();
                else if (xml.name() == "loopMarker")
                    temp.loopMarker = xml.readElementText().toInt();
                else if (xml.name() == "indexIn0")
                    temp.indexIn0 = xml.readElementText().toInt();
                else if (xml.name() == "indexIn1")
                    temp.indexIn1 = xml.readElementText().toInt();
                else if (xml.name() == "rangeIn0")
                    temp.rangeIn0 = xml.readElementText().toInt();
                else if (xml.name() == "rangeIn1")
                    temp.rangeIn1 = xml.readElementText().toInt();
                else if (xml.name() == "attack")
                    temp.attack = xml.readElementText().toInt();
                else if (xml.name() == "release")
                    temp.release = xml.readElementText().toInt();
                else if (xml.name() == "repeatMode")
                    temp.repeatMode = xml.readElementText().toInt();
                else if (xml.name() == "rndTick")
                    temp.rndTick = xml.readElementText().toInt();
                else if (xml.name() == "rndLen")
                    temp.rndLen = xml.readElementText().toInt();
                else if (xml.name() == "rndVel")
                    temp.rndVel = xml.readElementText().toInt();
                else if (xml.name() == "pattern")
                    temp.pattern = xml.readElementText();
                else if (xml.isStartElement() && (xml.name() == "muteMask")) {
                    while (!xml.atEnd()) {
                        xml.readNext();
                        if (xml.isEndElement())
                            break;
                        if (xml.isStartElement() && (xml.name() == "data")) {
                            temp.muteMask.clear();
                            QByteArray tmpArray =
                                    QByteArray::fromHex(xml.readElementText().toLatin1());
                            for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                                temp.muteMask.append(tmpArray.at(l1));
                            }
                        }
                        else skipXmlElement(xml);
                    }
                }
                else if (xml.isStartElement() && (xml.name() == "wave")) {
                    while (!xml.atEnd()) {
                        xml.readNext();
                        if (xml.isEndElement())
                            break;
                        if (xml.isStartElement() && (xml.name() == "data")) {
                            temp.wave.clear();
                            QByteArray tmpArray =
                                    QByteArray::fromHex(xml.readElementText().toLatin1());
                            int step = TPQN / temp.res;
                            int lt = 0;
                            Sample sample;
                            for (int l1 = 0; l1 < tmpArray.count(); l1++) {
                                sample.value = tmpArray.at(l1);
                                sample.tick = lt;
                                sample.muted = temp.muteMask.at(l1);
                                temp.wave.append(sample);
                                lt+=step;
                            }
                        }
                        else skipXmlElement(xml);
                    }
                }
                else skipXmlElement(xml);
            }
            tempToList(ix);
            ix++;
        }
    }
}
