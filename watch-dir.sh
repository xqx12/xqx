#!/bin/bash
#Filename: watch-dir.sh
#Last modified: 2017-04-12 03:11
# Author: Qixue Xiao <xiaoqixue_1@163.com>
#Description: 

if [ $# = 0 ]
then
    echo "must specify a dir"
    exit 0
fi

if [ ! -d $1 ]
then
    echo $1 "is not a dir"
    exit 0
fi

DIR=$1

declare -a OldNameArray
declare -a OldHashArray
declare -a NewNameArray
declare -a NewHashArray
TimeStamp=$(date +%Y%m%d%H%M%S)
FileNum=0

i=0
#names=( $(ls) )
#echo $names
#echo ${names[$i]}
for fname in `find $DIR -name "*"`
do
    OldNameArray[$i]=$fname
    OldHashArray[$i]=`ls -l $fname| md5sum| cut -c1-32 `
    #echo ${OldNameArray[$i]}
    #i=`expr $i+1`
    i=$[$i+1]
done

echo $i
FileNum=${#OldNameArray[@]}
echo "len=""${#OldNameArray[@]}"
echo "${OldHashArray[@]}"

while [ $FileNum != 0 ] ;
do
    i=`expr 0`

    CurTime=$(date +%s)
    for fname in `find $DIR -name "*"`
    do
        NewNameArray[$i]=$fname
        NewHashArray[$i]=`ls -l $fname| md5sum| cut -c1-32 `
        i=$[$i+1]
    done

    if [ $i -ne $FileNum ]
    then
        if [ $i -gt $FileNum ]
        then
            echo "new file create "$FileNum "-->" $i
            for((k=0;k<$i;k++))
            do
                j=0
                for((j=0;j<$FileNum;j++))
                do
                    if [ ${NewNameArray[$k]} = ${OldNameArray[$j]} ]
                    then
                        break
                    fi
                done
                if [ $j = $FileNum ]
                then
                    echo -e "\tnewfile -- "${NewNameArray[$k]}
                fi
            done
        fi
        if [ $i -lt $FileNum ]
        then
            echo "old file deleted"$FileNum "-->" $i
            for((j=0;j<$FileNum;j++))
            do
                for((k=0;k<$i;k++))
                do
                    if [ ${NewNameArray[$k]} = ${OldNameArray[$j]} ]
                    then
                        break
                    fi
                done
                if [ $k = $i ]
                then
                    echo -e "\tdelfile -- "${OldNameArray[$j]}
                fi
            done
        fi

        FileNum=$i
        i=0
        for fname in `find $DIR -name "*"`
        do
            OldNameArray[$i]=$fname
            OldHashArray[$i]=`ls -l $fname| md5sum| cut -c1-32 `
            i=$[$i+1]
        done
        continue


    fi

    FileNum=$i
    i=0

    #for fname in `find $DIR -name "*"`
    for((i=0;i<$FileNum;i++))
    do
        #NewNameArray[$i]=$fname
        #NewHashArray[$i]=`ls -l $fname| md5sum| cut -c1-32 `
        if [ ${OldNameArray[$i]} = ${NewNameArray[$i]} ]
        then
            if [ ${OldHashArray[$i]} != ${NewHashArray[$i]} ]
            then
                echo ${NewNameArray[$i]}" -- file change!"
                OldHashArray[$i]=${NewHashArray[$i]}
            fi
        fi
    done

    TimeStamp=$(date +%Y%m%d%H%M%S)
    sleep 3

done


