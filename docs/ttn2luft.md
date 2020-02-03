# TTN --> Luftdaten
rxf 2020-02-03

### Payload der LoRa-Sendung


Port | ByteNr | Wert [hex]| Beschreibung | Beispiele
-----|--------|------|------------- | ---------
1    |   0/1/2/3 | 00000107 | Anzahl der Impulse (counts) | => 263
1    | 4/5/6 | 0249F0 | Messzeit [ms] für diese Impulse (sample\_time\_ms) | => 150000
1	  |	  7/8   | 10C0   | Software-Version (software_version)| 1.12.0 (siehe unten)
1	  |	   9    |  16    | Bezeichnung des Zählrohres (tube) |Si**22**G
||||
2	  |	  0/1   | 0107   | BME280 Temperatur in 0.1° (temperature)| 0x107 => 26.3°
2	  |	   2    | 9A     | BME280 Feuchte in 0.5% (humiduty)|0x9A => 77.0%
2	  |	  3/4   | 26E0   | BME280 Luftdruck in 0.1 hPa (pressure) | 0x26E0 => 995.2 hPa

Erläuterung zur Software-Version: Die obersten 4 Bit sind die Major-Version (hier 1, max. 15), die folgenden 8 bit die Minor-Version (hier 0x0C => 12) und die untersten 4 Bit der Patchlevel (hier 0).

Bezeichnung der z. Zt. implementierten Zählrohre:  

Name | Nummer [hex]
-----|-------
SBM-19 | 0x13
SBM-20	 | 0x14
Si22G	| 0x16


Die Daten des BME280 werden nur gesendet, wenn auch ein BME280 vorhanden ist.

### Payload-Decoder
Wir verwenden **keinen** Payload-Decoder.
Sollte es nötig sein, die Daten irgendwie umzuwandeln, dann muss die HTTP-Integration das machen - also das Programm, das da dahinter sitzt.
