#include<mpi.h> 
#include <iostream>

void Pr1(int argc, char* argv[]) {
	int _processId, _numProcs;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	std::cout << "Hola mundo. Proceso " << _processId << std::endl;
	if (_processId == 0) //Hacemos que solo el proceso "principal" muestre el nº de procesos disponibles.
		std::cout << "Numero de procesos: " << _numProcs << std::endl;

	MPI_Finalize(); //Finalización OpenMPI
}

void Pr2(int argc, char* argv[]) {
	int _processId, _contrincante;
	int _pingPong = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual

	//Si el proceso es par, el contrincante será el id del proceso + 1. En caso contrario, id del proceso - 1
	_contrincante = _processId % 2 == 0 ? _processId + 1 : _processId - 1;

	//Mientras no se alcancen los 6 toques...
	while (_pingPong < 6) {
		std::cout << "Soy el proceso " << _processId << " y juego con el proceso " << _contrincante;
		//Lo que deberá hacer cada proceso dependerá de si ha comenzado el la partida y del turno por el que vaya.
		if ((_processId + _pingPong) % 2 == 0) {
			_pingPong++;
			//Incrementamos el valor de la variable _pingPong y enviamos el nuevo valor a nuestro contrincante. Se ha usado un Send bloqueante, aunque se podría cambiar por uno asincrono. 
			MPI_Send(&_pingPong, 1, MPI_INT, _contrincante, 0, MPI_COMM_WORLD); 
			std::cout << ". Envio " << _pingPong << std::endl;
		} else {
			//Almacenamos el valor recibido sobreescribiendo el valor de _pingPong. El Recv que se ha usado es bloqueante, por lo que el proceso se bloqueará en este punto hasta que reciba el mensaje. 
			MPI_Recv(&_pingPong, 1, MPI_INT, _contrincante, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::cout << ". Recibo " << _pingPong << std::endl;
		}
	}
	MPI_Finalize(); //Finalización OpenMPI
}

int main(int argc, char* argv[]) {
	Pr2(argc, argv);

	return 0;
}