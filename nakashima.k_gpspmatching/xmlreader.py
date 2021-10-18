"""
python xmlreader.py ○○.xml

"""
import sys
import re


if len(sys.argv) < 2 :
    print('input (and optionally output) file names are needed.')
    print('usage: python3 gpxreader gpxfile cvsfile[return]')
    exit()

with open(str(sys.argv[1:])+ '.csv', mode='w') as outfile:
    with open(sys.argv[1], mode='r') as f:
        flag = -1
        lines = f.read()
        for lines in lines.split("\n"):
            if flag == 1:
                if  lines != "</gml:posList>":
                    outfile.write(lines)
                    outfile.write("\n")
            if lines == "</gml:posList>":
                flag = -1
                
            if lines == "<gml:posList>":
                flag = 1