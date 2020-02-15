#!/usr/bin/python

from socket import *
import sys, getopt, socket

def usage():
    print '\ndbgterm.py -h <host> -p <port>'
    print 'Simple terminal to output debug\n\n'


def main(argv):
    host = None
    port = None

    try:
        opts, args = getopt.getopt(argv, "h:p:",["host=","port="])
    except getopt.GetoptError,err:
        print str(err)
        usage()
        sys.exit(2)

    for opt, arg in opts:
        if opt in ("-h", "--host"):
            host = arg
        elif opt in ("-p", "--port"):
            port = int(arg)

    if not host or not port:
        usage()
        sys.exit(2)

    # Create a TCP/IP socket
    socket.setdefaulttimeout(10)

    # Connect the socket to the port where the server is listening
    server_address = (host, port)
    timeout = False
    buffer  = None

    print '\n\n\n#*#*#*#*#*#      Trying to connect to %s port %s\n' % server_address

    try:

        while True:
            try:
                sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                sock.connect(server_address)

            except socket.timeout:
                sock.close()
                print '.'
                continue

            print '\n#*#*#*#*#*#      connected\n\n\n'

            while True:
                try:
                    buffer = sock.recv(1024)
                    if buffer == '%!PONG#':
                        timeout = False
                    else:
                        print '%s' %buffer

                except socket.timeout:
                    if (timeout):
                        print '\n\n\n#*#*#*#*#*#      connexion lost\n\n\n'
                        sock.close()
                        break

                    else:
                        timeout = True
                        sock.send('%!PING#')

    except Exception as e:
        print '\n\n\n#*#*#*#*#*#\n'
        print str(e)

if __name__ == "__main__":
    main(sys.argv[1:])
