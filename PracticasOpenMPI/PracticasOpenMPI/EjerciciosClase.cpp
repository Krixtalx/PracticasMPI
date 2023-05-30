#include <mpi.h> 
#include <iostream>
#include <thread>
#include <chrono> 
#include <random>
#include "EjerciciosClase.h"

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

void EjercicioTypeVector(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;
	int sendData[18], receiveData[6];

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos
	MPI_Datatype tvector;
	MPI_Type_vector(3, 2, 6, MPI_INT, &tvector);
	MPI_Type_commit(&tvector);
	if (_processId == 0) {
		for (int i = 0; i < 18; i++) {
			sendData[i] = i;
		}
		for (int i = 0; i < _numProcs - 1; i++) {
			std::cout << "Enviado al proceso " << i + 1 << " desde " << sendData[i * 2] << std::endl;
			MPI_Send(&sendData[i * 2], 1, tvector, i + 1, 0, MPI_COMM_WORLD);
		}
	} else {
		MPI_Recv(&receiveData, 6, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		std::cout << "Soy el proceso " << _processId << " y recibo: " << std::endl;
		for (int i = 0; i < 6; i++) {
			std::cout << receiveData[i] << ", ";
		}
		std::cout << std::endl;
	}


	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioTerna(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = 0;
	int blockLen[] = { 2, 3, 1 };
	int displacement[] = { 1, 6, 3 };
	int buffer[27], bufferRecv[18];


	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	MPI_Datatype type, terna;
	MPI_Type_contiguous(3, MPI_INT, &terna);
	MPI_Type_commit(&terna);
	MPI_Type_indexed(3, blockLen, displacement, terna, &type);
	MPI_Type_commit(&type);

	if (_processId == 0) {
		for (int i = 0; i < 27; i++) {
			buffer[i] = i;
		}
		MPI_Send(buffer, 1, type, 1, 0, MPI_COMM_WORLD);
	} else {
		for (int i = 0; i < 18; i++) {
			bufferRecv[i] = -1;
		}
		MPI_Recv(bufferRecv, 18, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
		for (int i = 0; i < 18; i++) {
			std::cout << bufferRecv[i] << ", ";
		}
		std::cout << std::endl;
	}

	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioComunicadores(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = -1;
	int newRanks[] = { 0, 2, 4, 6 };

	MPI_Group origGroup, newGroup;
	MPI_Comm newComm;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	if (_processId == 0)
		n = _numProcs;

	MPI_Comm_group(MPI_COMM_WORLD, &origGroup);
	MPI_Group_incl(origGroup, 4, newRanks, &newGroup);
	MPI_Comm_create(MPI_COMM_WORLD, newGroup, &newComm);
	int newRank = -1;
	if (_processId % 2 == 0)
		MPI_Comm_rank(newComm, &newRank);
	if (newRank >= 0) {
		MPI_Bcast(&n, 1, MPI_INT, 0, newComm);
		std::cout << "Soy el proceso " << _processId << " con nuevo rank " << newRank << " y recibo " << n << std::endl;
	}

	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioGrupoErrores(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = -1;
	int newRanks[] = { 0, 2, 4, 6 };

	MPI_Group origGroup, newGroup;
	MPI_Comm newComm;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos
	MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);

	if (_processId == 0)
		n = _numProcs;

	MPI_Comm_group(MPI_COMM_WORLD, &origGroup);
	MPI_Group_incl(origGroup, 4, newRanks, &newGroup);
	MPI_Comm_create(MPI_COMM_WORLD, newGroup, &newComm);

	int newRank = -1;
	int errorCode = MPI_Comm_rank(newComm, &newRank);
	if (errorCode == MPI_SUCCESS) {
		MPI_Bcast(&n, 1, MPI_INT, 0, newComm);
		std::cout << "Soy el proceso " << _processId << " con nuevo rank " << newRank << " y recibo " << n << std::endl;
	} else {
		std::cout << "Soy el proceso " << _processId << " y no he podido obtener un nuevo rango" << std::endl;
	}

	MPI_Finalize(); //Finalización OpenMPI
}

void EjercicioIntercomunicadores(int argc, char* argv[]) {
	int _processId, _numProcs;
	int n = -1;

	MPI_Comm splitComm, interComm;

	MPI_Init(&argc, &argv); //Inicialización OpenMPI
	MPI_Comm_rank(MPI_COMM_WORLD, &_processId);  // ID del proceso actual
	MPI_Comm_size(MPI_COMM_WORLD, &_numProcs);      // Nº de procesos

	int color = _processId % 2;


	MPI_Comm_split(MPI_COMM_WORLD, color, _processId, &splitComm);
	MPI_Intercomm_create(splitComm, 0, MPI_COMM_WORLD, 1 - color, 0, &interComm);

	int newRank = -1;
	if (color)
		MPI_Comm_rank(splitComm, &newRank);

	if (color) {
		MPI_Recv(&n, 1, MPI_INT, newRank, 0, interComm, MPI_STATUS_IGNORE);
		std::cout << "Soy el proceso " << _processId << " con nuevo rank " << newRank << " y recibo " << n << std::endl;
	} else {
		MPI_Send(&_processId, 1, MPI_INT, _processId, 0, interComm);
	}

	MPI_Finalize(); //Finalización OpenMPI
}
