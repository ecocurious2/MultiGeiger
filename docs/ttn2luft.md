#TTN --> Luftdaten
rxf 2019-11-07
###Vorschlag für Byteaufteilung:


Port | ByteNr | Wert | Beschreibung | Beispiele
-----|--------|------|------------- | ---------
1	  |	  0/1   | 0068   | cpm | counts per minute)
	  |	  2/3   | 0134   | Anzahl HV-Impulse  | 0x134 => 308  Impulse 
	  |	   4    |  19    | Bezeichnung des Rohres |SBM-**19**
	  |   5     | 18    | Software-Version  | 18 => V 1.8
2	  |	  0/1   | 0107   | BME280 Temperatur in 0.1° | 0x107 => 26.3°
	  |	   2    | 9A     | BME280 Feuchte in 0.5% |0x9A => 77.0%
	  |	  3/4   | 26E0   | BME280 Luftdruck in 0.1 hPa | 0x26E0 => 995.2 hPa
4	  |	  0/1   |  00C0  | externer Temp-Fühler (z.B. DS1820) | 0xC0 => 19.2°
Die interne Temperatur in der CPU kann leider nicht mehr ermittelt werden: den Sensor gibts in den aktuellen ESP32 nicht mehr.


###Payload-Decoder
Wir verwenden **keinen** Payload-Decoder.
Sollte es nötig sein, die Daten irgendwie umzuwandeln, dann muss die HTTP-Integration das machen - also das Programm, das da dahinter sitzt.
