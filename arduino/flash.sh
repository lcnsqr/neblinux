#!/bin/bash

AVRDUDE=`which avrdude`

if [ ! -x "$AVRDUDE" ]
then
  echo "avrdude não encontrado"
  exit 1
fi

$AVRDUDE -C avrdude.conf -v -p atmega328p -c arduino -P /dev/ttyUSB0 -b 57600 -D -U flash:w:device.hex:i
