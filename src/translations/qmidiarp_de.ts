<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="de">
<context>
    <name>ArpWidget</name>
    <message>
        <location filename="../arpwidget.cpp" line="70"/>
        <source>Input</source>
        <translation>Eingang</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="96"/>
        <location filename="../arpwidget.cpp" line="685"/>
        <source>Note Filter</source>
        <translation>Notenfilter</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="97"/>
        <source>&amp;Note</source>
        <translation>&amp;Note</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="160"/>
        <source>Output</source>
        <translation>Ausgang</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="250"/>
        <source>&amp;Mute</source>
        <translation>&amp;Stumm</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="171"/>
        <source>&amp;Port</source>
        <translation>Anschl&amp;uss</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="72"/>
        <source>&amp;Restart</source>
        <translation>&amp;Neustarten</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="76"/>
        <source>Restart pattern when a new note is received</source>
        <translation>Die Sequenz neu starten wenn eine neue Note empfangen wird</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="78"/>
        <source>&amp;Trigger</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="82"/>
        <source>Retrigger pattern when a new note is received</source>
        <translation>Die Sequenz mit dem timing der empfangenen Noten starten (triggern)</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="84"/>
        <source>&amp;Legato</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="88"/>
        <source>Retrigger / restart upon new legato note as well</source>
        <translation>Triggern / Neustarten auch beim Empfang von neuen Legato-Noten</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="187"/>
        <source>&amp;Show/hide in-out settings</source>
        <translation>&amp;Ein-/Ausgangspanele sichtbar/unsichtbar</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="209"/>
        <source>Pattern</source>
        <translation>Muster</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="214"/>
        <source>&amp;Edit Pattern</source>
        <translation>&amp;Bearbeiten</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="223"/>
        <source>&amp;Remove Pattern</source>
        <translation>&amp;Löschen</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="231"/>
        <source>&amp;Store Pattern</source>
        <translation>&amp;Speichern</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="243"/>
        <source>Pattern preset</source>
        <translation>Mustervorlage</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="261"/>
        <source>Defer mute to pattern end</source>
        <translation>Stummschaltung erst am Sequenzende anwenden</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="273"/>
        <source>Repeat mode - This is how notes are sequenced
when a chord is pressed</source>
        <translation>Wiederholungsmodus - So verhält sich das Arpeggio am Ende des Musters, wenn ein Akkord gespielt wird </translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="281"/>
        <source>Bounce</source>
        <translation>Hin-her</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="283"/>
        <source>Octave mode - The overall octave changes like this
once all pressed notes were played through</source>
        <translation>Oktaviermodus - So verändert sich die globale Oktave des Arpeggios wenn alle Noten durchgespielt wurden</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="293"/>
        <source>Octave range - the range above and below the played note
if Octave mode is not static</source>
        <translation>Oktav-Spanne - Die Anzahl der Oktaven oberhalb und unterhalb der gespielten Note, wenn der Oktaviermodus nicht statisch ist</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="750"/>
        <source>Could not read the pattern presets from the 
.qmidiarprc resource file. To create this file 
please just run the qmidiarp main application once.</source>
        <translation>Konnte die Arp-Muster presets nicht von der .qmidiarprc Datei lesen. Um diese Datei zu erzeugen, bitte die qmidiarp Anwendung einmal starten.</translation>
    </message>
    <message>
        <source>No trigger</source>
        <translation type="obsolete">Durchspielen</translation>
    </message>
    <message>
        <source>Kbd restart</source>
        <translation type="obsolete">Neustart</translation>
    </message>
    <message>
        <source>Kbd restart legato</source>
        <translation type="obsolete">Neustart Legato</translation>
    </message>
    <message>
        <source>Kbd trigger legato</source>
        <translation type="obsolete">Loslaufen Legato</translation>
    </message>
    <message>
        <source>Trigger Mode</source>
        <translation type="obsolete">Verhalten bei neuer Stakato-Note</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="301"/>
        <source>&amp;Latch Mode</source>
        <translation>&amp;Noten ha&amp;lten</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="339"/>
        <source>0..9  note played on keyboard, 0 is lowest
( ) numbers in parenthesis are stacked to chords
  + = -  octave up/reset/down
  t = g  semitone up/reset/down
 &lt; . &gt; tempo up/reset/down
  d h  note length up/down
  / \  velocity up/down
   p   pause</source>
        <translation>0..9 gespielte Note in aufsteigender Anordnung
( ) Noten zwischen Klammern werden als Akkord interpretiert
 + = - Octave auf/Normalwert/ab
 t = g  Halbton auf/Normalwert/ab
 &lt; . &gt; Tempo auf/ab
 d h Notenlänge auf/ab
 / \ Geschwindigkeit auf/ab
 p Pause</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="405"/>
        <source>&amp;Attack (beats)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="409"/>
        <source>&amp;Release (beats)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="681"/>
        <source>Note Filter - ACTIVE</source>
        <translation>Notenfilter - AKTIV</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="271"/>
        <location filename="../arpwidget.cpp" line="369"/>
        <location filename="../arpwidget.cpp" line="838"/>
        <source>Random</source>
        <translation>Zufall</translation>
    </message>
    <message>
        <source>Repeat mode</source>
        <translation type="obsolete">Wiederholungsmodus</translation>
    </message>
    <message>
        <source>Kbd trigger</source>
        <translation type="obsolete">Loslaufen</translation>
    </message>
    <message>
        <source>0..9  note played on keyboard, 0 is lowest
( ) numbers in parenthesis are stacked to chords
  + = -  octave up/reset/down
 &lt; . &gt; tempo up/reset/down
  d h  note length up/down
  / \  velocity up/down
   p   pause</source>
        <translation type="obsolete">0..9 gespielte Note in aufsteigender Anordnung
( ) Noten zwischen Klammern werden als Akkord interpretiert
 + = - Octave auf/Normalwert/ab
 &lt; . &gt; Tempo auf/ab
 d h Notenlänge auf/ab
 / \ Geschwindigkeit auf/ab
 p Pause</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="373"/>
        <source>&amp;Shift</source>
        <translation>&amp;Verschiebung</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="378"/>
        <source>Vel&amp;ocity</source>
        <translation>Ans&amp;chlag</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="383"/>
        <source>&amp;Length</source>
        <translation>&amp;Dauer</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="402"/>
        <location filename="../arpwidget.cpp" line="864"/>
        <source>Envelope</source>
        <translation>Hüllkurve</translation>
    </message>
    <message>
        <source>Could not read from resource file</source>
        <translation type="obsolete">Konnte Ressourcendatei nicht einlesen</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="834"/>
        <source>Random - ACTIVE</source>
        <translation>Zufall - AKTIV</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="860"/>
        <source>Envelope - ACTIVE</source>
        <translation>Hüllkurve - AKTIV</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="878"/>
        <source>%1: Store pattern</source>
        <translation>%1: Muster speichern</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="879"/>
        <source>New pattern</source>
        <translation>Neues Muster</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="879"/>
        <source>Arp pattern</source>
        <translation>Arp-Muster</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="915"/>
        <source>Remove &quot;%1&quot;?</source>
        <translation>&quot;%1&quot; löschen?</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="90"/>
        <source>&amp;Channel</source>
        <translation>&amp;Kanal</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="111"/>
        <source>&amp;Velocity</source>
        <translation>&amp;Anschlag</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="162"/>
        <source>C&amp;hannel</source>
        <translation>Kana&amp;l</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="271"/>
        <location filename="../arpwidget.cpp" line="281"/>
        <source>Static</source>
        <translation>Statisch</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="271"/>
        <location filename="../arpwidget.cpp" line="281"/>
        <source>Up</source>
        <translation>Aufwärts</translation>
    </message>
    <message>
        <location filename="../arpwidget.cpp" line="271"/>
        <location filename="../arpwidget.cpp" line="281"/>
        <source>Down</source>
        <translation>Abwärts</translation>
    </message>
</context>
<context>
    <name>GlobStore</name>
    <message>
        <location filename="../globstore.cpp" line="50"/>
        <source>End of</source>
        <translation>Ende von</translation>
    </message>
    <message>
        <location filename="../globstore.cpp" line="51"/>
        <source>After</source>
        <translation>Nach</translation>
    </message>
    <message>
        <location filename="../globstore.cpp" line="90"/>
        <source>&amp;Remove</source>
        <translation>&amp;Entfernen</translation>
    </message>
    <message>
        <location filename="../globstore.cpp" line="159"/>
        <source>&amp;Store</source>
        <translation>&amp;Speichern</translation>
    </message>
    <message>
        <location filename="../globstore.cpp" line="167"/>
        <source>&amp;Restore</source>
        <translation>&amp;Aufrufen</translation>
    </message>
</context>
<context>
    <name>GrooveWidget</name>
    <message>
        <location filename="../groovewidget.cpp" line="48"/>
        <source>&amp;Shift</source>
        <translation>&amp;Verschiebung</translation>
    </message>
    <message>
        <location filename="../groovewidget.cpp" line="54"/>
        <source>&amp;Velocity</source>
        <translation>&amp;Anschlag</translation>
    </message>
    <message>
        <location filename="../groovewidget.cpp" line="60"/>
        <source>&amp;Length</source>
        <translation>&amp;Dauer</translation>
    </message>
</context>
<context>
    <name>LfoWidget</name>
    <message>
        <location filename="../lfowidget.cpp" line="136"/>
        <source>Output</source>
        <translation>Ausgang</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="301"/>
        <source>&amp;Mute</source>
        <translation>&amp;Stumm</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="97"/>
        <location filename="../lfowidget.cpp" line="138"/>
        <source>MIDI &amp;CC#</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="144"/>
        <source>MIDI Controller number sent to output</source>
        <translation>Nummer des gesendeten MIDI Controllers</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="149"/>
        <source>C&amp;hannel</source>
        <translation>&amp;Kanal</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="161"/>
        <source>&amp;Port</source>
        <translation>An&amp;schluss</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="66"/>
        <source>Input</source>
        <translation>Eingang</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="86"/>
        <source>&amp;Legato</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="90"/>
        <source>Retrigger / restart upon new legato note as well</source>
        <translation>Triggern / Neustarten auch beim Empfang von neuen Legato-Noten</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="103"/>
        <source>MIDI Controller number to record</source>
        <translation>Die aufzunehmende MIDI Controller ID</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="108"/>
        <source>&amp;Channel</source>
        <translation>&amp;Kanal</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="181"/>
        <source>&amp;Show/hide in-out settings</source>
        <translation>&amp;Ein-/Ausgangspanele sichtbar/unsichtbar</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="203"/>
        <source>Wave</source>
        <translation>Welle</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="229"/>
        <source>Waveform Basis</source>
        <translation>Wellenformbasis</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="207"/>
        <source>Right button to mute points
Left button to draw custom wave
Wheel to change offset</source>
        <translation>Rechte Maustaste: Stummschalten einzelner Punkte
Linke Maustaste: Zeichnen der Wellenform
Mausrad: Verschieben des Offsets</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="68"/>
        <source>&amp;Note Off</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="72"/>
        <source>Stop output when Note is released</source>
        <translation>Keine Noten senden wenn die Taste losgelassen wird</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="74"/>
        <source>&amp;Restart</source>
        <translation>&amp;Neustarten</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="78"/>
        <source>Restart sequence when a new note is received</source>
        <translation>Die Sequenz neu starten wenn eine neue Note empfangen wird</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="80"/>
        <source>&amp;Trigger</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="84"/>
        <source>Retrigger sequence when a new note is received</source>
        <translation>Die Sequenz mit dem timing der empfangenen Noten starten (triggern)</translation>
    </message>
    <message>
        <source>&amp;Loop</source>
        <translation type="obsolete">&amp;Schleife</translation>
    </message>
    <message>
        <source>Play sequence as loop instead of a single run</source>
        <translation type="obsolete">Die Sequenz ständig wiederholen anstatt sie nur einmal zu spielen</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="237"/>
        <source>&amp;Frequency</source>
        <translation>&amp;Frequenz</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="249"/>
        <source>Frequency (cycles/beat): Number of wave cycles produced every beat</source>
        <translation>Frequenz (Zyklen pro beat): Anzahl der erzeugten Wellenzyklen pro Vierteltakt</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="256"/>
        <source>&amp;Resolution</source>
        <translation>&amp;Auflösung</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="266"/>
        <source>Resolution (events/beat): Number of events produced every beat</source>
        <translation>Auflösung (Signale/beat): Anzahl der pro Vierteltakt erzeugten MIDI Signale</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="273"/>
        <source>&amp;Length</source>
        <translation>&amp;Dauer</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="282"/>
        <source>Length of LFO wave in beats</source>
        <translation>Länge der LFO Wellenform in Vierteltakten</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="294"/>
        <source>Loop, bounce or play once going forward or backward</source>
        <translation>Wiederholen, hin- und zurück oder nur einmal spielen, vorwärts oder rückwärts</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="312"/>
        <source>Defer mute to pattern end</source>
        <translation>Stummschaltung erst am Sequenzende anwenden</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="320"/>
        <location filename="../lfowidget.cpp" line="321"/>
        <source>Re&amp;cord</source>
        <translation>&amp;Aufnahme</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="322"/>
        <source>Record incoming controller</source>
        <translation>Eingehenden MIDI controller aufzeichnen</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="332"/>
        <source>&amp;Amplitude</source>
        <translation>Am&amp;plitude</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="340"/>
        <source>&amp;Offset</source>
        <translation>&amp;Offset</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="647"/>
        <source>Sine</source>
        <translation>Sinus</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="647"/>
        <source>Saw up</source>
        <translation>Sägezahn auf</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="647"/>
        <source>Triangle</source>
        <translation>Dreieck</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="648"/>
        <source>Saw down</source>
        <translation>Sägezahn ab</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="648"/>
        <source>Square</source>
        <translation>Rechteck</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="648"/>
        <source>Custom</source>
        <translation>Frei</translation>
    </message>
    <message>
        <location filename="../lfowidget.cpp" line="217"/>
        <source>&amp;Waveform</source>
        <translation>&amp;Wellenform</translation>
    </message>
</context>
<context>
    <name>LogWidget</name>
    <message>
        <location filename="../logwidget.cpp" line="45"/>
        <source>&amp;Enable Log</source>
        <translation>&amp;Protokollieren</translation>
    </message>
    <message>
        <location filename="../logwidget.cpp" line="51"/>
        <source>Log &amp;MIDI Clock</source>
        <translation>&amp;MIDI Clock aufzeichnen</translation>
    </message>
    <message>
        <location filename="../logwidget.cpp" line="56"/>
        <source>&amp;Clear</source>
        <translation>&amp;Löschen</translation>
    </message>
    <message>
        <location filename="../logwidget.cpp" line="109"/>
        <source>MIDI Clock, tick</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../logwidget.cpp" line="114"/>
        <source>MIDI Start (Transport)</source>
        <translation>MIDI Clock Start (Transport)</translation>
    </message>
    <message>
        <location filename="../logwidget.cpp" line="118"/>
        <source>MIDI Continue (Transport)</source>
        <translation>MIDI Continue (Transport)</translation>
    </message>
    <message>
        <location filename="../logwidget.cpp" line="122"/>
        <source>MIDI Stop (Transport)</source>
        <translation>MIDI Stop (Transport)</translation>
    </message>
    <message>
        <location filename="../logwidget.cpp" line="126"/>
        <source>Unknown event type</source>
        <translation>Unbekanntes MIDI-Signal</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="../mainwindow.cpp" line="159"/>
        <source>Event Log</source>
        <translation>Protokoll</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="91"/>
        <source>Groove</source>
        <translation>Groove</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="100"/>
        <source>Global Store</source>
        <translation>Globaler Speicher</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="183"/>
        <source>&amp;New LFO...</source>
        <translation>&amp;Neuer LFO...</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="188"/>
        <source>&amp;New Sequencer...</source>
        <translation>&amp;Neuer Sequenzer...</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="189"/>
        <source>Ctrl+T</source>
        <comment>Module|New Sequencer</comment>
        <translation>Strg+T</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="190"/>
        <source>Add new Sequencer to tab bar</source>
        <translation>Neuen Sequenzer hinzufügen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="194"/>
        <source>&amp;New</source>
        <translation>&amp;Neu</translation>
    </message>
    <message>
        <source>Create new arpeggiator file</source>
        <translation type="obsolete">Neue Arpeggiator-Zusammenstellung erzeugen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="199"/>
        <source>&amp;Open...</source>
        <translation>&amp;Öffnen...</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="204"/>
        <source>&amp;Save</source>
        <translation>&amp;Speichern</translation>
    </message>
    <message>
        <source>Save current arpeggiator file</source>
        <translation type="obsolete">Aktuelle Arpeggiator-Zusammenstellung speichern</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="210"/>
        <source>Save &amp;as...</source>
        <translation>Speichern &amp;unter...</translation>
    </message>
    <message>
        <source>Save current arpeggiator file with new name</source>
        <translation type="obsolete">Aktuelle Arpeggiator-Zusammenstellung unter neuem Namen speichern</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="217"/>
        <source>&amp;Quit</source>
        <translation>&amp;Beenden</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="218"/>
        <source>Ctrl+Q</source>
        <comment>File|Quit</comment>
        <translation>Strg+Q</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="219"/>
        <source>Quit application</source>
        <translation>Die Anwendung beenden</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="222"/>
        <source>&amp;Run with internal clock</source>
        <translation>&amp;Start/Stop mit interner Uhr</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="247"/>
        <source>&amp;Connect to Jack Transport</source>
        <translation>Mit &amp;Jack Transport verbinden</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="256"/>
        <source>&amp;Show all IO panels</source>
        <translation>E/A Panele &amp;sichtbar</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="261"/>
        <source>&amp;Hide all IO panels</source>
        <translation>E/A Panele &amp;unsichtbar</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="269"/>
        <source>Ctrl+H</source>
        <comment>View|Event Log</comment>
        <translation>Strg+H</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="273"/>
        <source>&amp;Groove Settings</source>
        <translation>&amp;Groove</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="274"/>
        <source>Ctrl+G</source>
        <comment>View|Groove</comment>
        <translation>Strg+G</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="284"/>
        <source>&amp;Global Store</source>
        <translation>G&amp;lobaler Speicher</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="285"/>
        <source>Ctrl+$</source>
        <comment>View|GlobalStore</comment>
        <translation>Strg+$</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="291"/>
        <source>Mod&amp;ule</source>
        <translation>Mod&amp;ul</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="297"/>
        <source>&amp;Recently opened files</source>
        <translation>&amp;Zuletzt geöffnete Dateien</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="313"/>
        <source>&amp;MIDI Controllers...</source>
        <translation>&amp;MIDI-Steuerungen...</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="315"/>
        <source>Ctrl+M</source>
        <comment>View|MidiControllers</comment>
        <translation></translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="328"/>
        <source>&amp;File Toolbar</source>
        <translation>&amp;Dateien Werkzeugleiste</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="338"/>
        <source>&amp;Control Toolbar</source>
        <translation>&amp;Steuerungsleiste</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="387"/>
        <source>Clear</source>
        <translation>Leeren</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="433"/>
        <location filename="../mainwindow.cpp" line="446"/>
        <location filename="../mainwindow.cpp" line="459"/>
        <source>%1</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="445"/>
        <source>Add MIDI LFO</source>
        <translation>MIDI LFO hinzufügen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="458"/>
        <source>Add Step Sequencer</source>
        <translation>Sequenzer hinzufügen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="758"/>
        <source>QMidiArp XML files</source>
        <translation>QMidiArp XML Dateien</translation>
    </message>
    <message>
        <source>Old QMidiArp files</source>
        <translation type="obsolete">Alte QMidiArp Textdateien</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="782"/>
        <source>Could not read from file &apos;%1&apos;.</source>
        <translation>Konnte die Datei &apos;%1&apos; nicht lesen.</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="803"/>
        <source>This is not a valid xml file for </source>
        <translation>Dies ist keine gültige xml Datei für </translation>
    </message>
    <message>
        <source>The QMidiArp text file was imported. If you save this file, it will be saved using the newer xml format under the name
 &apos;%1&apos;.</source>
        <translation type="obsolete">Die QMidiArp Textdatei wurde importiert. Beim nächsten Speichern wird diese im neueren xml Format gespeichert unter dem Namen
 &apos;%1&apos;.</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="959"/>
        <source>Could not write to file &apos;%1&apos;.</source>
        <translation>Konnte in die Datei &apos;%1&apos; nicht schreiben.</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="1067"/>
        <source>Unnamed file was changed.
Save changes?</source>
        <translation>Die unbenannte Datei wurde verändert.
Die Änderungen speichern?</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="1069"/>
        <source>File &apos;%1&apos; was changed.
Save changes?</source>
        <translation>Die Datei &apos;%1&apos; wurde verändert.
Die Änderungen speichern?</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="1073"/>
        <source>Save changes</source>
        <translation>Änderungen speichern</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="1236"/>
        <source>Could not read from resource file</source>
        <translation>Konnte Ressourcendatei nicht einlesen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="238"/>
        <source>&amp;Use incoming MIDI Clock</source>
        <translation>&amp;Eingehendes MIDI Clock Signal nutzen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="196"/>
        <source>Create new QMidiArp session</source>
        <translation>Neue QMidiArp Session erzeugen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="201"/>
        <source>Open QMidiArp file</source>
        <translation>QMidiArp-Datei öffnen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="206"/>
        <source>Save current QMidiArp session</source>
        <translation>Aktuelle Session speichern</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="213"/>
        <source>Save current QMidiArp session with new name</source>
        <translation>Aktuelle Session unter neuem Namen speichern</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="268"/>
        <source>&amp;Event Log</source>
        <translation>&amp;Protokoll</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="276"/>
        <source>&amp;Settings</source>
        <translation>&amp;Einstellungen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="278"/>
        <source>Ctrl+P</source>
        <comment>View|Settings</comment>
        <translation>Strg+P</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="325"/>
        <source>&amp;About Qt...</source>
        <translation>&amp;Über Qt...</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="388"/>
        <source>Clear QMidiArp session</source>
        <translation>QMidiArp Session leeren</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="389"/>
        <source>Import file...</source>
        <translation>Datei importieren...</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="390"/>
        <source>Import QMidiArp file to NSM session</source>
        <translation>QMidiArp Datei in die NSM Session importieren</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="391"/>
        <source>Export session...</source>
        <translation>Session exportieren...</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="392"/>
        <source>Export QMidiArp NSM session to file</source>
        <translation>Die QMidiArp NSM Session in eine Datei exportieren</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="418"/>
        <source>About %1</source>
        <translation>Über %1</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="423"/>
        <source>About Qt</source>
        <translation>Über Qt</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="432"/>
        <source>Add MIDI Arpeggiator</source>
        <translation>MIDI-Arpeggiator hinzufügen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="757"/>
        <source>Open arpeggiator file</source>
        <translation>Arpeggiator-Datei öffnen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="232"/>
        <source>Tempo of internal clock</source>
        <translation>Tempo der internen Uhr</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="801"/>
        <source>Not a QMidiArp xml file.</source>
        <translation>Keine QMidiArp xml Datei.</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="1045"/>
        <source>Save arpeggiator</source>
        <translation>Arpeggiator Speichern</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="1045"/>
        <source>QMidiArp files</source>
        <translation>QMidiArp Dateien</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="1170"/>
        <source>JACK has shut down or could not be started, but you are trying
to run QMidiArp with JACK MIDI backend.

Alternatively you can use the ALSA MIDI backend 
by calling qmidiarp -a</source>
        <translation>JACK ist nicht oder nicht mehr verfügbar, aber QMidiArp wurde mit JACK MIDI Treiber aufgerufen.

Wenn gewünscht kann QMidiArp auch mit ALSA Treiber verwendet werden (Starten mit qmidiarp -a)</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="1281"/>
        <source>Could not write to resource file</source>
        <translation>Konnte Ressourcendatei nicht speichern</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="178"/>
        <source>&amp;New Arp...</source>
        <translation>&amp;Neuer Arp...</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="180"/>
        <source>Add new arpeggiator to tab bar</source>
        <translation>Neuen Arpeggiator hinzufügen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="185"/>
        <source>Add new LFO to tab bar</source>
        <translation>Neuen LFO hinzufügen</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="179"/>
        <source>Ctrl+A</source>
        <comment>Module|New Arp</comment>
        <translation>Strg+A</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="184"/>
        <source>Ctrl+L</source>
        <comment>Module|New LFO</comment>
        <translation>Strg+L</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="289"/>
        <source>&amp;File</source>
        <translation>&amp;Datei</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="290"/>
        <source>&amp;View</source>
        <translation>&amp;Ansicht</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="292"/>
        <source>&amp;Help</source>
        <translation>&amp;Hilfe</translation>
    </message>
    <message>
        <location filename="../mainwindow.cpp" line="323"/>
        <source>&amp;About %1...</source>
        <translation>&amp;Über %1...</translation>
    </message>
</context>
<context>
    <name>ManageBox</name>
    <message>
        <location filename="../managebox.cpp" line="53"/>
        <location filename="../managebox.cpp" line="56"/>
        <source>&amp;Clone...</source>
        <translation>&amp;Duplizieren...</translation>
    </message>
    <message>
        <location filename="../managebox.cpp" line="58"/>
        <source>Duplicate this Module in muted state</source>
        <translation>Dieses Modul in stummem Zustand duplizieren</translation>
    </message>
    <message>
        <location filename="../managebox.cpp" line="63"/>
        <source>&amp;Rename...</source>
        <translation>&amp;Umbenennen...</translation>
    </message>
    <message>
        <location filename="../managebox.cpp" line="64"/>
        <source>Rename this Module</source>
        <translation>Dieses Modul umbenennen</translation>
    </message>
    <message>
        <location filename="../managebox.cpp" line="69"/>
        <source>&amp;Delete...</source>
        <translation>&amp;Löschen...</translation>
    </message>
    <message>
        <location filename="../managebox.cpp" line="70"/>
        <source>Delete this Module</source>
        <translation>Dieses Modul löschen</translation>
    </message>
    <message>
        <location filename="../managebox.cpp" line="90"/>
        <source>Delete &quot;%1&quot;?</source>
        <translation>&quot;%1&quot; löschen?</translation>
    </message>
    <message>
        <location filename="../managebox.cpp" line="109"/>
        <source>New Name</source>
        <translation>Neuer Name</translation>
    </message>
</context>
<context>
    <name>MidiCCTable</name>
    <message>
        <location filename="../midicctable.cpp" line="48"/>
        <source>Re&amp;move</source>
        <translation>Ent&amp;fernen</translation>
    </message>
    <message>
        <location filename="../midicctable.cpp" line="49"/>
        <source>Re&amp;vert</source>
        <translation>&amp;Wiederherstellen</translation>
    </message>
    <message>
        <location filename="../midicctable.cpp" line="62"/>
        <source>MIDI Controllers - </source>
        <translation>MIDI-Steuerungen - </translation>
    </message>
    <message>
        <location filename="../midicctable.cpp" line="150"/>
        <source>Control</source>
        <translation>Steuerung</translation>
    </message>
    <message>
        <location filename="../midicctable.cpp" line="154"/>
        <source>CC#</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../midicctable.cpp" line="158"/>
        <source>Ch</source>
        <translation>Kan</translation>
    </message>
    <message>
        <location filename="../midicctable.cpp" line="162"/>
        <source>min</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../midicctable.cpp" line="166"/>
        <source>max</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>MidiControl</name>
    <message>
        <location filename="../midicontrol.cpp" line="45"/>
        <source>Cancel MIDI &amp;Learning</source>
        <translation>MIDI-Lernen &amp;Abbrechen</translation>
    </message>
    <message>
        <location filename="../midicontrol.cpp" line="150"/>
        <source>MIDI &amp;Learn</source>
        <translation>&amp;Lerne von MIDI</translation>
    </message>
    <message>
        <location filename="../midicontrol.cpp" line="155"/>
        <source>MIDI &amp;Forget</source>
        <translation>MIDI-Steuerungen &amp;vergessen</translation>
    </message>
</context>
<context>
    <name>ParStore</name>
    <message>
        <location filename="../parstore.cpp" line="131"/>
        <source>&amp;Store here</source>
        <translation>&amp;Hier speichern</translation>
    </message>
    <message>
        <location filename="../parstore.cpp" line="137"/>
        <source>&amp;Act on pattern only</source>
        <translation>&amp;Nur die Sequenz ändern</translation>
    </message>
    <message>
        <location filename="../parstore.cpp" line="143"/>
        <source>When finished</source>
        <translation>Am Ende</translation>
    </message>
    <message>
        <location filename="../parstore.cpp" line="149"/>
        <source>Stay here</source>
        <translation>Hier bleiben</translation>
    </message>
    <message>
        <location filename="../parstore.cpp" line="155"/>
        <source>Jump back</source>
        <translation>Zurückspringen</translation>
    </message>
    <message>
        <location filename="../parstore.cpp" line="160"/>
        <source>Jump to:</source>
        <translation>Springen auf:</translation>
    </message>
</context>
<context>
    <name>PassWidget</name>
    <message>
        <location filename="../passwidget.cpp" line="40"/>
        <source>&amp;Forward unmatched events to port</source>
        <translation>&amp;Unpassende MIDI Signale weiterleiten an Anschluss</translation>
    </message>
    <message>
        <location filename="../passwidget.cpp" line="63"/>
        <source>&amp;Compact module layout style</source>
        <translation>&amp;Kompakter Layout-Stil für Module</translation>
    </message>
    <message>
        <location filename="../passwidget.cpp" line="57"/>
        <source>&amp;Modules controllable by MIDI controller</source>
        <translation>&amp;Module sind MIDI-steuerbar</translation>
    </message>
    <message>
        <location filename="../passwidget.cpp" line="69"/>
        <source>&amp;Add new modules in muted state</source>
        <translation>&amp;Neue Module stummgeschaltet hinzufügen</translation>
    </message>
    <message>
        <location filename="../passwidget.cpp" line="89"/>
        <source>Settings - </source>
        <translation>Einstellungen - </translation>
    </message>
</context>
<context>
    <name>SeqWidget</name>
    <message>
        <location filename="../seqwidget.cpp" line="66"/>
        <source>Transpose the sequence following incoming notes</source>
        <translation>Die Sequenz durch eingehende Note transponieren</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="136"/>
        <source>Output</source>
        <translation>Ausgang</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="202"/>
        <source>&amp;Mute</source>
        <translation>&amp;Stumm</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="288"/>
        <source>Display</source>
        <translation>Ansicht</translation>
    </message>
    <message>
        <source>&amp;Full</source>
        <translation type="obsolete">&amp;Voll</translation>
    </message>
    <message>
        <source>&amp;Upper</source>
        <translation type="obsolete">&amp;Oben</translation>
    </message>
    <message>
        <source>&amp;Mid</source>
        <translation type="obsolete">&amp;Mitte</translation>
    </message>
    <message>
        <source>&amp;Lower</source>
        <translation type="obsolete">&amp;Unten</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="68"/>
        <source>&amp;Note Off</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="72"/>
        <source>Stop output when Note is released</source>
        <translation>Keine Noten senden wenn die Taste losgelassen wird</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="80"/>
        <source>&amp;Restart</source>
        <translation>&amp;Neustarten</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="84"/>
        <source>Restart sequence when a new note is received</source>
        <translation>Die Sequenz neu starten wenn eine neue Note empfangen wird</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="86"/>
        <source>&amp;Trigger</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="90"/>
        <source>Retrigger sequence when a new note is received</source>
        <translation>Die Sequenz mit dem timing der empfangenen Noten starten (triggern)</translation>
    </message>
    <message>
        <source>&amp;Loop</source>
        <translation type="obsolete">&amp;Schleife</translation>
    </message>
    <message>
        <source>Play sequence as loop instead of a single run</source>
        <translation type="obsolete">Die Sequenz ständig wiederholen anstatt sie nur einmal zu spielen</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="191"/>
        <source>Sequence</source>
        <translation>Sequenz</translation>
    </message>
    <message>
        <source>&amp;Sequence</source>
        <translation type="obsolete">&amp;Sequenz</translation>
    </message>
    <message>
        <source>Preset Number</source>
        <translation type="obsolete">Preset Index</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="235"/>
        <location filename="../seqwidget.cpp" line="236"/>
        <source>Re&amp;cord</source>
        <translation>&amp;Aufnahme</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="237"/>
        <source>Record step by step</source>
        <translation>Eingehende Noten Schritt für Schritt aufnehmen</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="257"/>
        <source>Resolution (notes/beat): Number of notes produced every beat</source>
        <translation>Auflösung (Noten/Beat): Zahl der in jedem Vierteltakt gespielten Noten</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="273"/>
        <source>Length of Sequence in beats</source>
        <translation>Länge der Sequenz in Vierteltakten</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="309"/>
        <source>Veloc&amp;ity</source>
        <translation>An&amp;schlag</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="317"/>
        <source>N&amp;ote Length</source>
        <translation>N&amp;otenlänge</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="74"/>
        <source>&amp;Velocity</source>
        <translation>&amp;Anschlag</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="283"/>
        <source>&amp;F</source>
        <translation>&amp;V</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="283"/>
        <source>&amp;U</source>
        <translation>&amp;O</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="283"/>
        <source>&amp;M</source>
        <translation>&amp;M</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="283"/>
        <source>&amp;L</source>
        <translation>&amp;U</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="284"/>
        <source>Full</source>
        <translation>Voll</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="284"/>
        <source>Upper</source>
        <translation>Oben</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="284"/>
        <source>Mid</source>
        <translation>Mitte</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="284"/>
        <source>Lower</source>
        <translation>Unten</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="60"/>
        <source>Input</source>
        <translation>Eingang</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="62"/>
        <source>&amp;Note</source>
        <translation>&amp;Note</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="78"/>
        <source>Set sequence velocity to that of incoming notes</source>
        <translation>Anschlag der Sequenz folgt dem der eingehende Noten </translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="92"/>
        <source>&amp;Legato</source>
        <translation></translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="96"/>
        <source>Retrigger / restart upon new legato note as well</source>
        <translation>Triggern / Neustarten auch beim Empfang von neuen Legato-Noten</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="105"/>
        <source>&amp;Channel</source>
        <translation>&amp;Kanal</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="170"/>
        <source>&amp;Show/hide in-out settings</source>
        <translation>&amp;Ein-/Ausgangspanele sichtbar/unsichtbar</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="214"/>
        <source>Defer mute, velocity, note length and transpose to pattern end</source>
        <translation>Stummschaltung, Anschlag, Notenlänge und Transposition erst am Sequenzende anwenden</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="228"/>
        <source>Loop, bounce or play once going forward or backward</source>
        <translation>Wiederholen, hin- und zurück oder nur einmal spielen, vorwärts oder rückwärts</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="325"/>
        <source>&amp;Transpose</source>
        <translation>&amp;Transponieren</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="148"/>
        <source>&amp;Port</source>
        <translation>Anschl&amp;uß</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="138"/>
        <source>C&amp;hannel</source>
        <translation>Ka&amp;nal</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="195"/>
        <source>Right button to mute points, left button to draw custom wave</source>
        <translation>Rechte Maustaste: Stummschalten einzelner Punkte, linke Maustaste: Zeichnen der Wellenform</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="247"/>
        <source>&amp;Resolution</source>
        <translation>&amp;Auflösung</translation>
    </message>
    <message>
        <location filename="../seqwidget.cpp" line="265"/>
        <source>&amp;Length</source>
        <translation>&amp;Dauer</translation>
    </message>
    <message>
        <source>Custom</source>
        <translation type="obsolete">Frei</translation>
    </message>
</context>
</TS>
