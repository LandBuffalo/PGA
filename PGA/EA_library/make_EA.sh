g++ -c  GA.cc  
g++ -c  CEC2014.cc 

ar rvs  libGA.a GA.o
ar rvs  libCEC2014.a CEC2014.o

mv -f libGA.a ../
mv -f libCEC2014.a ../

rm -f *.o