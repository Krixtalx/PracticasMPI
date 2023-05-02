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
	int _processId, _numProcs, _contrincante;
	int _pingPong = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	_contrincante = _processId % 2 == 0 ? _processId + 1 : _processId - 1;

	while (_pingPong < 6) {
		if ((_processId + _pingPong) % 2 == 0) {
			_pingPong++;
			MPI_Send(&_pingPong, 1, MPI_INT, _contrincante, 0, MPI_COMM_WORLD);
		} else {
			MPI_Recv(&_pingPong, 1, MPI_INT, _contrincante, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::cout << "Soy el proceso " << _processId << " y juego con el proceso " << _contrincante << ". Recibo " << _pingPong << " y enviare " << _pingPong + 1 << std::endl;
		}
	}
	MPI_Finalize(); //Finalización OpenMPI
}

int main(int argc, char* argv[]) {
	Pr2(argc, argv);

	return 0;
}