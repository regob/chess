#!/bin/sh

qmake --version
a=$?
qmake-qt5 --version
b=$?
qt5-qmake --version
c=$?

if [ $a -eq 0 ]; then
    qmake ../client.pro
elif [ $b -eq 0 ]; then
    qmake-qt5 ../client.pro
elif [ $c -eq 0 ]; then
    qt5-qmake ../client.pro
else
    echo "No qmake found."
    exit
fi

make
echo "Client installed."

