import numpy as np


class ECGFilter():
    def __init__(self, noise=0.000001, l=100, w=11, sample_time=1 / 125, deg=0.99, acth=0.7, noisedeg=0.999, t_len=5):
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


if __name__ == '__main__':
    import matplotlib.pyplot as plt

    with open("./Data0.txt", "r") as f:
        data = np.array([float(i) for i in f.readlines()])

    f = ECGFilter()
    datat = [f.getFilteredData(i) for i in data]
    l = 8000
    h = 8900
    debug0 = [0 for i in range(l, h)]
    plt.plot([i[3] for i in datat[l:h]])
    # plt.plot([i[3] for i in datat[l:h]])
    plt.plot([i[1] for i in datat[l:h]])
    plt.show()
