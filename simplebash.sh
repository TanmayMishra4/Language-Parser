#!/bin/bash
# This script file should be executable i.e. :    chmod u+x bsatest.sh
# then run it with: ./bsatest.sh
# This script should be in the same directory the Alloc directory
# If extension has been attempted, uncomment on lines 10, 41-55
rm parse_n
make parse_n
timeout 20 ./parse_n errortext.txt 
r = $?
case "$r" in
1)
echo "Passed error test correctly"
;;
0)
echo "Error: Return value is 0 for error case"
exit 1
;;
*)
echo "Incorrect return value of ($r) returned for incorrect case"
exit 1
;;
esac
timeout 20 ./parse_n TTL/5x5.ttl
r = $?
case "$r" in
0)
echo "Passed valid test correctly"
;;
1)
echo "Error: Return value is 1 for correct case"
exit 1
;;
*)
echo "Incorrect return value of ($r) returned for correct case"
exit 1
;;
esac

