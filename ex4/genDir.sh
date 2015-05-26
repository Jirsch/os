#!/bin/bash

rm -f -r /tmp/root/
rm -f -r /tmp/mount/


mkdir /tmp/mount
mkdir /tmp/root
mkdir /tmp/root/inDir
mkdir /tmp/root/secondDir

for i in {1..1000000}; do
    if !((i%1000)); then
        echo -n $i >> /tmp/root/inDir/large
        echo "|" >> /tmp/root/inDir/large
    fi

    echo -n $((i % 10)) >> /tmp/root/inDir/large
    echo "|" >> /tmp/root/inDir/large

done

echo "1111111111222222222233333333334444444444" > /tmp/root/text