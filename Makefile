CC = g++
LIBDIR = -L/opt/local/lib
LIBS = -lboost_thread-mt -lboost_system-mt -lboost_thread-mt -lboost_date_time-mt

.PHONY: all clean serial_sync serial_async serial_save serial_read

all: serial_sync serial_async serial_save serial_read 

clean:
	rm -f serial_sync serial_sync.o
	rm -f serial_async serial_async.o
	rm -f serial_save serial_save.o
	rm -f serial_read serial_read.o

serial_sync: serial_sync.o
	$(CC) -O4 $(LIBDIR) $(LIBS) -o $@ $^

serial_async: serial_async.o
	$(CC) -O4 $(LIBDIR) $(LIBS) -o $@ $^

serial_save: serial_save.o
	$(CC) -O4 $(LIBDIR) $(LIBS) -o $@ $^

serial_read: serial_read.o
	$(CC) -O4 $(LIBDIR) $(LIBS) -o $@ $^

%.o: %.cpp
	$(CC) -Wall -c $<
