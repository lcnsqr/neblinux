#!/bin/bash

AVRDUDE=`which avrdude`
PORT=/dev/ttyUSB0

if [ ! -x "$AVRDUDE" ]
then
  echo "avrdude não encontrado"
  exit 1
fi

$AVRDUDE -v -p atmega328p -c arduino -P $PORT -b 57600 -D -U flash:w:device.hex:i
