B= -Wall
all: clean redes-auth
redes-auth: mensajeria.c
	g++ $(B) -o mensajeria mensajeria.c
clean:
	rm -f *.o
	rm -f mensajeria