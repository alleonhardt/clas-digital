all:
	cd build && make

reset_static:
	rm /etc/clas-digital-devel/updated.txt

reset_static_stable:
	rm /etc/clas-digital/updated.txt
	
install:
	cd build && cmake --build . --target install

install_stable:
	cd build && rm CMakeCache.txt && cmake .. -DSTABLE_INSTALL=TRUE && cmake --build . && cmake --build . --target install && rm CMakeCache.txt 
