#!/usr/bin/python
# read and execute proceduce file *.rdbe for a specific rdbe
# rdbe2: 134.104.69.35
# rdbe1: 134.104.69.34
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
                  default = "134.104.69.34",
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
  BUFFER_SIZE = 1024
  s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
  s.settimeout( int(timeout) )
  communicationOk = True
  # try connection and send message
  try:
    s.connect( (ip_addr, int( ip_port ) ) )
    #sleep(0.2)  # added for slow devices, e.g. DMM4040
#   if verbose: print "  Sending message:  ", message,
    s.send( message )
    # try to flush and wait
    message = ""                   # clear message
  except:                          # Time out
    print "Error: no connection, no message sent",
    message = "no_connection"
    communicationOk = False
    sys.exit()
  # receive data
  if ( communicationOk ):
      try:
        message = s.recv( BUFFER_SIZE )     # receive string
        if verbose: print "  Message received: ", message,
      except:
        message = "timeout"
        communicationOk = False
        sys.exit()
  #sleep(0.2)    # added for slow devices, e.g. DMM4040
  s.close()
  return message

# main program
def main():
  global opts
  (opts, args) = parser.parse_args()
  if len(args) < 1:
    print "\nPlease, give a command\n"
    parser.print_help()
  else:
    vsiCommand = args[0]+";\n"
    verbose = False
    if opts.verbose:
      verbose = True
    # send/receive command
    #print 'VSI command:  ', vsiCommand,
    vsiResponse = sendReceive( opts.ip_addr,
      opts.ip_port, opts.timeout, vsiCommand,
      opts.verbose)
    print  vsiResponse,
# stand alone program
if __name__ == "__main__":
  main()
