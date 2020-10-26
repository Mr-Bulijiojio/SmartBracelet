import serial
import time
import os
import numpy
import pyqtgraph as pg
from pyqtgraph.Qt import QtGui
from ECGFilter import *

Dataport = None
signal = [0 for i in range(300)]
readBuffer = ""
# dataBuffer =

Filer = ECGFilter()

filename = "data.txt"
fp = open(filename, "wb")


def serialConfig():
    global Dataport

    Dataport = serial.Serial('COM8', 921600)
    Dataport.flushInput()  # 清空缓冲区

    return Dataport


def readUartData(Dataport):
    global readBuffer

    data = 0
    dataOK = 0

    count = Dataport.inWaiting()

    if count > 20:
        segData = Dataport.readline().decode()
        if segData.count('.') < 2:
            data = (-float(segData))
            dataOK = 1

    return dataOK, data


cnt = 1
length = 10000


def plotData():
    global signal
    global cnt

    dataOK, data = readUartData(Dataport)

    if dataOK:

        # cnt += 1
        #
        #
        # if cnt < length:
        #     fp.write((str(data) + "\n").encode())
        # elif cnt == length:
        #     fp.close()
        #     print('finish')


        _, data ,rate,_= Filer.getFilteredData(data)
        print(rate)
        signal.append(data)
        # while len(signal) > 300:
        #     signal.pop(0)
        signal.pop(0)
        s1.setData(numpy.array((list(range(0, 300)), signal)).T)


# START QtAPPfor the plot
app = QtGui.QApplication([])

# Set the plot
pg.setConfigOption('background', 'w')
win = pg.GraphicsWindow(title="ECG Graph")


p1 = win.addPlot()
p1.setXRange(0, 300)
p1.setYRange(-3.3, 3.3)
p1.setLabel('left', text='Y position (mm)')
p1.setLabel('bottom', text='time (pre 50ms)')
PEN = pg.mkPen(width=3, color='r')
s1 = p1.plot([], [], pen=PEN)

Dataport = serialConfig()

timer = pg.QtCore.QTimer()
timer.timeout.connect(plotData)  # 定时调用plotData函数
timer.start(5)  # 多少ms调用一次

app.exec_()
