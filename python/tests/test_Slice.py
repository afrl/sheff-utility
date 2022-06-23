import numpy as np

import utility

arr = np.array([0.]*10, 'd')
arr_slice = utility.slice(arr)
print('size = ', arr_slice.size())
