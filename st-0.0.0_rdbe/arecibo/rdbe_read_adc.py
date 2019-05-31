#!/usr/bin/python
# NAIC AO, lquintero,yamaro, 6 Jun 2011

from optparse import OptionParser
from time import sleep
import socket
from binascii import b2a_hex
from struct import *
import sys
sys.path.insert(0, '/usr2/st/statlib-1.1')
from statlib import stats

# command line options
usage = "%prog [options] \'message\'"
description = "Modbus command using SCPI"
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
                  default = 5050,
                  help="device ip port")
parser.add_option( "-t", "--timeout",
                  dest = "timeout",
                  default = 5,
                  help="socket timeout")

# scpi command, send and send/receive mode
def sendReceive( ip_addr, ip_port, timeout, message, verbose):
#  BUFFER_SIZE = 8192
  BUFFER_SIZE = 1024
#  BYTES_PER_PACKAGE = 1448  # to confirm!
  s = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
  s.settimeout( int(timeout) )
  communicationOk = True
  # try connection and send message
  try:
    s.connect( (ip_addr, int( ip_port ) ) )
    #sleep(0.2)  # added for slow devices, e.g. DMM4040
    s.send( message )
    # try to flush and wait
    if verbose: print "message sent: ", message
    message = ""                   # clear message
  except:                          # Time out
    if verbose: print "Error: no connection, no message sent"
    message = ""
    communicationOk = False
  # receive data
  if ( communicationOk ):
      try:
        message = s.recv( BUFFER_SIZE )     # receive string
      except:
        if verbose: print "Warning: Not received message (timeout)"
        message = ""
        communicationOk = False
  #sleep(0.2)    # added for slow devices, e.g. DMM4040
  s.close()
  return message
  

# main program
def main():
  (opts, args) = parser.parse_args()
  if len(args) != 1:
    print "\nPlease, select channel: 0/1\n"
    parser.print_help()
  else:
    if opts.verbose:
      verbose = True
      #print "connecting to ", opts.ip_addr
      #print "port number", opts.ip_port
    message = args[0]
    message = sendReceive( opts.ip_addr, opts.ip_port, opts.timeout,
                           message, opts.verbose)
    hexMessage = b2a_hex( message )
    
    #Unpacking message
    msg_len = len(message)
    num = unpack(str(msg_len)+'B', message)
    num = map(lambda x: x-128, num)
    #print num
    print "Maximum: ", max(num), "Minimum: ", min(num) , "Mean: ", stats.mean(num) , "Standard deviation: ", stats.stdev(num) , "Variance: ", stats.var(num),
    
# stand alone program
if __name__ == "__main__":
  main()
