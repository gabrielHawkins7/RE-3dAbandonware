import std/os
import std/random


{.compile: "ValidateKey.c".}

proc CHKSUM(serial_key: cstring): cint {.importc, cdecl.}

proc validate_key(serial_key: cstring, hwid: cstring, param_01: cint): cint {.importc, cdecl.}

# XF83WBD-8473803-USE
# Valid Serial Key
# XF[##]W[XX]-[######X]-CCC
# XF - HWID, W - PLATFORM, CCC - CHKSUM
echo "Generating Serial Key"
echo "Valid Key Example: XF83WBD-8473803-USE"

randomize()

var skey: string = ""

for i in 0..18:
    case (i)
    of 0:
        skey.add("X")
    of 1:
        skey.add("F")
    of 2,3:
        skey.add($rand(0..9))
    of 4:
        skey.add("W")
    of 5:
        var valid_chars = ['B','C','E','N','R','U']
        var index = rand(0..100)
        skey.add(valid_chars[index mod 5])
    of 6:
        var valid_chars = ['B','C','D','F','G','M','J','K','L','N','P','S','T','Z']
        var index = rand(0..100)
        skey.add(valid_chars[index mod 13])
    of 7:
        skey.add("-")
    of 8,9,10,11,12,13,14:
        skey.add($(rand(0..9)))
    of 15:
        skey.add("-")
    of 16,17,18:
        skey.add("A")
    else: echo "who"

#echo skey

let testkey = "XF83WBD-8473803-AAA"


let chksum = CHKSUM(skey)

#echo "Last 3 must match: ", chksum


var chkvalue = (int) chksum
var digits: array[3,int]
var outdig: array[3,int]

for i in countdown(2,0):
    digits[i] = chkvalue mod 24
    chkvalue = (int) chkvalue / 24

for i in countup(0,2):
    var d = digits[i]
    if d <= 7:
        outdig[i] = int('A') + d
    elif d <= 12:
        outdig[i] = int('J') + (d-8)
    else:
        outdig[i] = int('P') + (d-13)

skey[16] = char(outdig[0])
skey[17] = char(outdig[1])
skey[18] = char(outdig[2])


echo "Generated Key: ", skey

echo "Testing Key..."

var valid = validate_key(cstring(skey), "XF", 1)

if valid == 1:
    echo "Generated correct key [", skey, "]"
else:
    echo "Unable to generate key :<"



