// Redes de Computadoras
// Tecnologo en Informatica FIng - CETP
//
// Taller 3  - Programacion con Sockets TCP

#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <arpa/inet.h> // para inet_Addr, etc
#include <netdb.h> // estructuras
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h> 
#include <sys/wait.h>
#include <sys/signal.h>
#include <ctype.h> 

#define MAX_LARGO_MENSAJE 255
#define BACKLOG 2


using namespace std;

int fd1;    // File Descriptor Socket del servidor
int fd2;    // File Descriptor del socket luego de Accept

void resetString (char * & s)
// Resetea un string.
{
	s[0] = '\0';
}

char * agregarCero(char * cad,int num)
// Chequea si el num es < 10 y me devuelve un string con el '0'
// concatenado con num en dicho caso
{
	char * aux = new char[25];
	resetString(aux);
	strcat(aux,"0");
	sprintf(cad,"%d",num);
	if(num<10){
		strcat(aux,cad);
		return aux;
	}
	else{
		delete[] aux;
		return cad;
	}
}

char * getFecha()
// Obtiene la fecha y la almacena en un string
// con formato DD/MM/YYYY
{
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	int DD = timeinfo -> tm_mday;
	int MM = (timeinfo -> tm_mon) + 1; //xq enero = 0
	int YYYY = (timeinfo -> tm_year) + 1900 ; //xq el año es desde 1900
	char * s = new char[19];
	resetString(s);
	char * cad = new char[25];
	resetString(cad);
	//chequeo si alguna es menor q 10 para concatenarle un '0'
	cad = agregarCero(cad,DD);
	strcat(s,cad);
	strcat(s,"/");
	cad = agregarCero(cad,MM);
	strcat(s,cad);
	strcat(s,"/");
	cad = agregarCero(cad,YYYY);
	strcat(s,cad);
	strcat(s, "\r\n");
	delete [] cad;
	return s;
}

char * getHora()
// Obtiene la hora local y la almacena en un string
// con formato hh:mm:ss
{
	time_t rawtime;
	struct tm * timeinfo;
	time ( &rawtime );
	timeinfo = localtime ( &rawtime );
	int hh = timeinfo -> tm_hour;
	int mm = timeinfo -> tm_min;
	int ss = timeinfo -> tm_sec;
	char * s = new char[19];
	resetString(s);
	char * cad = new char[25];
	resetString(cad);
	//chequeo si alguna es menor q 10 para concatenarle un '0'
	cad = agregarCero(cad,hh);
	strcat(s,cad);
	strcat(s,":");
	cad = agregarCero(cad,mm);
	strcat(s,cad);
	strcat(s,":");
	cad = agregarCero(cad,ss);
	strcat(s,cad);
	strcat(s, "\r\n");
	delete [] cad;
	return s;
}

void manejadorSenhales (int signal)
// Manejador de las senhales.
// Aca se debe implementar la accion a realizar cuando se recibe la senhal
// Deberia haber un manejador de senhales para cada hijo si hacen cosas distintas
{
	if (signal == SIGINT)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGINT CTRL+C recibido\33[00m\n";
	}
	if (signal == SIGTERM)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGTERM Terminacion de programa\33[00m\n";
	}
	if (signal == SIGSEGV)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGSEGV violacion de segmento\33[00m\n";
	}
	if (signal == SIGCHLD)
	{
		cout << "\33[46m\33[31m[" << "]" << " SIGCHLD \33[00m\n";
	}
	if (signal == SIGPIPE)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGPIPE \33[00m\n";
	}
	if (signal == SIGKILL)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGKILL \33[00m\n";
	}
	if (signal == SIGALRM)
	{
		cout << "\33[46m\33[31m[" << getpid() << "]" << " SIGALRM Timeout excedido \33[00m\n";
	}
	close(fd1);
	close(fd2);
	exit(1);
}

int main(int argc, char * argv[])
// En argc viene la cantidad de argumentos que se pasan,
// si se llama solo al programa, el nombre es el argumento 0
// En nuestro caso:
//      - argv[0] es el string "servidor", puede cambiar si se llama con otro path.
//      - argv[1] es el puerto de escucha del servidor.

{

	if (argc < 2)
	{
			cout << "\33[46m\33[31m[ERROR]:" << " Faltan argumentos: port.\33[00m\n";
			exit (1);
	}


	// Estructuras para el manejo de Senhales
	// Deberia haber un manejador de senhales para cada hijo si hacen cosas distintas
	// *********************************
	struct sigaction sa;
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &manejadorSenhales;

	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGTERM, &sa, NULL);
	sigaction(SIGPIPE, &sa, NULL);
	sigaction(SIGSEGV, &sa, NULL);
	sigaction(SIGKILL, &sa, NULL);
	signal(SIGALRM, SIG_IGN);
	// **********************************

	cout << "\33[34mRedes de Computadoras\33[39m: Servidor Fecha/Hora\nEscuchando en el puerto " << argv[1] << ".\nProceso de pid: " << getpid() << ".\n";

	fd1 = socket(AF_INET, SOCK_STREAM, 0);

	if (fd1 < 0){
		cout << "\33[46m\33[31m[ERROR]: socket()\33[00m\n";
		exit (0);
	}

	struct sockaddr_in server; // donde se cargan los datos del servidor
	struct sockaddr_in client; // donde se cargan los datos del cliente que se conecte

	server.sin_family = AF_INET;              // Se cargan los datos del servidor, ip escucha, puerto.
	server.sin_port = htons(atoi(argv[1])); 
	server.sin_addr.s_addr = INADDR_ANY;
	bzero(&(server.sin_zero),8);


	int error = bind(fd1,(struct sockaddr*)&server, sizeof(struct sockaddr));
	if (error < 0){
		cout << "\33[46m\33[31m[ERROR]: bind()\33[00m\n";
		exit (0);
	}

	error = listen(fd1,BACKLOG);
	if (error < 0){
		printf("\33[46m\33[31m[ERROR]: listen()\33[00m\n");
		exit(0);
   	}

	char buf[MAX_LARGO_MENSAJE];
	int numbytes;
	while (true){ 
      	unsigned int sin_size = sizeof(struct sockaddr_in);
		fd2 = accept(fd1,(struct sockaddr *)&client, &sin_size);
		if (fd2 < 0){
			printf("\33[46m\33[31m[ERROR]: accept()\33[00m\n");
			exit(0);
		}
		printf(" conexion recibida desde > %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port)); 
		send(fd2,"Redes - Servidor Hora\r\n",23,0);
		cout << "    mensaje enviado > " << "Redes - Servidor Hora\r\n";
		//sleep(1);

		if ((numbytes = recv(fd2, buf, MAX_LARGO_MENSAJE, 0)) == -1){
			printf("\33[46m\33[31m[ERROR]: recv()\33[00m\n");
			exit(0);
		}
		buf[numbytes - 2]='\0';
		printf("    mensaje recibido < %s\n", buf); 
		if (strcmp (buf, "fecha") == 0){
			strcpy (buf, getFecha());
			send(fd2, buf, 12,0);
			cout << "    mensaje enviado > " << buf;
		}
		else if (strcmp (buf, "hora") == 0){
			strcpy (buf, getHora());
			send(fd2, buf, 10,0);
			cout << "    mensaje enviado > " << buf;
		}
		else{
			send(fd2,"ERROR: Comando o estado incorrecto.\r\n",37,0);
			cout << "    mensaje enviado > " << "ERROR: Comando o estado incorrecto.\r\n";
		}
		close(fd2);
	}
}
