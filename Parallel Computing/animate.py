import csv

import matplotlib.animation as animation
import matplotlib.pyplot as plt
import numpy as np
import scipy.integrate as integrate
from scipy.spatial.distance import pdist, squareform


class Universe:
    def __init__(self):
        self.iter_cnt = 0
        self.iter_max = 0
        self.time_per_iter = 0.
        self.time_elapsed = 0.
        self.size = 0
        self.mass = []
        self.state = []

        # load init data
        with open('./data/init.csv', 'r') as init_f:
            reader = csv.reader(init_f)
            # data = list(reader)
            for i, row in enumerate(reader):
                if i == 0:  # meta info
                    self.size = int(row[0])
                    self.time_per_iter = float(row[1])
                    self.iter_max = int(row[2])
                    continue
                self.mass.append(float(row[0]))
                self.state.append([float(i) for i in row[1:]])
        self.mass = np.asarray(self.mass)
        self.state = np.asarray(self.state)

    def step(self, dt):
        self.time_elapsed += dt
        # if it's time to fetch next iteration
        if self.time_elapsed >= self.time_per_iter:
            self.time_elapsed = 0.
            if self.iter_cnt < self.iter_max:
                with open('./data/' + str(self.iter_cnt) + '.csv', 'r') as f:
                    reader = csv.reader(f)
                    for i, row in enumerate(reader):
                        for j, val in enumerate(row):
                            self.state[i][j] = float(val)
                self.iter_cnt += 1
            else:
                print("Simulation finished!")
                exit(0)
        else:
            self.state[:, :2] += dt * self.state[:, 2:]


uni = Universe()
dt = 1 / (uni.time_per_iter * 30)  # this controls the speed

# set up figure and animation
fig = plt.figure()
fig.subplots_adjust(left=0, right=1, bottom=0, top=1)
ax = fig.add_subplot(111, aspect='equal', autoscale_on=False,
                     xlim=(-uni.size, uni.size), ylim=(-uni.size, uni.size))

# planets holds the locations of the planets
planets, = ax.plot([], [], 'bo', ms=600)

# rect is the universe edge
rect = plt.Rectangle([-uni.size, -uni.size], 2 * uni.size, 2 *
                     uni.size, ec='none', lw=2, fc='none')
ax.add_patch(rect)


def init():
    """init animation"""
    global uni, rect
    planets.set_data([], [])
    rect.set_edgecolor('none')
    return planets, rect


def animate(i):
    """perform animation step"""
    global uni, rect, dt, ax, fig
    uni.step(dt)

    ms = int(fig.dpi * 2 * (uni.size / 160) * fig.get_figwidth() /
             np.diff(ax.get_xbound())[0])

    # update pieces of the animation
    rect.set_edgecolor('k')
    # print(uni.state)
    planets.set_data(uni.state[:, 0], uni.state[:, 1])
    planets.set_markersize(ms)
    return planets, rect


ani = animation.FuncAnimation(
    fig, animate, frames=600, interval=10, blit=True, init_func=init)

plt.show()
