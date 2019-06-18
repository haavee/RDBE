#!/usr/bin/python
# read and execute proceduce file *.rdbe for a specific rdbe
# rdbe2: 192.168.4.26
# rdbe1: 192.168.4.18
# NAIC AO, lquintero, yamaro, 6 Jun 2011

import sys  
import re # regular expr.
from optparse import OptionParser
from time import sleep
import socket
from binascii import b2a_hex
import sys

# command line options
usage = "%prog [options] \'message\'"
description = "Procedure file for RDBE boxes"
parser = OptionParser(usage=usage, description=description)
parser.add_option( "-v", "--verbose",
                  action="store_true", dest="verbose", default=False,
                  help = "verbose mode")
parser.add_option( "-i", "--ip-address",
                  dest = "ip_addr",
                  default = "192.168.4.18",
                  help = "device ip address")
parser.add_option( "-p", "--port",
                  dest = "ip_port",
                  default = 5000,
                  help="device ip port")
parser.add_option( "-t", "--timeout",
                  dest = "timeout",
                  default = 30,
                  help="socket timeout")

# send/receive VSI command
def sendReceive( ip_addr, ip_port, timeout, message, verbose):
  if verbose:
    print "  Connecting to IP/Port: ", ip_addr, \
          "/", ip_port
#    print "  Timeout: ", opts.timeout, \ 
#          "s (for dbe_personality use more than)"
  BUFFER_SIZE = 1024
  s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
  s.settimeout( int(timeout) )
  communicationOk = True
  # try connection and send message
  try:
    if verbose: print "  Opening socket..."
    s.connect( (ip_addr, int( ip_port ) ) )
    #sleep(0.2)  # added for slow devices, e.g. DMM4040
    if verbose: print "  Sending message:  ", message,
    s.send( message )
    # try to flush and wait
    message = ""                   # clear message
  except:                          # Time out
    print "Error: no connection, no message sent"
    message = ""
    communicationOk = False
    sys.exit()
  # receive data
  if ( communicationOk ):
      try:
        message = s.recv( BUFFER_SIZE )     # receive string
        if verbose: print "  Message received: ", message,
      except:
        if verbose: print "Warning: Not received message (timeout)"
        message = ""
        communicationOk = False
        sys.exit()
  #sleep(0.2)    # added for slow divices, e.g. DMM4040
  if verbose: print "  Closing socket..."
  s.close()
  return message

# main program
def main():
  global opts
  (opts, args) = parser.parse_args()
  if len(args) != 1:
    print "\nPlease, specify a procedure file\n"
    parser.print_help()
  else:
    fileName = './' + args[0]
    fileOk = True
    verbose = False
    if opts.verbose:
      verbose = True
      print 'Opening: ', fileName
    # trying to open fileName
    try:
      procFile = open(fileName, 'r')  # read only
    except IOError:
      print 'The file does not exist. Enter a valid name.'
      fileOk = False
    # read line by line, and send/receive command
    if fileOk:
      for vsiCommand in procFile:
        # condition for empty spaces or short lines
        # not eficient, but it works...
        # consider: re.sub(r'\s', '', vsiCommand)
        # to remove white spaces
        if vsiCommand[0] != '#' and len(vsiCommand)>5:
          print 'VSI command:  ', vsiCommand,
          vsiResponse = sendReceive( opts.ip_addr,
            opts.ip_port, opts.timeout, vsiCommand,
            opts.verbose)
          print 'VSI response: ', vsiResponse,
        else:
          if verbose: print "Comment line: ", vsiCommand,
        if verbose: print " "
      procFile.close()
   
# stand alone program
if __name__ == "__main__":
  main()
