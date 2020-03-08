TARGET = idana
 
SRCS = $(TARGET).cc FilePath.cc CountManager.cc
OBJS = $(TARGET).o  FilePath.o  CountManager.o
 
ROOTCFLAGS = $(shell root-config --cflags)
# ROOTCFLAGS = -pthread -std=c++11 -m64 -I/opt/root-6.20.00/install/include
ROOTLIBS   = $(shell root-config --libs)
# ROOTLIBS   = -L/opt/root-6.20.00/install/lib -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -pthread -lm -ldl -rdynamic
ROOTGLIBS = $(shell root-config --glibs)
# ROOTGLIBS = -L/opt/root-6.20.00/install/lib -lGui -lCore -lImt -lRIO -lNet -lHist -lGraf -lGraf3d -lGpad -lROOTVecOps -lTree -lTreePlayer -lRint -lPostscript -lMatrix -lPhysics -lMathCore -lThread -lMultiProc -lROOTDataFrame -pthread -lm -ldl -rdynamic

CXXFLAGS   = $(ROOTCFLAGS) -Wall -fPIC
CXXLIBS    = $(ROOTLIBS)
CC = g++ 
 
$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(CXXLIBS) $(CXXFLAGS)  -o $@

# suffix rule
.cc.o:
	$(CC) $(CXXLIBS) $(CXXFLAGS) -c $<

# clean
clean:
	rm -f $(TARGET) $(OBJS)
