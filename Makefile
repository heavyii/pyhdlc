#compile so file for python
 
CC = gcc
CFLAG = -Wall -o2
INCLUDE = -I/usr/include/python2.7/
 
all: pyhdlc.so
 
%.o:%.c
	${CC} ${CFLAG} ${INCLUDE} -fpic -c $< -o $@
 
pyhdlc_obj=pyhdlc.o hdlc.o
pyhdlc.so: ${pyhdlc_obj}
	${CC} ${CFLAG} -shared $^ -o $@
 
.PYONY: clean
clean:
	rm *.so *.o
