all: driver lfs

driver: driver.o
	g++ -o driver driver.o

driver.o: driver.cpp
	g++ -c driver.cpp

lfs: lfs.o
	g++ -o lfs lfs.o

lfs.o: lfs.cpp
	g++ -c lfs.cpp

clean:
	rm driver.o
	rm driver
	rm lfs.o
	rm lfs
