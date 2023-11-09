#!/bin/bash


filesdir=$1
searchstr=$2


if [[ $filesdir = '' ]] || [[ $searchstr = '' ]]
then
	echo missing arguments
	exit 1
fi


if [ !  -d ${filesdir} ]
then
	echo argument 1 is not a directory
	exit 1
fi

X=$( ls ${filesdir} | wc -l )



Y=$( grep -s ${searchstr} ${filesdir}/* | wc -l )

echo The number of files are $X and the number of matching lines are $Y
