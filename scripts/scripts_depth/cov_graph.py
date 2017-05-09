import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import re

source_type='Blood'
f=open('Blood_BloodCoverage.csv','r')
each_line=[]
x_axis=[4,10,20,50,100]
fig= plt.figure()
for line in f:
	if re.search(source_type,line):
		each_line=[x.strip() for x in line.split(',')]
		y_axis=each_line[2:]
plt.plot(x_axis,y_axis)
plt.show()	 

fig.savefig("graph.pdf",bbox_inches='tight')
