import asyncio
import numpy as np
from multiprocessing import Process, Pipe
from PyQt5 import QtCore, QtGui, QtWidgets
import sys
import pyqtgraph as pg
import time
from PyQt5.QtWidgets import QSizePolicy

def process1(pipe):
    class TurningDetected():
        def __init__(self):
            self.pre = 0
            self.k_buffer = np.zeros(5)
            self.k_point = 0
            self.t_pre = 0
            self.flag = False
        def get(self, angle):
            self.k_buffer[self.k_point] = abs((angle - self.pre + 180) % 360 - 180)
            t = 1.4 * self.k_buffer[(self.k_point + 3) % 5] - 0.4 * sum(self.k_buffer)
            t = t if t > 0 else 0
            l = t - self.t_pre
            self.k_point = 1 + self.k_point if self.k_point < 4 else 0
            self.t_pre = t
            self.pre = angle
            if self.flag:
                if l < 2:
                    self.flag = False
                return 0
            else:
                if l > 6:
                    self.flag = True
                    return 1
                else:
                    return 0
            
            
    class ECGFilter():
        def __init__(self, noise=0.0000004, l=100, w=11, sample_time=1/125, deg=0.99, acth = 0.7, noisedeg = 0.999, t_len = 7):
            self.noise = noise
            self.l = l
            self.w = w
            self.lind = -1
            self.wind = -1
            self.fifo = np.zeros(l)
            self.wienerfifo = np.zeros(w)
            self.wienervfifo = np.zeros(w)
            self.maxn = 0
            self.minn = 0
            self.deg = deg
            self.acth = acth
            self.top = 0
            self.pre_counter = 0
            self.cur_counter = 0
            self.noisedeg = noisedeg
            self.sample_time = sample_time
            self.t = np.zeros(t_len)
            self.t_len = t_len
            self.t_ind = 0
            self.flag = 0
            self.preacc = 0
    
        def calc_hth(self):
            return self.acth * self.maxn + (1 - self.acth) * self.minn
        
        def calc_lth(self):
            return self.acth * self.minn + (1 - self.acth) * self.maxn
    
        def getFilteredData(self, data):
            """
            Returns
            (中值滤波后的结果, 中值滤波加维纳滤波后的结果, 心率间隔)
            """
            data = -data
            self.cur_counter += 1
            if self.lind == -1:
                for i in range(self.l):
                    self.fifo[i] = data
                self.lind = 0
            elif self.lind == self.l - 1:
                self.lind = 0
                self.fifo[self.lind] = data
                data -= np.median(self.fifo)
            else:
                self.lind += 1
                self.fifo[self.lind] = data
                data -= np.median(self.fifo)
            if self.wind == -1:
                datav = data ** 2
                for i in range(self.w):
                    self.wienerfifo[i] = data
                    self.wienervfifo[i] = datav
                self.wind = 0
                dataf = data
                datav -= dataf
            elif self.wind == self.w - 1:
                self.wind = 0
                self.wienerfifo[self.wind] = data
                self.wienervfifo[self.wind] = data ** 2
                dataf = np.median(self.wienerfifo)
                datav = np.mean(self.wienervfifo) - dataf ** 2
            else:
                self.wind += 1
                self.wienerfifo[self.wind] = data
                self.wienervfifo[self.wind] = data ** 2
                dataf = np.median(self.wienerfifo)
                datav = np.mean(self.wienervfifo) - dataf ** 2
            
            if datav < self.noise:
                res = dataf
            else:
                res = (self.wienerfifo[(self.wind + self.w // 2 + 1) % self.w] - dataf)
                res *= (1 - self.noise * self.top ** 2 / datav)
                res += dataf
            self.top *= self.noisedeg
            if res >= self.top:
                    self.top = res
            self.maxn *= self.deg
            if res > self.preacc:
                speed = (res - self.preacc)
                if speed >= self.calc_hth() and self.flag:
                    self.t[self.t_ind] = self.cur_counter - self.pre_counter
                    if self.t_ind == self.t_len - 1:
                        self.t_ind = 0
                    else:
                        self.t_ind += 1
                    self.pre_counter = self.cur_counter
                    self.flag = 0
                elif speed <= self.calc_lth() and not self.flag:
                    self.flag = 1
                if speed >= self.maxn:
                    self.maxn = speed
            self.preacc = res
            t = np.mean(self.t)
            if t == 0:
                t = 1
            return data, res, 60 / t / self.sample_time, self.top * self.flag
    
    class Data():
        def __init__(self, name="", data=()):
            self.data = data
            self.name = name
            
    def handle_recv_mother(queue):
        async def handle_recv(reader, writer):
            print("Socket connected")
            while(True):
                data = await reader.readline()
                try:
                    queue.put_nowait(data.decode().strip())
                except UnicodeDecodeError:
                    print("UnicodeDecodeError when decode socket data")
        return handle_recv

    async def handle_send(pipe, queue):
        cntstep = 0
        cntemp = 0
        cntecg = 0
        totlelength = 600
        fil = ECGFilter()
        dec = TurningDetected()
        distance = 0
        while(True):
            data = await queue.get()
            data = data.split(":")
            # print(data)
            try:
                data[1] = data[1].split(",")
            except:
                print("Data Format Error Split")
                print(data)
                continue
            dictsend = {"ECG":None, "Temp": None, "Step": None}
            if data[0] == "ECG":
                try:
                    message = [fil.getFilteredData(float(data[1][i]))[1:3] for i in range(len(data[1]) - 1)]
                except:
                    print("Data Format Error ECG")
                    print("data:", data)
                    continue
                if cntecg < totlelength:
                    with open("./saveecg.txt", "a") as fp:
                        for k in message:
                            fp.write(str(k[0])+", ")
                        cntecg += 1
                        print("Step ecg No.", cntecg)
                dictsend["ECG"] = message
                pipe.send(dictsend)
                # print("ECG Got")
            elif data[0] == "Temp":
                try:
                    message = (float(data[1][0]))
                except:
                    print("Data Format Error Temp")
                    print("data:", data)
                    continue
                if cntemp < totlelength:
                    with open("./savetemp.txt", "a") as fp:
                        st = str(message)+", "
                        fp.write(st)
                        cntemp += 1
                        print("Step temp No.", cntemp)
                dictsend["Temp"] = message
                pipe.send(dictsend)
                # print("Temp Got")
            elif data[0] == "Step":
                try:
                    message =  [float(data[1][i]) for i in range(len(data[1]) - 1)]
                    if message[1] < -181 or message[1] > 181:
                        print("Data Format Error Step")
                        print("data:", data)
                        continue
                    message.append(dec.get(message[1]))
                except:
                    print("Data Format Error Step")
                    print("data:", data)
                    continue
                    
                if cntstep < totlelength:
                    with open("./savefoot.txt", "a") as fp:
                        st = str(message[0])+", "
                        fp.write(st)
                        cntstep += 1
                        print("Step save No.", cntstep)

                dictsend["Step"] = message
                pipe.send(dictsend)
                # print("Step Got")
            else:
                print("Data Invalid")
                print(data)
    async def main(pipe):
        queue = asyncio.Queue()
        server = await asyncio.start_server(
        handle_recv_mother(queue), '0.0.0.0', 1334)
        task2 = asyncio.create_task(handle_send(pipe, queue))
        await task2
        async with server:
            await server.serve_forever()
    
    asyncio.run(main(pipe))

def gui(pipe):
    print(f'GUI connected')
    class MainGui(QtWidgets.QMainWindow):
        left = 100
        top = 100
        width = 960
        height = 700
        length = 400
        output_heart=[0 for j in range(0, length)]
        def __init__(self):
            super().__init__()
            self.title = 'Contoller'
            self.acc = 0
            self.init_ui()
        def init_ui(self):
            self.setWindowTitle(self.title)
            self.setGeometry(self.left, self.top, self.width, self.height)

            self.main_widget = QtWidgets.QWidget()
            self.main_layout = QtWidgets.QGridLayout()
            self.main_widget.setLayout(self.main_layout)

            self.left_widget = QtWidgets.QWidget()
            self.left_layout = QtWidgets.QGridLayout()
            self.left_widget.setLayout(self.left_layout)
            self.left_widget.setSizePolicy(QSizePolicy.Maximum, QSizePolicy.Expanding)

            self.right_widget = QtWidgets.QWidget()
            self.right_layout = QtWidgets.QGridLayout()
            self.right_widget.setLayout(self.right_layout)
            self.right_widget.setSizePolicy(QSizePolicy.Expanding, QSizePolicy.Expanding)

            self.main_layout.addWidget(self.left_widget, 0, 0, 13, 1)
            self.main_layout.addWidget(self.right_widget, 0, 1, 12, 6)

            self.main_layout.setColumnStretch(0, 1)
            self.main_layout.setColumnStretch(1, 6)
            self.setCentralWidget(self.main_widget)  # 设置窗口主部件

            self.lb1 = QtWidgets.QLabel("温度检测:24.125")
            self.lb2 = QtWidgets.QLabel("心跳速率:0")
            self.lb3 = QtWidgets.QLabel("步数计算:0")
            self.lb4 = QtWidgets.QLabel("里程计算:0")
            btn1 = QtWidgets.QPushButton("里程清零")
            btn1.clicked.connect(self.cle)

            plot_Rate_heart = pg.PlotWidget()
            self.autoPixelRange=True

            self.left_layout.addWidget(self.lb1, 0, 0, 1, 2)
            self.left_layout.addWidget(self.lb2, 2, 0, 1, 2)
            self.left_layout.addWidget(self.lb3, 4, 0, 1, 2)
            self.left_layout.addWidget(self.lb4, 6, 0, 1, 2)
            self.left_layout.addWidget(btn1, 7, 0)
            self.left_layout.setRowStretch(0, 1)
            self.left_layout.setRowStretch(1, 1)
            self.left_layout.setRowStretch(2, 1)
            self.left_layout.setRowStretch(3, 1)
            self.left_layout.setRowStretch(4, 1)
            self.left_layout.setRowStretch(5, 1)
            self.left_layout.setRowStretch(6, 1)
            self.left_layout.setRowStretch(7, 1)
            self.left_layout.setRowStretch(8, 7)

            self.right_layout.addWidget(plot_Rate_heart, 1, 0, 3, 2)
            self.right_layout.setRowStretch(1, 0)
            self.right_layout.setRowStretch(2, 5)
            self.right_layout.setRowStretch(11, 0)

            self.plot_Rate_heart = plot_Rate_heart.plot()
            self.plot_Rate_heart.setData(self.output_heart,
                                         pen='y', symbol=None, symbolBrush='g')
        def refresh(self, data):
            t = data["Temp"]
            if t:
                self.lb1.setText('温度检测:%f' % t)
                QtWidgets.QApplication.processEvents()
            t = data["Step"]
            if t:
                self.acc += t[-1]
                self.lb3.setText('步数计算:%d' % t[0])
                self.lb4.setText('里程计算:%d' % ((self.acc + t[0]) * 0.75))
                QtWidgets.QApplication.processEvents()
            t = data["ECG"]
            if t:
                self.lb2.setText('心跳速率:%f' % t[-1][1])
                QtWidgets.QApplication.processEvents()
                self.output_heart = (self.output_heart + [i[0] for i in t])[-400:]
                self.plot_Rate_heart.setData(self.output_heart,
                                         pen='y', symbol=None, symbolBrush='g')
        def cle(self):
            self.acc = 0
    app = QtWidgets.QApplication(sys.argv)
    gui = MainGui()
    gui.show()
    while(True):
        # print("hhhhhhhhh")
        gui.refresh(pipe.recv())
    sys.exit(app.exec_())

if  __name__ == "__main__":
    conn_send, conn_recv = Pipe()
    p_process1 = Process(target=process1, args=(conn_send,))
    p_gui = Process(target=gui, args=(conn_recv,))
    p_process1.start()
    p_gui.start()
    p_process1.join()
    p_gui.join()
