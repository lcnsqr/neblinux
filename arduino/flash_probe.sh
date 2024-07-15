#!/bin/bash

AVRDUDE=`which avrdude`
PORT=/dev/ttyACM0

if [ ! -x "$AVRDUDE" ]
then
  echo "avrdude não encontrado"
  exit 1
fi

$AVRDUDE -v -p atmega328p -c arduino -P $PORT -b 115200 -D -U flash:w:probe.hex:i
