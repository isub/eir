app_eir.fdx: dict_eir.o app_eir.o app_eir_server.o app_eir_blacklist.o
	g++ -shared -o app_eir.fdx  dict_eir.o app_eir.o app_eir_server.o app_eir_blacklist.o
dict_eir.o: dict_eir.c
	gcc -fPIC -c -Wall -Wextra dict_eir.c -lfdproto -lfdcore
app_eir.o: app_eir.c
	gcc -fPIC -c -Wall -Wextra app_eir.c -lfdproto -lfdcore
app_eir_server.o: app_eir_server.c
	gcc -fPIC -c -Wall -Wextra app_eir_server.c -lfdproto -lfdcore
app_eir_blacklist.o: app_eir_blacklist.cpp
	g++ -fPIC -c -Wall -Wextra app_eir_blacklist.cpp
install:
	sudo cp app_eir.fdx /usr/lib/freeDiameter
clean:
	rm -rf *.o *.fdx
