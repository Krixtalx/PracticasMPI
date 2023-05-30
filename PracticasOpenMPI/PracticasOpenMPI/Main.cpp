#include <mpi.h> 
#include <iostream>
#include <thread>
#include <chrono> 
#include <random>
#include "EjerciciosClase.h"

void Pr1(int argc, char* argv[]) {
	int _processId, _numProcs;

	MPI_Init(&argc, &argv); //Inicializaci�n OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N� de procesos

	std::cout << "Hola mundo. Proceso " << _processId << std::endl;
	if (_processId == 0) //Hacemos que solo el proceso "principal" muestre el n� de procesos disponibles.
		std::cout << "Numero de procesos: " << _numProcs << std::endl;

	MPI_Finalize(); //Finalizaci�n OpenMPI
}

void Pr2(int argc, char* argv[]) {
	int _processId, _numProcs, _contrincante;
	int _pingPong = 0;

	MPI_Init(&argc, &argv); //Inicializaci�n OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N� de procesos

	//Solo comenzar� el juego en caso de que el n� de procesos sea par.
	if (_numProcs % 2 == 0) {
		//Si el proceso es par, el contrincante ser� el id del proceso + 1. En caso contrario, id del proceso - 1
		_contrincante = _processId % 2 == 0 ? _processId + 1 : _processId - 1;

		//Mientras no se alcancen los 6 toques...
		while (_pingPong < 6) {
			std::cout << "Soy el proceso " << _processId << " y juego con el proceso " << _contrincante;
			//Lo que deber� hacer cada proceso depender� de si ha comenzado el la partida y del turno por el que vaya.
			if ((_processId + _pingPong) % 2 == 0) {
				_pingPong++;
				//Incrementamos el valor de la variable _pingPong y enviamos el nuevo valor a nuestro contrincante. Se ha usado un Send bloqueante, aunque se podr�a cambiar por uno asincrono. 
				MPI_Send(&_pingPong, 1, MPI_INT, _contrincante, 0, MPI_COMM_WORLD);
				std::cout << ". Envio " << _pingPong << std::endl;
			} else {
				//Almacenamos el valor recibido sobreescribiendo el valor de _pingPong. El Recv que se ha usado es bloqueante, por lo que el proceso se bloquear� en este punto hasta que reciba el mensaje. 
				MPI_Recv(&_pingPong, 1, MPI_INT, _contrincante, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				std::cout << ". Recibo " << _pingPong << std::endl;
			}
		}
	} else {
		if (_processId == 0)
			std::cout << "No se puede comenzar el juego debido a que el numero de procesos es impar." << std::endl;
	}
	MPI_Finalize(); //Finalizaci�n OpenMPI
}

void Pr3(int argc, char* argv[]) {
	int _processId, _numProcs;
	unsigned _numPoints = 10000000; //n� de puntos a lanzar.
	unsigned _counter = 0; //n� de puntos en el interior de la circuferencia. 

	MPI_Init(&argc, &argv); //Inicializaci�n OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N� de procesos

	//Generador de n�meros aleatorios. Se utiliza una distribuci�n uniforme.
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(-1, 1);

	double _x, _y, _squareDistance;
	for (size_t i = 0; i < _numPoints; i++) {
		_x = dis(gen);
		_y = dis(gen);
		_squareDistance = _x * _x + _y * _y; //Dado que el radio de la circuferencia es 1, no es necesario hacer la raiz cuadrada.
		if (_squareDistance <= 1)
			_counter++;
	}
	double _q = (double)_counter / (double)_numPoints; //Hacemos casteo a double para evitar divisi�n de enteros.

	//Creamos un buffer usando memoria din�mica para hacerlo adaptativo al n� de procesos.
	double* _buffer = new double[_numProcs];
	for (size_t i = 0; i < _numProcs; i++) {
		_buffer[i] = 0;
	}

	//Todos los procesos envian sus resultados al proceso 0, quien realizar� la agregaci�n.
	MPI_Gather(&_q, 1, MPI_DOUBLE, _buffer, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	//Se agregan los resultados de cada uno de los procesos y se obtiene el resultado final. 
	if (_processId == 0) {
		_q = 0;
		for (size_t i = 0; i < _numProcs; i++) {
			_q += _buffer[i];
		}
		_q *= 4 / (double)_numProcs;
		std::cout << "Aproximaci�n de Pi: " << _q << std::endl;
	}

	MPI_Finalize(); //Finalizaci�n OpenMPI
}

void Pr4(int argc, char* argv[]) {
	int _processId, _numProcs;
	const int _numDatos = 100000; //N�mero de datos total a repartir entre todos los procesos.
	MPI_Init(&argc, &argv); //Inicializaci�n OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N� de procesos

	const int fixedValue = _numDatos + _numDatos % _numProcs; //Ajustamos el n� de valores en funci�n al n� de procesos, de manera que todos los procesos reciban la misma cantidad de datos, aunque unos pocos sean nulos.
	const int dataPerProcess = fixedValue / _numProcs; //Calculamos cuantos datos se le envia a cada proceso.
	int* _partialBuffer = new int[dataPerProcess]; //Reservamos memoria en cada proceso para recibir los datos enviados por _processId==0.

	if (_processId == 0) {
		int* _buffer = new int[fixedValue];
		for (int i = 0; i < _numDatos; i++) {
			_buffer[i] = i; //Rellenamos el buffer de valores a enviar de manera secuencial. De esta manera sabemos que el valor final deber� de ser _numDatos-1
		}
		for (int i = _numDatos; i < fixedValue; i++) {
			_buffer[i] = INT32_MIN; //Se a�aden los valores "nulos" al final del buffer. El valor nulo depender� de la operaci�n utilizada. En este caso es el valor MIN de int32. En caso de que la operaci�n fuera suma, el nulo ser�a 0. 
		}
		MPI_Scatter(_buffer, dataPerProcess, MPI_INT, _partialBuffer, dataPerProcess, MPI_INT, 0, MPI_COMM_WORLD); //Repartimos los datos entre todos los procesos.
		delete[] _buffer;
	} else {
		MPI_Scatter(nullptr, 0, MPI_INT, _partialBuffer, dataPerProcess, MPI_INT, 0, MPI_COMM_WORLD); //Recibimos los datos en los procesos "trabajadores"
	}
	int _currentMax = INT32_MIN;
	for (int i = 0; i < dataPerProcess; i++) { //Calculamos el m�ximo del subconjunto de datos de este proceso. 
		if (_currentMax < _partialBuffer[i])
			_currentMax = _partialBuffer[i];
	}
	std::cout << "Soy el proceso " << _processId << " y mi maximo es " << _currentMax << std::endl;

	int _finalMax = 0;
	MPI_Reduce(&_currentMax, &_finalMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD); //Enviamos los resultados obtenidos al proceso principal, quien realizar� la reducci�n final. 

	MPI_Barrier(MPI_COMM_WORLD);

	if (_processId == 0)
		std::cout << "Soy el proceso " << _processId << " y el maximo final es " << _finalMax << std::endl;

	delete[] _partialBuffer;
	MPI_Finalize(); //Finalizaci�n OpenMPI
}

void Pr5(int argc, char* argv[]){
	int _processId, _numProcs;
	int matrixADim[]={3,3};
	int matrixBDim[]={3,2};

	int matrixA[]={	3, 2, 1,
					1, 1, 3,
					0, 2, 1};

	int matrixB[]={	2, 1,
					1, 0,
					3, 2};

	MPI_Init(&argc, &argv); //Inicializaci�n OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N� de procesos


}

int main(int argc, char* argv[]) {
	EjercicioIntercomunicadores(argc, argv);

	return 0;
}