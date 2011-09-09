#!/bin/sh

CWD=`pwd`

cd structg
make clean
cd $CWD

rm -rf ./structg/PRE
rm -rf *.h *.stg *.a
