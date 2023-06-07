#include <mpi.h> 
#include <iostream>
#include <thread>
#include <chrono> 
#include <iomanip>
#include <random>
#include <filesystem>
#include <fstream>
#include <map>
#include <set>
#include <string>

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
	constexpr int _numDatos = 100000; //N�mero de datos total a repartir entre todos los procesos.
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

void printMatrix(const int* matrix, const int numCol, const int numRows) {
	for (int cols = 0; cols < numCol; ++cols) {
		for (int rows = 0; rows < numRows; ++rows) {
			std::cout << std::setw(3);
			std::cout << matrix[cols * numRows + rows] << " ";
		}
		std::cout << std::endl;
	}
}

void Pr5(int argc, char* argv[]) {
	int _processId, _numProcs;
	constexpr int matrixADim[] = { 3,3 };
	constexpr int matrixBDim[] = { 3,2 };
	constexpr int dimFinalMatrix = matrixADim[0] * matrixBDim[1];
	constexpr int multSize = matrixADim[1] + matrixBDim[0];
	const int matrixA[] = { 3, 2, 1,
							1, 1, 3,
							0, 2, 1 };

	const int matrixB[] = { 2, 1,
							1, 0,
							3, 2, };

	int receiveBuffer[multSize];

	MPI_Init(&argc, &argv); //Inicializaci�n OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N� de procesos

	if (_numProcs < dimFinalMatrix) { //Una limitación de esta implementación es que es necesario tener el mismo número de procesos que el tamaño final de la matriz.
		if (_processId == 0)
			std::cout << "Numero de procesos insuficiente" << std::endl;
	} else {
		if (_processId == 0) {
			//Proceso principal
			const auto packedMatrix = new int[dimFinalMatrix * multSize]; //Creamos una matriz nueva donde agruparemos en cada "fila" lo que se enviará a cada proceso.
			int packedMatrixIndex = 0;
			for (int matrixAi = 0; matrixAi < matrixADim[0]; ++matrixAi) {
				for (int matrixBi = 0; matrixBi < matrixBDim[1]; ++matrixBi) {
					for (int matrixAj = 0; matrixAj < matrixADim[1]; ++matrixAj) {
						packedMatrix[packedMatrixIndex++] = matrixA[matrixAi * matrixADim[0] + matrixAj];
					}
					for (int matrixBj = 0; matrixBj < matrixBDim[0]; ++matrixBj) {
						packedMatrix[packedMatrixIndex++] = matrixB[matrixBi + matrixBj * matrixBDim[1]];
					}
				}
			}
			MPI_Scatter(packedMatrix, multSize, MPI_INT, &receiveBuffer, multSize, MPI_INT, 0, MPI_COMM_WORLD); //Enviamos a cada proceso los datos que necesita para realizar la multiplicación
			delete[] packedMatrix;
		} else {
			MPI_Scatter(nullptr, 0, MPI_INT, &receiveBuffer, multSize, MPI_INT, 0, MPI_COMM_WORLD); //Recibimos los datos en los procesos secundarios.
		}

		int result = 0;
		for (int i = 0; i < matrixADim[1]; ++i) {
			result += receiveBuffer[i] * receiveBuffer[i + matrixADim[1]]; //Calculamos el resultado parcial
		}

		if (_processId == 0) {
			int finalMatrix[dimFinalMatrix];
			MPI_Gather(&result, 1, MPI_INT, finalMatrix, 1, MPI_INT, 0, MPI_COMM_WORLD); //Recibimos los resultados parciales de cada una de los procesos
			std::cout << "Resultado de la multiplicacion de matrices: " << std::endl;
			printMatrix(finalMatrix, matrixADim[0], matrixBDim[1]); //Mostramos por pantalla el resultado final
		} else {
			MPI_Gather(&result, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD); //Enviamos al proceso principal el resultado parcial calculado.
		}
	}

	MPI_Finalize(); //Finalizaci�n OpenMPI
}

void Pr6(int argc, char* argv[]) {
	int _processId, _numProcs;
	constexpr int dim[] = { 4,4 };
	constexpr int periods[] = { true, true };
	int numAEnviar = 10; //Indica la cantidad de numeros aleatorios que enviarán los procesos de las filas 0. 
	int coords[2];
	int n = 0;
	int n2 = 0;
	MPI_Init(&argc, &argv); //Inicializaci�n OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N� de procesos
	srand(_processId);

	MPI_Comm cartComm;
	MPI_Cart_create(MPI_COMM_WORLD, 2, dim, periods, true, &cartComm);
	MPI_Cart_coords(cartComm, _processId, 2, coords);

	if (coords[1] == 1 || coords[1] == 2)
		numAEnviar++; //Ajustamos el nº de iteraciones de las filas intermedias, que deben de realizar una mas

	for (int i = 0; i < numAEnviar; ++i) {
		switch (coords[1]) {
		case 0:
			n = rand() % 20 + 1;
			n += n % 2;
			MPI_Send(&n, 1, MPI_INT, _processId + 1, 0, cartComm); //Los procesos de la fila 0 solo envian el nº aleatorio generado. 
			std::cout << "It " << i << ": " << "Soy el proceso " << _processId << " en fila " << coords[1] << " y columna " << coords[0] << " y envio " << n << " al proceso " << _processId + 1 << std::endl;
			break;
		case 1:
			if (i == 0) { //Los procesos de la fila 1 y 2 solo reciben en la primera it
				MPI_Recv(&n, 1, MPI_INT, _processId - 1, 0, cartComm, MPI_STATUS_IGNORE);
				std::cout << "It " << i << ": " << "Soy el proceso " << _processId << " en fila " << coords[1] << " y columna " << coords[0] << " y recibo " << n << std::endl;
			} else if (i < numAEnviar - 1) { //Los procesos de la fila 1 y 2 reciben y envian en las it intermedias
				n2 = n / 2;
				MPI_Sendrecv(&n2, 1, MPI_INT, _processId + 1, 0, &n, 1, MPI_INT, _processId - 1, 0, cartComm, MPI_STATUS_IGNORE);
				std::cout << "It " << i << ": " << "Soy el proceso " << _processId << " en fila " << coords[1] << " y columna " << coords[0] << ". Recibo " << n << " y envio " << n2 << " al proceso " << _processId + 1 << std::endl;
			} else { //Los procesos de la fila 1 y 2 solo envian en la última it
				n2 = n / 2;
				MPI_Send(&n2, 1, MPI_INT, _processId + 1, 0, cartComm);
				std::cout << "It " << i << ": " << "Soy el proceso " << _processId << " en fila " << coords[1] << " y columna " << coords[0] << " y envio " << n2 << " al proceso " << _processId + 1 << std::endl;
			}
			break;
		case 2:
			if (i == 0) {//Los procesos de la fila 1 y 2 solo reciben en la primera it
				MPI_Recv(&n, 1, MPI_INT, _processId - 1, 0, cartComm, MPI_STATUS_IGNORE);
				std::cout << "It " << i << ": " << "Soy el proceso " << _processId << " en fila " << coords[1] << " y columna " << coords[0] << " y recibo " << n << std::endl;
			} else if (i < numAEnviar - 1) {//Los procesos de la fila 1 y 2 reciben y envian en las it intermedias
				n2 = n + 100;
				MPI_Sendrecv(&n2, 1, MPI_INT, _processId + 1, 0, &n, 1, MPI_INT, _processId - 1, 0, cartComm, MPI_STATUS_IGNORE);
				std::cout << "It " << i << ": " << "Soy el proceso " << _processId << " en fila " << coords[1] << " y columna " << coords[0] << ". Recibo " << n << " y envio " << n2 << " al proceso " << _processId + 1 << std::endl;
			} else {//Los procesos de la fila 1 y 2 solo envian en la última it
				n2 = n + 100;
				MPI_Send(&n2, 1, MPI_INT, _processId + 1, 0, cartComm);
				std::cout << "It " << i << ": " << "Soy el proceso " << _processId << " en fila " << coords[1] << " y columna " << coords[0] << " y envio " << n2 << " al proceso " << _processId + 1 << std::endl;
			}
			break;
		case 3:
			//Los procesos de la fila 3 solo reciben el dato y muestran el resultado final.
			MPI_Recv(&n, 1, MPI_INT, _processId - 1, 0, cartComm, MPI_STATUS_IGNORE);
			n2 = n * 2;
			std::cout << "It " << i << ": " << "Soy el proceso " << _processId << " en fila " << coords[1] << " y columna " << coords[0] << ". Recibo " << n << " y doy como resultado final " << n2 << std::endl;
			break;
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		if (_processId == 1) {
			std::cout << std::endl << "Nuevo ciclo" << std::endl;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	MPI_Finalize(); //Finalizaci�n OpenMPI
}

void Pr7(int argc, char* argv[]) {
	int _processId, _numProcs;
	constexpr int _numDatos = 100000; //Tam del buffer a enviar
	int _buffer[_numDatos];

	MPI_Init(&argc, &argv); //Inicializaci�n OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N� de procesos
	if (_processId == 0) {
		std::cout << "Tam del buffer " << _numDatos << std::endl;
	}
	std::this_thread::sleep_for(std::chrono::milliseconds(20));
	if (argc == 2) {
		const int op = atoi(argv[1]);
		//Dependiendo del parámetro especificado como argumento, se realizará una operación u otra. 
		switch (op) {
		case 0:
			std::cout << "Soy el proceso " << _processId << " y voy a enviar el buffer a " << (_processId + 1) % _numProcs << " en el modo Ssend" << std::endl;
			_buffer[0] = _processId;
			MPI_Ssend(_buffer, _numDatos, MPI_INT, (_processId + 1) % _numProcs, 0, MPI_COMM_WORLD);
			MPI_Recv(_buffer, _numDatos, MPI_INT, _processId != 0 ? (_processId - 1) % _numProcs : _numProcs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

			std::cout << "Soy el proceso " << _processId << " y he recibido " << _buffer[0] << std::endl;
			break;
		case 1:
			std::cout << "Soy el proceso " << _processId << " y voy a enviar el buffer a " << (_processId + 1) % _numProcs << " en el modo Rsend" << std::endl;
			_buffer[0] = _processId;
			MPI_Rsend(_buffer, _numDatos, MPI_INT, (_processId + 1) % _numProcs, 0, MPI_COMM_WORLD);
			MPI_Recv(_buffer, _numDatos, MPI_INT, _processId != 0 ? (_processId - 1) % _numProcs : _numProcs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::cout << "Soy el proceso " << _processId << " y he recibido " << _buffer[0] << std::endl;
			break;
		case 2:
		{
			std::cout << "Soy el proceso " << _processId << " y voy a enviar el buffer a " << (_processId + 1) % _numProcs << " en el modo Bsend" << std::endl;
			_buffer[0] = _processId;
			int size = _numDatos + static_cast<int>((MPI_BSEND_OVERHEAD / sizeof(int)));
			const auto _bsendBuffer = new int[size];
			size *= sizeof(int);
			MPI_Buffer_attach(_bsendBuffer, size);
			MPI_Bsend(_buffer, _numDatos, MPI_INT, (_processId + 1) % _numProcs, 0, MPI_COMM_WORLD);
			MPI_Recv(_buffer, _numDatos, MPI_INT, _processId != 0 ? (_processId - 1) % _numProcs : _numProcs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::cout << "Soy el proceso " << _processId << " y he recibido " << _buffer[0] << std::endl;
			MPI_Buffer_detach(_bsendBuffer, &size);
			delete[] _bsendBuffer;
		}
		break;
		case 3:
			std::cout << "Soy el proceso " << _processId << " y voy a enviar el buffer a " << (_processId + 1) % _numProcs << " en el modo Send" << std::endl;
			_buffer[0] = _processId;
			MPI_Send(_buffer, _numDatos, MPI_INT, (_processId + 1) % _numProcs, 0, MPI_COMM_WORLD);
			MPI_Recv(_buffer, _numDatos, MPI_INT, _processId != 0 ? (_processId - 1) % _numProcs : _numProcs - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			std::cout << "Soy el proceso " << _processId << " y he recibido " << _buffer[0] << std::endl;
			break;
		default:
			std::cout << "No se ha especificado un modo de operación válido en el argumento" << std::endl;
			break;
		}
	} else {
		std::cout << "No se ha especificado el modo de operación en el argumento" << std::endl;
	}

	MPI_Finalize(); //Finalizaci�n OpenMPI
}

//Estructura para almacenar los datos de las películas
struct puntuacionFinal {
	int _filmId = 0;
	mutable int _numVal = 0;
	mutable int _valSum = 0;

	float getMean() const {
		if (_numVal == 0)
			return 0;
		return static_cast<float>(_valSum) / static_cast<float>(_numVal);
	}

	friend bool operator < (const puntuacionFinal& a, const puntuacionFinal& b) {
		return a._filmId < b._filmId;
	}
};

void Pr8(int argc, char* argv[]) {
	int _processId, _numProcs;
	MPI_File _dataFile;
	MPI_Offset _fileSize;
	unsigned _lineSize = 0;
	unsigned _numLineas = 0;
	std::set<puntuacionFinal> _valPeliculas;
	const std::string _fileName = "practica8_data.txt";

	MPI_Init(&argc, &argv); //Inicializacion OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // N de procesos

	//Hacemos que el fichero principal examine la primera linea del fichero para obtener parámetros necesarios como la longitud de linea. 
	if (_processId == 0) {
		std::ifstream input(_fileName, std::ios::binary);
		if (input.good()) {
			std::string line;
			std::getline(input, line, '\r'); //Se usa '\r' dado que el fichero ha sido creado con un macintosh que utiliza CR para el salto de línea. 
			_lineSize = line.length() + 1;
		}
		input.close();
		std::cout << "[Proceso " << _processId << "]: El tam de linea es de " << _lineSize << " caracteres" << std::endl;
	}
	MPI_Bcast(&_lineSize, 1, MPI_INT, 0, MPI_COMM_WORLD); //Se envia el tamaño de linea a cada proceso 

	//Abrimos el fichero en modo lectura. 
	MPI_File_open(MPI_COMM_WORLD, _fileName.c_str(), MPI_MODE_RDONLY, MPI_INFO_NULL, &_dataFile);
	MPI_File_get_size(_dataFile, &_fileSize);
	//Calculamos el número de lineas en función al tamaño total del fichero y al tamaño de cada linea. 
	_numLineas = _fileSize / (_lineSize * sizeof(char) * _numProcs);
	if (_processId == 0)
		std::cout << "[Proceso " << _processId << "]: El fichero abierto tiene un tam de " << _fileSize << " bytes" << std::endl;
	std::cout << "[Proceso " << _processId << "]: Procesando " << _numLineas << " lineas..." << std::endl;

	//Creamos los tipo de datos necesarios para que ajustar el offset de cada proceso, de manera que cada proceso lea lineas distintas sin solaparse.
	MPI_Datatype line;
	MPI_Datatype readCycle;
	MPI_Aint lb;
	MPI_Aint extent;
	MPI_Type_contiguous(_lineSize, MPI_CHAR, &line);
	MPI_Type_commit(&line);
	MPI_Type_get_extent(line, &lb, &extent);
	MPI_Type_create_resized(line, 0, _numProcs * extent, &readCycle);
	MPI_Type_commit(&readCycle);

	//Crearemos una vista para que cada proceso lea una linea distinta.
	MPI_File_set_view(_dataFile, _processId * extent, line, readCycle, "native", MPI_INFO_NULL);

	//Creamos la estructura de datos donde cargar todas las lineas que deba leer el proceso.
	const int numChar = (_lineSize * _numLineas);
	const auto text = new char[numChar];
	//Leemos el fichero completamente.
	MPI_File_read(_dataFile, text, numChar, MPI_CHAR, MPI_STATUS_IGNORE);

	//Procesamos el fichero completo almacenando los datos formateados y agregados en un set.
	for (int i = 0; i < _numLineas; ++i) {
		const int offset = i * _lineSize;
		std::string aux;
		/*aux.assign(text + offset, text + offset + 5);
		const int userId = std::stoi(aux);*/
		aux.assign(text + offset + 5, text + offset + 10);
		const int filmId = std::stoi(aux);
		aux.assign(text + offset + 10, text + offset + 12);
		const int val = std::stoi(aux);
		//std::cout << "Usuario " << userId << " - Pelicula " << filmId << " - Val " << val << std::endl;
		auto vec = _valPeliculas.find({ filmId });
		if (vec != _valPeliculas.end()) {
			vec->_numVal++;
			vec->_valSum += val;
		} else {
			puntuacionFinal p{ filmId, 1, val };
			_valPeliculas.insert(p);
		}
	}
	std::cout << "[Proceso " << _processId << "]: Procesamiento de texto finalizado" << std::endl;

	//Realizamos una transformación a vector para poder enviarlo con OpenMPI.
	std::vector<puntuacionFinal> resultadosParciales;
	resultadosParciales.reserve(_valPeliculas.size());
	for (const auto& val_pelicula : _valPeliculas) {
		resultadosParciales.emplace_back(val_pelicula);
		//std::cout << "Pelicula " << val_pelicula._filmId << " - Val " << val_pelicula.getMean() << std::endl;
	}

	//Calculamos el tam maximo de resultados parciales para poder añadir resultados "nulos" en los procesos con menor num de resultados.
	int auxSize = resultadosParciales.size();
	int maxResultSize = 0;
	MPI_Allreduce(&auxSize, &maxResultSize, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);

	for (int i = auxSize; i < maxResultSize; ++i) {
		resultadosParciales.push_back({});
	}

	if (_processId == 0) {
		std::vector<puntuacionFinal> resultados;
		//Reservamos memoria suficiente para recibir todos los resultados
		resultados.resize(maxResultSize * _numProcs);
		std::cout << "[Proceso " << _processId << "]: Recibiendo datos..." << std::endl;
		//Usamos el gather para recibir el gather de todos los procesos. Calculamos el tam de bytes a enviar respecto a int. De esta manera nos ahorramos tener que declarar un tipo de datos OpenMPI.
		MPI_Gather(resultadosParciales.data(), resultadosParciales.size() * sizeof(puntuacionFinal) / sizeof(int), MPI_INT, resultados.data(), maxResultSize * sizeof(puntuacionFinal) / sizeof(int), MPI_INT, 0, MPI_COMM_WORLD);
		//Reutilizamos el set para agregar los resultados parciales de cada proceso.
		_valPeliculas.clear();
		for (const auto& resultado : resultados) {
			auto vec = _valPeliculas.find(resultado);
			if (vec != _valPeliculas.end()) {
				vec->_numVal += resultado._numVal;
				vec->_valSum += resultado._valSum;
			} else {
				_valPeliculas.insert(resultado);
			}
			//std::cout << "Pelicula " << resultado._filmId << " - Val " << resultado.getMean() << std::endl;
		}
		//Transformamos a vector para poder usar la función sort. 
		std::vector<puntuacionFinal> resultadosFinales;
		resultadosFinales.reserve(_valPeliculas.size());
		for (const auto& val_pelicula : _valPeliculas) {
			resultadosFinales.emplace_back(val_pelicula);
			//std::cout << "Pelicula " << val_pelicula._filmId << " - Val " << val_pelicula.getMean() << std::endl;
		}
		//Ordenamos en función a la media
		std::sort(resultadosFinales.begin(), resultadosFinales.end(),
			[](const puntuacionFinal& a, const puntuacionFinal& b) {
				return a.getMean() > b.getMean();
			});
		//Mostramos por pantalla las 10 películas con mejores puntuaciones que superen las 20 valoraciones.
		int fallos = 0;
		for (int i = 0; i < 10; ++i) {
			if (resultadosFinales[i + fallos]._numVal < 20) {
				i--;
				fallos++;
			} else
				std::cout << "Puesto " << i + 1 << ": Pelicula " << resultadosFinales[i + fallos]._filmId << ", valorada " << resultadosFinales[i + fallos]._numVal << " veces. Valoracion media: " << resultadosFinales[i + fallos].getMean() << std::endl;
		}
	} else {
		std::cout << "[Proceso " << _processId << "]: Realizando envio de datos al proceso principal..." << std::endl;
		//Enviamos los resultados parciales al proceso principal, quien realizará la agregacion final y mostrara los resultados por pantalla.
		MPI_Gather(resultadosParciales.data(), resultadosParciales.size() * sizeof(puntuacionFinal) / sizeof(int), MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);
	}
	delete[] text;
	MPI_File_close(&_dataFile);
	MPI_Finalize(); //Finalizaci�n OpenMPI
}

int main(int argc, char* argv[]) {
	Pr8(argc, argv);

	return 0;
}