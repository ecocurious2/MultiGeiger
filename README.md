
# MultiGeiger
Wir sind Ecocurious, Deine Umwelt-, Natur-  und Technik-Community. Aktuell bauen wir einen Multigeiger, um Radioaktivität zu messen. https://ecocurious.de/projekte/multigeiger/

Ziel ist es, einen Low-Cost-Geigerzähler (für Gamma-Strahlung) zu entwickeln und ein Bürger-Messnetz in Deutschland aufzubauen.

Unsere Hard- und Software hat Jürgen Böhringer konzipiert (http://www.boehri.de). Reinhard/rexfue hat die Software und Platine weitergedacht und kümmert sich um die Einbindung der Sensoren in unsere Map https://ecocurious.de/multigeiger-karte/.

Das Besondere an unserem Multigeiger ist, dass er wahlweise mit einem LoRa- oder Wifi-Modul betrieben werden kann. Wir haben die ersten Workshops gestartet, in denen Du die Hardware und das Gehäuse mit unserer Unterstützung zusammengebauen kannst.

Workshop-Termine findest Du hier: https://www.meetup.com/de-DE/Ecocurious-deine-Umwelt-Natur-und-Technik-Community/
und hier:https://ecocurious.de/events/

Klingt das interessant für Dich? Dann mach mit, herzliche Einladung!



## Installation
Hier in GitHub die neueste Release ( https://github.com/ecocurious/MultiGeiger/releases ) als Source code (zip) oder Source Code (tar.gz) herunterladen und entpacken.
Mit der Arduino-IDE in dem neuen Verzeichnis die Datei *multigeiger.ino* im Verzeichnis *multigeiger* öffnen.

Die Platine unterstützt zwei verschiedene Heltec-Bausteine, verschiedene Zählrohre und optional einen Temperatur/Luftdruck/Luftfeuchtigkeits-Sensor.
Die Software kann via Netzwerk Daten zu verschiedenen Services senden.

 * **Heltec WiFi Kit 32**
Diese MCU hat ein großes Display und WiFi.
Auf dem Board wird dieser Baustein in die längeren Buchsenleisten gesteckt.
Für die Arduino-IDE ist als Board der **Heltec WiFi Kit 32** einzustellen.

 * **Heltec Wireless Stick**
Diese MCU hat ein sehr kleines Display, dafür aber zusätzlich zu WiFi noch LoRa.
Es wird in die kürzeren Buchsenleisten gesteckt.
Für die Arduino-IDE muss als Board **Heltec Wireless Stick** eingestellt werden.

Um die Heltec-Boards in der Arduino IDE auswählen zu können, muss https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_dev_index.json in den Preferences unter Additional Boards Manager URLs hinzugefügt werden. Danach können unter Tools->Board->Boards Manager die Heltec Boards (Name "Heltec ESP32...")
installiert und anschließend unter Tools->Board ausgewählt werden.

Die Hard- und Software-Einstellungen muss man über diese Dateien machen (siehe Kommentare dort):

 * **./multigeiger/userdefines.h** (immer notwendig, ein Beispiel hierzu wird in userdefines-example.h mitgeliefert)
 * **./platformio.ini** (nur bei platformio, ein Beispiel hierzu wird in platformio-example.ini mitgeliefert)

Als externe Libraries werden benötigt:

 * U8g2 von Oliver, Version 2.26.14
 * Adafruit BME280 Library, Version 1.0.7
 * Adafruit Unified Sensor, Version 1.02
 * IotWebConf, Version 2.3.0

Für LoRa zusätzlich:
 * MCCI LoRaWAN LMIC library, Version >= 2.3.2  
**Achtung:** Bitte prüfen, dass in der Datei  project_config/lmic_project_config.h (in der obersten Ebene in dieser Library) unbedingt
die richtigen Configs eingestellt sind. Die Datei muss folgendermassen aussehen:
```
// project-specific definitions
#define CFG_eu868 1
//#define CFG_us915 1
//#define CFG_au921 1
//#define CFG_as923 1
// #define LMIC_COUNTRY_CODE LMIC_COUNTRY_CODE_JP	/* for as923-JP */
//#define CFG_in866 1
#define CFG_sx1276_radio 1
//#define LMIC_USE_INTERRUPTS
```

Falls der Compiler andere Libraries anmahnt, diese bitte in der Arduino IDE per *Sketch -> Include Library -> Manage Libraries ..* installieren.

## Ablauf nach dem Start
Das Gerät baut einen eigenen WLAN-Accesspoint (AP) auf. Die SSID des AP lautet **ESP32-xxxxxxxx**, wobei die xxx
die Chip-ID (bzw. die MAC-Adresse) des WLAN-Chips sind (Beispiel: **ESP32-51564452**).
**Bitte diese Nummer notieren, sie wird später noch gebraucht.**
Dieser Access-Point bleibt für 30sec aktiv. Danach versucht das Gerät, sich mit dem (früher) eingestellten WLAN
zu verbinden. Dieser Verbindungsversuch dauert ebenfalls 30sec. Kommt keine Verbindung zu Stande, wird wieder der
eigene AP für 30sec erzeugt. Wenn das WLAN nicht erreicht werden kann, läuft dieses Spiel endlos.
Solange keine Verbindung zum WLAN besteht, wird auf dem Display in der untersten Zeile ganz klein *connecting ...*
angezeigt. Diese Anzeige verschwindet, sobald eine WLAN-Verbindung hergestellt ist.

## Einstellung des WLAN
Wenn das Gerät den eigenen AP aufgebaut hat, verbindet man sich mit diesem. Entweder mit einem Handy oder einem PC o.ä..
Die Verbindung fragt nach einem Passwort, es lautet **ESP32Geiger**.
Ist die Verbindung mit dem Accesspoint hergestellt, hat man beliebig Zeit, die Daten einzugeben.
Es öffnet sich **automatisch** die Startseite des Gerätes. Es braucht also - in der Regel - nicht extra der Browser aufgerufen werden.
Falls die Startseite ausnahmsweise doch nicht erscheint, so muss mit dem Browser die Adresse **192.168.4.1** aufgerufen werden und nun erscheint die Startseite.
Dort findet man einen Link zur __configure page__ - dort drauf klicken und man kommt zur Einstellungsseite.


Diese hat die folgenden Zeilen:
 * Geiger accesspoint SSID  
 Dies ist die SSID des eingebauten APs und kann zwar geändert werden, sollte aber nicht! Der Sensor wird mit dieser Nummer bei sensor.community (früher: luftdaten.info) angemeldet. Wird sie geändert, muss eine neue Anmeldung erfolgen.
 * Geiger accesspoint password
 Dies ist das Passwort für den eingebauten AP. Dieses **MUSS** beim ersten Mal geändert werden. Es kann natürlich auch das gleiche Passwort wieder verwendet werden - wichtig ist nur, dass da was reingeschrieben wird und dass man das **nicht vergessen** darf.
 * WiFi client SSID  
 Hier muss die SSID des WLANs für den Netzwerk/Internet-Zugang eingegeben werden.
 * WiFi client password  
 Und hier das zugehörige Passwort.

Es wird empfohlen, beim WLAN das Gastnetz zu verwenden (falls ein solches existiert). Normalerweise wird das Gastnetz im Router vom normalen Netz abgeschottet und ist damit sicherer.

Ist alles eingegeben, kann man auf **Apply** drücken. Nun werden die eingestellten Daten übernommen und in das interne EEPROM gespeichert. Nun bitte **unbedingt** über **Abbrechen** diese Seite verlassen! Nur dann verlässt das Programm den Config-Mode und verbindet sich mit dem heimischen WLAN. Wenn es kein **Abbrechen** gibt, dann wieder zurück in die WLAN-Einstellungen des Gerätes gehen und da dann das normale Heim-Netzwerk wieder einstellen.

Auf der Einstellungsseite gibt es auch einen Link __Firmware update__ - hiermit kann man die Software auf dem MultiGeiger aktualisieren.
Man braucht dazu die zum Gerät passende .bin-Datei, wählt diese dann über **Browse...** aus und klickt zum Aktualisieren auf **Update**.
Danach dauert es ca. 30s für das Hochladen und Flashen der Datei.

Der Browser zeigt dann (hoffentlich) **Update Success! Rebooting...** an, der MultiGeiger startet dann neu und ab dann ist die neue
Firmware aktiv.

Erscheint **Update error: ...**, dann hat das Update nicht geklappt - es ist dann die seitherige Firmware weiter aktiv.

## Server
Es werden jeweils einen Messzyklus lang die Impulse gezählt und dann die "Counts per Minute" (cpm) berechnet.
Jeweils nach diesem Zyklus werden die Daten zu den Servern bei *sensor.community* und bei *madavi.de* gesendet.

Bei *sensor.community* werden die Daten gespeichert und stehen am nächsten Tag zum Abruf als CSV-Datei bereit:
http://archive.sensor.community/DATE/DATE_radiation_si22g_sensor_SID.csv
wobei DATE = Datum im Format YYYY-MM-DD ist (beides mal gleich) und SID die Sensornummer des Sensors (**nicht** die ChipID). Bei anderen Sensoren ist der Zählrohr-Name **si22g** durch den entsprechenden Namen zu ersetzen (z.B.: sbm-20 oder sbm-19) 

Bei *madavi* werden die Daten in einer RRD-Datenbank abgelegt und können direkt aktuell als Grafik über diesen Link betrachtet werden:
https://www.madavi.de/sensor/graph.php?sensor=esp32-CHIPID-si22g
Hier ist dann CHIPID die ChipId (also die Ziffern der SSID des internen Accesspoints).

Während der Übertragung der Daten zu den Servern wird in der Statuszeile (unterste Zeile) des Displays kurz der Name des Servers eingeblendet.

## Anmeldung bei sensor.community (luftdaten.info)
Damit die Daten, die der Sensor nach sensor.community schickt, von dem Server auch angenommen werden, muss man sich dort anmelden. Das geschieht über die Seite https://meine.luftdaten.info.
Zuerst über den *Registrieren*-Knopf einen Account anlegen. Dann damit über *Login* einloggen und *Neuen Sensor registrieren* anklicken.
Dann das Formular ausfüllen:
 * Erste Zeile, Sensor ID:
 Hier die Nummer (nur die Zahlen) der SSID des Sensors eingeben (z.B. bei dem Sensor ESP-51564452 also dann nur 51564452 eingeben)
 * Zweite Zeile, Sensor Board:
 Hier *esp32* auswählen (über die kleinen Pfeile rechts)
 * Basisinformation:
 Hier die Adresse eingeben (mit dem Land!). Der interne Name des Sensors kann beliebig vergeben werden, muss aber eingegeben werden. Bitte den Haken bei **Indoor-Sensor** setzen, so lange der Sensor wirklich innen ist.
 * Zusätzliche Informationen:
 Kann freigelassen werden, darf aber auch ausgefüllt werden.
 * Hardware-Konfiguration:
 Hier als Sensor-Typ den Eintrag **Radiation Si22G** (oder ggf. entsprechend) auswählen. Für den zweiten Sensor kann DHT22 stehen bleiben, das ist für uns irrelevant.
 * Position
 Hier bitte die Koordinaten eingeben, so genau wie möglich (oder über den rechten Knopf die Koordinaten rechnen lassen). Dies wird benötigt, um den Sensor später auf der Karte anzeigen zu können.

 Nun mit *Einstellungen speichern* das Ganze beenden. Dann auf der Übersichts-Seite bei diesem Sensor auf *Daten* klicken. Nun steht hinter *Sensor ID* die ID des Sensors. Diese bitte merken: sie wird für die Abfrage bei sensor.community bzw. bei der Anzeige auf https://multigeiger.citysensor.de benötigt (zur Zeit ist das noch laufend in Arbeit - kann/wird also ab und zu ausfallen :wink: ).

