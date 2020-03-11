all:
	cd build && make

reset_static: reset_static_books reset_static_catalogue
reset_static_books:
	rm /etc/clas-digital-devel/updated_static_books.txt
reset_static_catalogue:
	rm /etc/clas-digital-devel/updated_catalogue.txt


reset_static_stable: reset_static_books_stable reset_static_catalogue_stable
reset_static_books_stable:
	rm /etc/clas-digital/updated_static_books.txt
reset_static_catalogue_stable:
	rm /etc/clas-digital/updated_catalogue.txt



install:
	cd build && cmake --build . --target install

install_stable:
	cd build && rm CMakeCache.txt && cmake .. -DSTABLE_INSTALL=TRUE && cmake --build . && cmake --build . --target install && rm CMakeCache.txt 
