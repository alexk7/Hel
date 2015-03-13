#!/bin/bash

#echo '#pragma once'

content=`sed '/#pragma once/d;/#include ".*/d;'`
headers=`ls ./include/*.h`
symbols=`echo -n "$headers" | sed 's|.*/\(.*\)\.h|\1|' | tail -r`

echo -n "$content" | grep -Fo "$symbols"

#echo $pgm
#content=`sed '/#pragma once/d;/#include ".*/d;'`
#echo $content
