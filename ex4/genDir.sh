#!/bin/bash

rm -f -r /tmp/root/
rm -f -r /tmp/mount/


mkdir /tmp/mount
mkdir /tmp/root
mkdir /tmp/root/inDir
mkdir /tmp/root/secondDir

for i in {1..999999}; do
    echo ((i % 10)) >> /tmp/root/inDir/large
    echo "|" >> /tmp/root/inDir/large
    if !((i%1000)); then
        echo $i >> /tmp/root/inDir/large
        echo "|" >> /tmp/root/inDir/large
    fi
done

echo "1111111111222222222233333333334444444444" > /tmp/root/text