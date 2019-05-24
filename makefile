run:
	cd build && cmake && make -j 4
	bin/clas-digital.o

Sserver:
	screen -S server
Cserver:
	screen -r server
