/*
	Izquierdo Vera, Javier
	B2 - UGR
	javierizquierdovera@gmail.com
*/
#ifndef AGENT__
#define AGENT__

#include <string>
#include <map>
using namespace std;

// -----------------------------------------------------------
//				class Agent
// -----------------------------------------------------------
class Environment;
class Agent{
public:
	Agent(){
        trufa_size_=-1;
        bump_=false;

        // Inicializamos a -1 || 1 (desconocido)
       for(int i = 0; i < 20; i++)
        	for(int j = 0; j < 20; j++){
        		representacion[i][j].first = 1;
        		representacion[i][j].second = -1; // Mayor prioridad lo desconocido
        	}

        // Como no sabemos donde empezamos, suponemos el centro (así nunca nos faltará espacio en la matriz)
        posX = 9;
        posY = 9;

        // Siempre empieza orientado al norte -> 1
        orientacion = 1;

        // Para no dejar a media un método de movimiento
        accion_incompleta = 0;

        // La prioridad empieza con el valor más alto -> 0
        prioridad_actual = 0;

        // Ninguna acción ha sido la última
        ult_accion = 0;

        // SIn objetivo
        objetivo_fijado = false;

        //Pasos al empezar -> 0
        pasos = 0;

		// Evitar vueltas buscando el objetivo...
		rep_pos = 0;
		regresando = 0;
		sig_op = 0;


	}

	enum ActionType
	{
	    actFORWARD,
	    actTURN_L,
	    actTURN_R,
	    actSNIFF,
	    actEXTRACT,
	    actIDLE
	};

	void Perceive(const Environment &env);
	ActionType Think();
private:
	int trufa_size_;
	bool bump_;


    // Creamos la representación y fijamos el tamaño a 20x20 (el doble)
	pair<int,int> representacion[20][20];
	int posX;
	int posY;

	// Prioridad que dará a la próxima casilla libre
	unsigned int prioridad_actual;

	// ---- ORIENTACIÓN ----
	//	1 -> Norte ^
	//	2 -> Sur v
	//	3 -> Este ->
	//	4 -> Oeste <-
	int orientacion;

	//Métodos para agilizar
	int accion_incompleta;
	// ---- ORIENTACIÓN ----
	//	1 -> Arriba ^
	//	2 -> Abajo v
	//	3 -> Derecha ->
	//	4 -> Izquierda <-
	ActionType Arriba();
	ActionType Abajo();
	ActionType Derecha();
	ActionType Izquierda(); 

	void FijarObjetivo(int posXob, int posYob);

	// DEBUG: Mostrar representacion
	void Memoria();
	void Orientacion();
	void whereAreU();

	// Última acción
	//	0 -> Trufa  *
	//	1 -> Arriba ^
	//	2 -> Abajo v
	//	3 -> Derecha ->
	//	4 -> Izquierda <-
	int ult_accion;

	map<pair<int,int>,int> trufas; // [[posX,posY],nivel]
	bool objetivo_fijado; // Si está activado, el agente irá a por la truca trufas[id_objetivo]
	pair<int,int> id_objetivo; // Coordenadas del objetivo
	void CompletarRastreo(); // Guardar la información
	void Revisar(); // Comprobar si hay que marcar objetivo
	int pasos; // Pasos dados


	// Evitar vueltas buscando el objetivo...
	int rep_pos; // Contador, si lo sobrepasa ha de retroceder


	map<pair<int,int>,int> camino_pisado;
	int regresando;
	int sig_op;
	

};

string ActionStr(Agent::ActionType);

#endif
