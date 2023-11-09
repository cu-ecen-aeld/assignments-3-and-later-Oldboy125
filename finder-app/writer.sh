#!/bin/bash


writefile=$1
writestr=$2


if [[ $writefile = '' ]] || [[ $writestr = '' ]]
then
    echo missing arguments
    exit 1
fi

echo $writestr>tmp

$( install -D tmp $writefile )

rm -f tmp

if [ !  -e ${writefile} ]
then
    echo Error file not created
    exit 1
fi

