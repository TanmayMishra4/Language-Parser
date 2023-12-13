#!/bin/bash
rm parse_n
make parse_n
touch testoutput.txt
directory=`ls TTLs/`
for i in $directory
do
    timeout 20 ./parse_n TTLs/$i > testoutput.txt
    r=$?
    if [ "$i" = "fail_parse_ok_interp.ttl" ]; then
        case $r in
        1)
        echo "Passed test for $i correctly"
        ;;
        *)
        echo "Error: Return value is $r for $i"
        exit 1
        ;;
        esac
    else
        case $r in
        0)
        echo "Passed test for $i correctly"
        ;;
        *)
        echo "Error: Return value is $r for $i"
        exit 1
        ;;
        esac
    fi  
done
rm testoutput.txt

