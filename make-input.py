import sys
import random

n = int(sys.argv[1])
filename = sys.argv[2]
with open(filename, 'w') as f:
    f.writelines(str(random.randint(1, 10 ** 9)) + '\n' for _ in range(n))
    
# bash 
# python make-input.py 100000000 input-8.txt