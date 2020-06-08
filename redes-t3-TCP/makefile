todo: servidor cliente
servidor: servidor.o
	g++ -Wall -o servidor servidor.o
servidor.o: servidor.c
	g++ -Wall -c servidor.c
cliente: cliente.o
	g++ -Wall -o cliente cliente.o
cliente.o: cliente.c
	g++ -Wall -c cliente.c
limpiar:
	rm -f *.o
	rm -f servidor
	rm -f cliente

