CC = g++
INCDIR = -I/opt/local/include
LIBDIR = -L/opt/local/lib
GLLIB = -framework GLUT -framework OpenGL
LIBS = -lboost_thread-mt -lboost_system-mt -lboost_thread-mt -lboost_date_time-mt

.PHONY: all clean serial_sync serial_async serial_save serial_read serial_show

all: serial_sync serial_async serial_save serial_read serial_show 

clean:
	rm -f serial_sync serial_sync.o
	rm -f serial_async serial_async.o
	rm -f serial_save serial_save.o
	rm -f serial_read serial_read.o
	rm -f serial_show serial_show.o

serial_sync: serial_sync.o
	$(CC) -O4 $(LIBDIR) $(LIBS) -o $@ $^

serial_async: serial_async.o
	$(CC) -O4 $(LIBDIR) $(LIBS) -o $@ $^

serial_save: serial_save.o
	$(CC) -O4 $(LIBDIR) $(LIBS) -o $@ $^

serial_read: serial_read.o
	$(CC) -O4 $(LIBDIR) $(LIBS) -o $@ $^

serial_show: serial_show.o
	$(CC) -O4 $(LIBDIR) $(LIBS) $(GLLIB) -o $@ $^

%.o: %.cpp
	$(CC) -Wall $(INCDIR) -c $<
