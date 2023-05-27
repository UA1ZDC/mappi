#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#samp_rate = 5e6
#gain = 24.1
#freq = 94.0237e6
#       96.8457e6
#       85.5163e6

from PyQt5 import Qt
import sys
import getopt
#import usrp_rx_hrpt
#import FY3AB
#import FY3C
#import METOP_AB
#import meteor2_hrpt
import metop_decoder_demo

#handlers = { 'NOAA 15':    [ usrp_rx_hrpt.usrp_rx_hrpt ],
#             'NOAA 18':    [ usrp_rx_hrpt.usrp_rx_hrpt ],
#             'NOAA 19':    [ usrp_rx_hrpt.usrp_rx_hrpt ],
 #            'FENGYUN 3A': [ FY3AB.FY3AB ],
 #            'FENGYUN 3B': [ FY3AB.FY3AB ],
 #            'FENGYUN 3C': [ FY3C.FY3C ],
 #            'METOP-A':    [ METOP_AB.METOP_AB ],
#	     'METEOR-M 2': [ meteor2_hrpt.meteor2_hrpt ]
#             }

handlers = {
	     'NOAA 15':	   [ metop_decoder_demo.metop_decoder_demo ],
	     'NOAA 18':	   [ metop_decoder_demo.metop_decoder_demo ],
	     'NOAA 19':	   [ metop_decoder_demo.metop_decoder_demo ],
    	     'NOAA 20':	   [ metop_decoder_demo.metop_decoder_demo ],
	     'FENGYUN 3A':    [ metop_decoder_demo.metop_decoder_demo ],
	     'FENGYUN 3B':    [ metop_decoder_demo.metop_decoder_demo ],
	     'FENGYUN 3C':    [ metop_decoder_demo.metop_decoder_demo ],
	     'METEOR-M 2':    [ metop_decoder_demo.metop_decoder_demo ],
	     'METOP-A':	   [ metop_decoder_demo.metop_decoder_demo ],
	     'METOP-B':	   [ metop_decoder_demo.metop_decoder_demo ],
	     'METOP-C':	   [ metop_decoder_demo.metop_decoder_demo ],
	     'METEOR-M 2': [  metop_decoder_demo.metop_decoder_demo  ]
}

#Вывод ошибки
def printError(err):
    sys.stderr.write("Ctrl Error: " + err + "\n")

def printOut(out):
    sys.stderr.write("Ctrl Output: " + out + "\n")

#Проверка введённой команды
def check_command(answ, tb):
    command = ""
    value = 1.0
    try:
        command, value = answ.split("=", 1)
    except ValueError:
        printError("неверный формат команды")
        return

    try:
        value = float(value)
    except ValueError:
        printError("неверное значение")
        return
    
    command = command.strip()

    if command == "freq":
        tb.set_freq(value)
        value = tb.get_freq()   
    elif command == "quit":
        return
    else :
        printError("неверная команда")
        return

#    print command, "=", value

#---

def print_help():
    print('\nmappi_rxwrapper.py -n <sat_name> -f <freq>');
    print('Command format')
    print('\tfreq = <value>\t Set frequency')
    print( '\tquit          \t Exit')    


#def main(argv, top_block_cls=usrp_rx_hrpt.usrp_rx_hrpt):
def main(argv):
    try:
        opts, args = getopt.getopt(argv,"hf:n:",["freq=","name="])
    except getopt.GetoptError:
        print_help()
        sys.exit(2)

#    qapp = Qt.QApplication(sys.argv)

    freq = -1
    name = ''
        
    for opt, arg in opts:
        if opt == '-h':
            print_help()
            sys.exit()
        elif opt in ("-f", "--freq"):
            freq = float(arg)
        elif opt in ("-n", "--name"):
            name = arg

    name = name.strip();

    if not name:
        printError("не указано название спутника")
        sys.exit(1)

    tb = handlers[name][0]()
    
    if freq != -1:
        tb.set_freq(freq)


    tb.start()
    tb.wait()

    # if "NOAA" not in name:
    #     tb.show()    

    # def quitting():
    #     tb.stop()
    #     tb.wait()
    # qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)

   
    # qapp.exec_()

   # answ = raw_input()
   # while answ != "q" and answ != "quit":
   #     check_command(answ, tb)
   #     answ = raw_input()
        

   # tb.stop()
   # tb.wait()
    

    if len(handlers[name]) > 1:
        printOut("Process file to raw")
        pb = handlers[name][1]()
        tb.start()
        tb.wait()
    
    

if __name__ == '__main__':
    main(sys.argv[1:])
