#include <mpi.h> 
#include <iostream>
#include <thread>
#include <chrono> 
#include <random>

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

	//Solo comenzará el juego en caso de que el nº de procesos sea par.
	if (_numProcs % 2 == 0) {
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
	} else {
		if (_processId == 0)
			std::cout << "No se puede comenzar el juego debido a que el numero de procesos es impar." << std::endl;
	}
	MPI_Finalize(); //Finalización OpenMPI
}

void Pr3(int argc, char* argv[]) {
	int _processId, _numProcs;
	unsigned _numPoints = 10000000; //nº de puntos a lanzar.
	unsigned _counter = 0; //nº de puntos en el interior de la circuferencia. 

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	//Generador de números aleatorios. Se utiliza una distribución uniforme.
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
	double _q = (double)_counter / (double)_numPoints; //Hacemos casteo a double para evitar división de enteros.

	//Creamos un buffer usando memoria dinámica para hacerlo adaptativo al nº de procesos.
	double* _buffer = new double[_numProcs];
	for (size_t i = 0; i < _numProcs; i++) {
		_buffer[i] = 0;
	}

	//Todos los procesos envian sus resultados al proceso 0, quien realizará la agregación.
	MPI_Gather(&_q, 1, MPI_DOUBLE, _buffer, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	//Se agregan los resultados de cada uno de los procesos y se obtiene el resultado final. 
	if (_processId == 0) {
		_q = 0;
		for (size_t i = 0; i < _numProcs; i++) {
			_q += _buffer[i];
		}
		_q *= 4 / (double)_numProcs;
		std::cout << "Aproximación de Pi: " << _q << std::endl;
	}

	MPI_Finalize(); //Finalización OpenMPI
}

void Pr4(int argc, char* argv[]) {
	int _processId, _numProcs;
	const int _numDatos = 100000; //Número de datos total a repartir entre todos los procesos.
	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	const int fixedValue = _numDatos + _numDatos % _numProcs; //Ajustamos el nº de valores en función al nº de procesos, de manera que todos los procesos reciban la misma cantidad de datos, aunque unos pocos sean nulos.
	const int dataPerProcess = fixedValue / _numProcs; //Calculamos cuantos datos se le envia a cada proceso.
	int* _partialBuffer = new int[dataPerProcess]; //Reservamos memoria en cada proceso para recibir los datos enviados por _processId==0.

	if (_processId == 0) {
		int* _buffer = new int[fixedValue];
		for (int i = 0; i < _numDatos; i++) {
			_buffer[i] = i; //Rellenamos el buffer de valores a enviar de manera secuencial. De esta manera sabemos que el valor final deberá de ser _numDatos-1
		}
		for (int i = _numDatos; i < fixedValue; i++) {
			_buffer[i] = INT32_MIN; //Se añaden los valores "nulos" al final del buffer. El valor nulo dependerá de la operación utilizada. En este caso es el valor MIN de int32. En caso de que la operación fuera suma, el nulo sería 0. 
		}
		MPI_Scatter(_buffer, dataPerProcess, MPI_INT, _partialBuffer, dataPerProcess, MPI_INT, 0, MPI_COMM_WORLD); //Repartimos los datos entre todos los procesos.
		delete[] _buffer;
	} else {
		MPI_Scatter(nullptr, 0, MPI_INT, _partialBuffer, dataPerProcess, MPI_INT, 0, MPI_COMM_WORLD); //Recibimos los datos en los procesos "trabajadores"
	}
	int _currentMax = INT32_MIN;
	for (int i = 0; i < dataPerProcess; i++) { //Calculamos el máximo del subconjunto de datos de este proceso. 
		if (_currentMax < _partialBuffer[i])
			_currentMax = _partialBuffer[i];
	}
	std::cout << "Soy el proceso " << _processId << " y mi maximo es " << _currentMax << std::endl;

	int _finalMax = 0;
	MPI_Reduce(&_currentMax, &_finalMax, 1, MPI_INT, MPI_MAX, 0, MPI_COMM_WORLD); //Enviamos los resultados obtenidos al proceso principal, quien realizará la reducción final. 

	MPI_Barrier(MPI_COMM_WORLD);

	if (_processId == 0)
		std::cout << "Soy el proceso " << _processId << " y el maximo final es " << _finalMax << std::endl;

	delete[] _partialBuffer;
	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioBarrier(int argc, char* argv[]) {
	int _processId, _numProcs;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	for (size_t i = 0; i < 2; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(_processId));
		std::cout << "SIN BARRIER. Soy el proceso " << _processId << ", Hola" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(_processId));
		std::cout << "SIN BARRIER. Soy el proceso " << _processId << ", Adios" << std::endl;
	}

	for (size_t i = 0; i < 2; i++) {
		std::this_thread::sleep_for(std::chrono::milliseconds(_processId));
		MPI_Barrier(MPI_COMM_WORLD);
		std::cout << "CON BARRIER. Soy el proceso " << _processId << ", Hola" << std::endl;
		std::this_thread::sleep_for(std::chrono::milliseconds(_processId));
		MPI_Barrier(MPI_COMM_WORLD);
		std::cout << "CON BARRIER. Soy el proceso " << _processId << ", Adios" << std::endl;
	}

	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioBroadcast(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	if (_processId == 0) {
		n = 100;
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
	} else {
		MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
		std::cout << "Soy el proceso " << _processId << " y recibo " << n << std::endl;
	}

	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioGather(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos
	int* _buffer = new int[_numProcs];
	for (size_t i = 0; i < _numProcs; i++) {
		_buffer[i] = 0;
	}
	MPI_Gather(&_processId, 1, MPI_INT, _buffer, 1, MPI_INT, 0, MPI_COMM_WORLD);

	std::cout << "Soy el proceso " << _processId << " y recibo ";
	for (size_t i = 0; i < _numProcs; i++) {
		std::cout << _buffer[i] << ", ";
	}
	std::cout << std::endl;
	delete[] _buffer;
	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioScatter(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos
	int* _buffer = new int[_numProcs];
	for (size_t i = 0; i < _numProcs; i++) {
		_buffer[i] = i;
	}
	MPI_Scatter(_buffer, 1, MPI_INT, &n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	std::cout << "Soy el proceso " << _processId << " y recibo " << n;

	std::cout << std::endl;
	delete[] _buffer;
	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioReduce(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	MPI_Reduce(&_processId, &n, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

	std::cout << "Soy el proceso " << _processId << " y recibo la suma " << n;

	std::cout << std::endl;
	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioAllGatherAlltoall(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos
	int* _bufferEnvio = new int[_numProcs];
	int* _bufferRecepcion = new int[_numProcs * _numProcs];
	for (size_t i = 0; i < _numProcs; i++) {
		_bufferEnvio[i] = i + 10 * _processId;
	}
	MPI_Allgather(_bufferEnvio, _numProcs, MPI_INT, _bufferRecepcion, _numProcs, MPI_INT, MPI_COMM_WORLD);

	std::cout << "Soy el proceso " << _processId << " y he recibido ";
	for (size_t i = 0; i < _numProcs * _numProcs; i++) {
		std::cout << _bufferRecepcion[i] << ", ";
	}
	std::cout << std::endl;

	MPI_Barrier(MPI_COMM_WORLD);

	MPI_Alltoall(_bufferEnvio, 1, MPI_INT, _bufferRecepcion, 1, MPI_INT, MPI_COMM_WORLD);

	std::cout << "Soy el proceso " << _processId << " y he recibido ";
	for (size_t i = 0; i < _numProcs; i++) {
		std::cout << _bufferRecepcion[i] << ", ";
	}

	std::cout << std::endl;
	delete[] _bufferEnvio;
	delete[] _bufferRecepcion;
	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioScan(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	MPI_Scan(&_processId, &n, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

	std::cout << "Soy el proceso " << _processId << " y recibo la suma " << n;

	std::cout << std::endl;
	MPI_Finalize(); //Finalización OpenMPI
}

void customOp(int* inVec, int* inOutVec, int* len, MPI_Datatype* dtype) {
	for (size_t i = 0; i < *len; i++) {
		inOutVec[i] = inOutVec[i] * 10 + inVec[i];
	}
}

void EjercicioOperacionCustom(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos
	MPI_Op myOp;
	MPI_Op_create((MPI_User_function*)customOp, false, &myOp);

	MPI_Reduce(&_processId, &n, 1, MPI_INT, myOp, 0, MPI_COMM_WORLD);

	if (_processId == 0)
		std::cout << "Soy el proceso " << _processId << " y he recibido en el reparto " << n << std::endl;

	MPI_Op_free(&myOp);

	MPI_Finalize(); //Finalización OpenMPI
}

int main(int argc, char* argv[]) {
	Pr4(argc, argv);

	return 0;
}