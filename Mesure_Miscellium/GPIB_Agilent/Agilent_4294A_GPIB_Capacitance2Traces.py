

import time
import pyvisa
import matplotlib.pyplot as plt

########################################################
rm = pyvisa.ResourceManager()
timedelay=15
pts=100

m4294a=rm.open_resource("GPIB::17")
m4294a.write("*IDN?")
print(m4294a.read())

m4294a.write("E4TP M1")	        # adapter
m4294a.write("MEAS CSR")        # type of measurement (p.36)
# m4294a.write("MEAS IMPH")	
m4294a.write("SWPP FREQ")	    # set sweep parameter (p.449)
m4294a.write("SWPT LOG")
m4294a.write("STAR 4E1")
m4294a.write("STOP 4E6")

#points
m4294a.write("POIN "+str(pts))
m4294a.write("AVERFACT 10")     # Sets the averaging factor of the sweep averaging function

timeref=time.strftime("%a%d%b%Y_%Hh%Mm%Ss", time.gmtime())
label=input("\n >>>>>> define label : ") 

m4294a.write("BWFACT 4")        # Sets the bandwidth
##m4294a.write("CONT")		    # continuous sweep
m4294a.write("TRAC A")		    # Sets the active trace
m4294a.write("SING")            # Performs a single sweep


ff=open('Mesure_freqCoupure/Electrode_Tiges/AvecMisc_apresColo/meas_'+label+'.txt','w')


time.sleep(timedelay)
m4294a.write("OUTPDTRC?")       # Reads out the values of all measurement points in a data trace array
valseq=str(m4294a.read())
ff.write(str(valseq)+'\n')

print(valseq)

m4294a.write("TRAC B")		
m4294a.write("SING")

ff.write("###############")
ff.write("\n")
ff.write("\n")

time.sleep(timedelay)
m4294a.write("OUTPDTRC?")
valseq=str(m4294a.read())
ff.write(str(valseq)+'\n')

print(valseq)

ff.close()
m4294a.close()   



