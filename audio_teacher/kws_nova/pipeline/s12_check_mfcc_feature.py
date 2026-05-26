import os
import sys
import numpy as np
from dsp_mfcc_feature import mfcc_feature_one_file

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage : python s11_mfcc_feature_file.py <pcm_file>")
        sys.exit(1)

    np.set_printoptions(suppress=True, threshold=np.inf, linewidth=10**6)
    teacher = mfcc_feature_one_file(sys.argv[1])

    student = np.fromfile(sys.argv[1], dtype=np.float32, offset=32000)
    a_1d = teacher.flatten()
    diff = a_1d - student
    max_dev = np.max(np.abs(diff))
    print(max_dev)