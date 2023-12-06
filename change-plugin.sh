#!/bin/bash
# ./change-plugin.sh 

$(grep -Pzo '(?s)<CSVDataSet guiclass="TestBeanGUI" testclass="CSVDataSet" testname="[\w -!@#$%¨&*()]+" enabled="true">.{0,150}"filename">([\w\.]{4,50})<\/stringProp>.{0,30}"ignoreFirstLine">([a-z]{4,5})<.+?"recycle">([a-z]{4,5})<\/.+?"variableNames">([a-zA-Z0-9!@#$%¨&*,]+)<\/stringProp>.{0,15}<\/CSVDataSet>' $1 | sed 's/\x0/\n /g' > tempx.file) 

result=`./change-plugin tempx.file $1`
$(rm tempx.file)

case "$result" in
    *Error*)
        echo $result
        sucess="0"
        ;;
    *)
        echo "new file: " $result
        sucess="1"
        ;;
esac

