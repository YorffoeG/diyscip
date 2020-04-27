#!/usr/bin/python

##
 # DIYSCIP (c) by Geoffroy HUBERT - yorffoeg@gmail.com
 # This file is part of DIYSCIP <https://github.com/yorffoeg/diyscip>.
 # 
 # DIYSCIP is licensed under a
 # Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License.
 # 
 # You should have received a copy of the license along with this
 # work. If not, see <http://creativecommons.org/licenses/by-nc-sa/4.0/>.
 # 
 # DIYSCIP is distributed in the hope that it will be useful,
 # but WITHOUT ANY WARRANTY; 
 

import sys, getopt
import gzip
import shutil

def usage():
    print '\nhtml2cpp.py -s <source>'
    print 'Convert an html source to cpp source\n\n'


def main(argv):
    source = None
    destination = None

    try:
        opts, args = getopt.getopt(argv, "s:d:",["source=", "destination="])
    except getopt.GetoptError,err:
        print str(err)
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-s", "--source"):
            source = arg
        elif opt in ("-d", "--destination"):
            destination = arg

    if not source:
        usage()
        sys.exit(2)

    if not destination:
        destination = source + '.h'

    with open(source, 'rb') as f_in:
        with gzip.open(source + '.gz', 'wb') as f_out:
            shutil.copyfileobj(f_in, f_out)

    f_in.close()
    f_out.close()

    hf = open(destination, 'w')
    hf.write('/* automatically created by html2cpp from ' + source + '   */\n');
    hf.write('const char home[] PROGMEM = {');

    gz = open(source + '.gz', 'rb')
    ba = bytearray(gz.read())
    len= 0
    for byte in ba:
        hf.write('0x' + ''.join('{:02x}'.format(byte)) + ',')
        len = len + 1

    gz.close()
    hf.write('0};\n');
    hf.write('const int home_len = ' + str(len + 1) + ';\n')
    hf.close()

    


if __name__ == "__main__":
    main(sys.argv[1:])
