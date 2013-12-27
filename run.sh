#!/bin/sh

for f in `ls scenes`; do
  ./cray scenes/$f renders/${f%.*}.png $1 $2
done
