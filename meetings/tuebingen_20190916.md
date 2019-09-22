#Geiger-Meeting   2019-09-16

##Vorbereitung
###Vorstellung neue Platine (Rev 1.3)
 * Funktion mit Heltec WiFi Kit 32 und Heltec Wireless Stick ist OK
 * Probleme
 	* neue Heltec Wifi 32 haben bei den Stromversorgungs-PINs anderes Pinning (siehe Bilder)
 	* DIP-Schalter benötigt PullUPs
 	* I2C-Anschluss geht so nicht, muss umgelegt werden - PIN12 **muss** beim Booten auf **low** liegen. I2C-Platinen haben da aber einen PullUP dran!  
 	 Kann auf 4/5 (I2C vom Display geändert werden)
 	
###Neue embedded Software  (V1.6)
 * Auslesen des BME280 dabei
 * BME280 wird automatisch erkannt
 * LoRa Funktion (bei LoRa fähiger CPU, muss gesondert übersetzt werden)
 * Display auf Wireless Stick funktioniert
 * Senden der Daten zu Luftdaten und zu Madavi (selektierbar bei der Kompilation)
 * Ebenso Senden zu TTN einstellbar 
 
 
###Fragen
#####Hardware
  * Heltec Lora 32 mit großem Display vorsehen oder reicht der Wireless Stick
  * Dip-Switch:
   * nötig?
   * wenn ja, dann -> können die PullUPs als SMD 0805 gebaut werden?  Sollte man schon von Hand löten können

   
#####Web-Software
 * 7-Tage-Darstellung nun als gleitender Mittelwert **und** als fester (statischer). Was soll weiter drin bleiben?
 * Welche Mittelwert - Zeiten verwenden?
 * 30 Tage: Tagesmittelwerte  OK?

#####Embedded Software
 * 400V mit 100MOhm belasten: bricht ein und bleibt dann stehen, d.h. lädt nicht mehr
 * Display-Routinen scheinen den INT zu sperren (muss noch weiter getestet werden!)
 
 
##Beschlüsse beim Metting
##### Layout
 * DIP-Schalter
 	* default nicht bestücken
 	* Kurzschlussleitungen ins Layout -> 0 == default Wert
 * 	Großes (dickes) Zählrohr:
   * Sicherungshalter ganz an den Rand setzen (Begrenzung im Layout überschreiben)
   * **Nur** den Wifi Kit 32  und den Wirelss Stick vorsehen
  	
#####Software
 * Vitaldaten: (rxf)
 	* Temperatur im Gehäuse: dazu den Sensor auf dem ESP32 nutzen
 	* Anzahl der Pulse für die HV-Erzeugung sammeln und mit senden (Anzahl alls 60sec oder so)
 * die 400V brechen mit 100M=hm Last wirklich zusammen. Embedded Software muss noch geändert werden, so dass alle 1sec die HV nachgepulst wird, unabh. ob ein Geigerpuls da ist oder nicht (jb).
 * Test der Displayroutinen muss noch erfolgen (wegen den INT) (rxf).
 
#####Gehäuse
 * Verschieden Möglichkeite diskutiert
 * Elektriker-Rohr mit Doppelmuffe (eingeklebt mit Silikon-Kleber)
 * Andere Möglichkeit: völlig dicht!
 
#####Allgemein
 * rxf fragt Rajko, wie das mit extra Daten ist 
 * max. Anzahl Teilnehmer an  einem Workshop: 10
 * Workshop-Teilnehmer brauchen Löterfahrung!
 * Workshop frühestens etwa Mitte November
 