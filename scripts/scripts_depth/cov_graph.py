import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import re
import numpy as np
import sys

source_type=sys.argv[1]
f=open(source_type+'_'+source_type+'Coverage.csv','r')
each_line=[]
x_axis=np.arange(0,101,5)
fig= plt.figure()
ax=fig.add_subplot(111)
ax.set_xlabel('Depth')
ax.set_ylabel('Fraction of capture target bases >= depth')
for line in f:
  if re.search(source_type,line):
    each_line=[x.rstrip() for x in line.split(',')]
    y_axis=each_line[2:]
plt.plot(x_axis,y_axis)
plt.show()   

fig.savefig("graph.pdf",bbox_inches='tight')
