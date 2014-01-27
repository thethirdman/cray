#!/bin/sh

for f in `ls scenes/*.xml`; do
  of=${f#scenes/}
  echo "./cray $f renders/${of%.xml}.png $1 $2"
  ./cray $f renders/${of%.xml}.png $1 $2
done
