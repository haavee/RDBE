#!/usr/bin/python
# ALC autoadjustment
# ADC readings from rdbe port 5050
# dbe_alc command port 5000
# NAIC AO, lquintero, yamaro, 8 Jun 2011

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
description = "Auto adjust ALC attanuator"
parser = OptionParser(usage=usage, description=description)
parser.add_option( "-v", "--verbose",
                  action="store_true", dest="verbose", default=False,
                  help = "verbose mode")
parser.add_option( "-i", "--ip-address",
                  dest = "ip_addr",
                  default = "134.104.69.34",
                  help = "device ip address")
parser.add_option( "-t", "--timeout",
                  dest = "timeout",
                  default = 30,
                  help="socket timeout")

# ---------------------------------------------------------
# send and receive message using portSocket
# ---------------------------------------------------------
def sendReceive ( portSocket, message, packetNumber, buffer_size, verbose ):
  # sending message
  try:
    portSocket.send( message )
    message = ""                   # clear message
  except:                          # Time out
    print "sendReceive(): Error: no connection, no message sent"
    portSocket.close()
    sys.exit()
    
  # read packetNumber messages, return the last one
  for i in range(packetNumber):
    try:
      sleep(0.1)
      message = portSocket.recv( buffer_size )     # receive string
#      print "len: ", len(message), '(', i+1, '),',
    except:                          # Time out
      print "sendReceive(): Error: no connection, no message received, No. ", i+1
      portSocket.close()
      sys.exit()
  return message
  
# ---------------------------------------------------------
# read ADC values and return variance
# ---------------------------------------------------------
# var = adc_var ( ip_addr, PORT_ADC, timeout, channel, ADC_PACKET,
#                 BUFFER_SIZE, verbose)
def adc_var ( ip_addr, ip_port, timeout, channel, packetNumber, buffer_size, verbose):
  
  s_adc = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
  s_adc.settimeout( int(timeout) )
  # try connection
  try:
    s_adc.connect( (ip_addr, int( ip_port ) ) )
  except:
    print "adc_var(): Error, no ADC connection, port: ", ip_port,
    s_adc.close()
    sys.exit()

  # open connection
  message = sendReceive ( s_adc, channel, packetNumber, 
                          buffer_size, verbose ) 
  # close connection
  s_adc.close()
                          
  # unpacking message
  msg_len = len(message)
  if msg_len > 0:
    num = unpack(str(msg_len)+'B', message)
    #  add offset, [0 255] -> [-128 128]
    num = map(lambda x: x-128, num)
    if verbose:
      print "Min=", min(num), "Max=", max(num), "Mean=", stats.mean(num), "Var=", stats.var(num),  "Std=", stats.stdev(num)
    var = stats.var(num)
  else:
    var = "N/A"
  return var
  
# ---------------------------------------------------------
# set ALC using dbe_alc command (VSI-S)
# ---------------------------------------------------------
# message = alc_set  ( s_alc, channel, attn_current, SOLAR, verbose )
def alc_set  ( s_alc, channel, attn_current, SOLAR, verbose ):

  packetNumber = 1
  buffer_size = 1024
  # VSI-S command 
  # Example:channel=0, attn_current=8, SOLAR='off'
  #         messageOut='dbe_alc=0:8:off; messageIn=!dbe_alc:0;

  message = 'dbe_alc=' + str(channel) + ':' + str(attn_current) + ':' + SOLAR + ';' + chr(10)
  #print "alc_set(): ALC setup, VSI-S command:  ", message,
  message = sendReceive ( s_alc, message, packetNumber, 
                          buffer_size, verbose ) 
  #print "alc_set(): ALC setup, VSI-S response: ", message,
  return message
# ---------------------------------------------------------
# main program
# ---------------------------------------------------------
def main():
  (opts, args) = parser.parse_args()
  if len(args) != 1:
    print "\nPlease, select channel: 0/1\n"
    parser.print_help()
  else:
    if opts.verbose:
      verbose = True
    else:
      verbose = False

    SOLAR     = "off"
    PORT_ADC  = 5050
    PORT_ALC  = 5000
    STEP_SIZE = 2
    ATTN_MIN  = 0
    ATTN_MAX  = 31
    VAR_REF   = 10
    BUFFER_SIZE = 1024
    ADC_PACKET = 32
    ip_addr = opts.ip_addr
    timeout = opts.timeout
    
    # ALC socket
    s_alc = socket.socket( socket.AF_INET, socket.SOCK_STREAM )
    s_alc.settimeout( int(timeout) )
    #sleep(1)
    # try connection
    try:
      s_alc.connect( (ip_addr, int( PORT_ALC ) ) )
      if verbose: 
        print "main(): ALC, connecting to IP/PORT: ", ip_addr, '/', PORT_ALC
    except:
      print "main(): Error: No ALC connection, port: ", PORT_ALC,
      s_alc.close()
      sys.exit()
      
    # channel number, 0 or 1 - check this number!
    channel = args[0]
    
    # maximum attenuation to start
    msg = alc_set  ( s_alc, channel, ATTN_MAX, SOLAR, verbose )
    msg = alc_set  ( s_alc, channel, ATTN_MAX, SOLAR, verbose )
    sleep(2)
    var = adc_var ( ip_addr, PORT_ADC, timeout, channel, ADC_PACKET, 
                    BUFFER_SIZE, verbose)
#    var = adc_var ( s_adc, channel, ADC_PACKET, BUFFER_SIZE, verbose )
    #print "Variance (max. attn): ", var, "@", ATTN_MAX, "dB"
    #print " " # delete line
    attn_current = ATTN_MAX
    
    while var < VAR_REF:
      # step down attenuator
      attn_current = attn_current - STEP_SIZE
      msg = alc_set  ( s_alc, channel, attn_current, SOLAR, verbose )
#      sleep(2)
      var = adc_var ( ip_addr, PORT_ADC, timeout, channel, ADC_PACKET, 
                      BUFFER_SIZE, verbose)
      print "Variance: ", var, "@", attn_current, "dB"
#     print " " # consider to delete
    print "Final variance: ", var, "@", attn_current, "dB",
#    s_adc.close()
    s_alc.close()
   
# stand alone program
if __name__ == "__main__":
  main()
