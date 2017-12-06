# the compiler: gcc
CC = gcc

# compiler flags:

# -Wall turns on most, but not all, compiler warnings
CFLAGS = -W -Wall -g

# the build target executable:
TARGET = uspsv1 uspsv2 uspsv3 uspsv4
all: $(TARGET)


uspsv1: uspsv1.o p1fxns.o queue.o 
	$(CC) $(CFLAGS) -o uspsv1 uspsv1.o p1fxns.o queue.o
uspsv2: uspsv2.o p1fxns.o queue.o 
	$(CC) $(CFLAGS) -o uspsv2 uspsv2.o p1fxns.o queue.o 

uspsv3: uspsv3.o p1fxns.o queue.o 
	$(CC) $(CFLAGS) -o uspsv3 uspsv3.o p1fxns.o queue.o 

uspsv4: uspsv4.o p1fxns.o queue.o 
	$(CC) $(CFLAGS) -o uspsv4 uspsv4.o p1fxns.o queue.o 
# usps versions
uspsv1.o: uspsv1.c p1fxns.h
uspsv2.o: uspsv2.c p1fxns.h
uspsv3.o: uspsv3.c p1fxns.h
uspsv4.o: uspsv4.c p1fxns.h



# extra header library
p1fxns.o: p1fxns.c p1fxns.h
queue.o: queue.c queue.h


clean:
	$(RM) $(TARGET) *.o