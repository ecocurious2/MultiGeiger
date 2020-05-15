.. include:: global.rst.inc
.. highlight:: none
.. _usage:

Usage
=====

OLED display
############

Top line
--------

Left: Time since power-on (not shown on small displays).

Right: Overall average radiation since power-on.

Middle area
-----------

Current CPM (counts per minute) displayed using a rather big font.

Bottom line
-----------

This is a status display with 8 positions, numbered 0..7:

Rules of thumb:

- ``.`` usually means "off" or "unused".
- if you see some *number* (``0`` .. ``7``) within the status display line, something went wrong.


Positions:

- 0: WiFi

  - ``A``: AccessPoint active
  - ``w``: WiFi client trying to connect
  - ``W``: WiFi client connected
  - ``0``: some error happened
- 1: sensors.community transmission

  - ``.``: off (not configured / enabled)
  - ``?``: init (enabled, before 1st transmission)
  - ``S``: sending
  - ``s``: idle (shown after successful sending)
  - ``1``: sending failed (shown after trying to send)
- 2: madavi transmission

  - ``.``: off (not configured / enabled)
  - ``?``: init (enabled, before 1st transmission)
  - ``M``: sending
  - ``m``: idle (shown after successful sending)
  - ``2``: sending failed (shown after trying to send)
- 3: TTN ("The Things Network")

  - ``.``: off (not configured, not enabled, no LoRa hardware)
  - ``?``: init (enabled, before 1st transmission)
  - ``T``: sending
  - ``t``: idle (shown after successful sending)
  - ``3``: sending failed (shown after trying to send)
- 4: reserved for BT
- 5: unused
- 6: unused
- 7: High-Voltage Capacitor charging

  - ``H``: OK
  - ``7``: failure to charge HV capacitor

