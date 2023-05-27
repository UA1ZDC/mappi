#!/usr/bin/env python2
# -*- coding: utf-8 -*-

#samp_rate = 5e6
#gain = 24.1
#freq = 94.0237e6
#       96.8457e6
#       85.5163e6

from PyQt4 import Qt
import sys
import getopt
import fmqt

#Вывод ошибки
def printError(err):
    sys.stderr.write("Ctrl Error: " + err + "\n")


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
    elif command == "gain":
        tb.set_gain(value)
        value = tb.get_gain()
    elif command == "rate":
        tb.set_samp_rate(value)
        value = tb.get_samp_rate()
    elif command == "quit":
        return
    else :
        printError("неверная команда")
        return

#    print command, "=", value

#---

def print_help():
    print
    print 'mappi_rxwrapper.py -r <sample_rate> -g <gain> -f <freq>'
    print 'Command format'
    print '\tfreq = <value>\t Set frequency'
    print '\tgain = <value>\t Set gain'
    print '\trate = <value>\t Set sample rate'
    print '\tquit          \t Exit'


def main(argv, top_block_cls=fmqt.fmqt):
    try:
        opts, args = getopt.getopt(argv,"hr:g:f:n:",["rate=","gain=","freq="])
    except getopt.GetoptError:
        print_help()
        sys.exit(2)

    qapp = Qt.QApplication(sys.argv)
        
    tb = top_block_cls()
        
    for opt, arg in opts:
        print "opt", opt, arg
        if opt == '-h':
            print_help()
            sys.exit()
        elif opt in ("-r", "--rate"):
            tb.set_samp_rate(float(arg))
        elif opt in ("-g", "--gain"):
            tb.set_gain(float(arg))
        elif opt in ("-f", "--freq"):
            print "arg", arg
            #tb.set_freq(float(arg))
            tb._freq_win.d_widget.sliderChanged(float(arg))


    tb.start()
    tb.show()

    def quitting():
        tb.stop()
        tb.wait()
    qapp.connect(qapp, Qt.SIGNAL("aboutToQuit()"), quitting)

    
    qapp.exec_()

    # answ = raw_input()
    # while answ != "q" and answ != "quit":
    #     check_command(answ, tb)
    #     answ = raw_input()
        

    # tb.stop()
    # tb.wait()

if __name__ == '__main__':
    main(sys.argv[1:])
