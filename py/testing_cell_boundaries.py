import numpy as np
import matplotlib.pyplot as plt
from matplotlib.path import Path
from matplotlib.patches import PathPatch



np.random.seed(0)

# [[x0, x1], [y0, y1]]
bounds = np.array([[-10, 10], [-8, 8]])
orig, ranges = bounds.T[0], bounds.T[1] - bounds.T[0]
N = 10
cells = np.array([np.random.rand(2) * ranges + orig for _ in range(N)])

plt.scatter(*cells.T, color='blue')

plt.show()
