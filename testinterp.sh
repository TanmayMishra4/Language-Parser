#!/bin/bash
rm interp_s
make interp_s
touch testoutput.txt
directory=`ls TTLs/`
for i in $directory
do
    timeout 20 ./interp_s TTLs/$i output.txt > testoutput.txt
    r=$?
    if [ "$i" = "fail_parse_ok_interp.ttl" ] || [ "$i" = "ok_parse_fail_interp.ttl" ]; then
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
echo -e "\n"
echo "ALL TEST CASES PASSED!!"
rm testoutput.txt

