/*
	Izquierdo Vera, Javier
	B2 - UGR
	javierizquierdovera@gmail.com
*/
#include "agent.h"
#include "environment.h"
#include <iostream>
#include <cstdlib>
#include <vector>
#include <utility>

//Cálculos
#define arriba_posible ((posY + 1 < 20) && (representacion[posX][posY+1].first != obstaculo))
#define abajo_posible ((posY - 1 > -1) && (representacion[posX][posY-1].first != obstaculo))
#define derecha_posible ((posX + 1 < 20) && (representacion[posX+1][posY].first != obstaculo))
#define izquierda_posible ((posX - 1 > -1) && (representacion[posX-1][posY].first != obstaculo))
#define arriba_posible_valido arriba_posible && (camino_pisado.find(pair<int,int>(posX,posY+1)) == camino_pisado.end())
#define abajo_posible_valido abajo_posible && (camino_pisado.find(pair<int,int>(posX,posY-1)) == camino_pisado.end())
#define derecha_posible_valido derecha_posible && (camino_pisado.find(pair<int,int>(posX+1,posY)) == camino_pisado.end())
#define izquierda_posible_valido izquierda_posible && (camino_pisado.find(pair<int,int>(posX-1,posY)) == camino_pisado.end())
#define retirada 10

// Orientación y acciones
#define arriba 1
#define abajo 2
#define derecha	3 
#define	izquierda 4
#define	rastrear 5
#define extraer 7
#define otro 0

// Representación
#define obstaculo 0
#define camino 2
#define trufa 5
#define me 6

// Prioridades
#define pri_arriba representacion[posX][posY+1].second
#define pri_abajo representacion[posX][posY-1].second
#define pri_derecha	representacion[posX+1][posY].second
#define	pri_izquierda representacion[posX-1][posY].second
#define ignorar 9999999
#define alerta -2 // Prioridad si es crítico

// Rastreo
#define critico 1000 // (critico - nivel aromatico) < 0 -> El agente se siente atraido por la trufa
#define rematar 1000 // (critico - nivel aromatico) < 0 -> El agente va directo a por la trufa
#define comienzo_rastreo 40 // El agente comienza a rastrear a partir de los pasos -> comienzo_rastreo
#define extractNow 8 // Recoger en el momento ---- {0, 1, 2, 4, 6, 8, 10, 12, 14, 16, 18, 20}


// 0.01

// Agent:---------------------
// 10 - 1358
// 12 - 1552
// 8 - 1338

// Mapa 1:---------------------
// 10 - 1530
// 12 - 1500
// 8 - 1494

// Mapa 2:---------------------
// 10 - 1440
// 12 - 1422
// 8 - 1458

// Mapa 3:---------------------
// 10 - 1526
// 12 - 1534
// 8 - 1540


// 0.02

// Agent:---------------------
// 10 - 2854
// 12 - 2726
// 8 - 2810

// Mapa 1:---------------------
// 10 - 3094
// 12 - 3026
// 8 - 3114

// Mapa 2:---------------------
// 10 - 2960
// 12 - 2988
// 8 - 3030

// Mapa 3:---------------------
// 10 - 2862
// 12 - 2742
// 8 - 2890




using namespace std;
// -----------------------------------------------------------
Agent::ActionType Agent::Think()
{

					
	ActionType accion;

	// Completar acciones incompletas:
	switch(accion_incompleta){
		case arriba: accion = Arriba();
				cout << "Arriba" << endl; // Debug
				break;
		case abajo: accion = Abajo();
				cout << "Abajo" << endl; // Debug
				break;
		case derecha: accion = Derecha();
				cout << "Derecha" << endl; // Debug
				break;
		case izquierda: accion = Izquierda();
				cout << "Izquierda" << endl; // Debug
				break;
		case otro: cout << "Otro" << endl; // Debug

				/// ------------------------------------ ¿HEMOS CHOCADO O HEMOS AVANZADO? ---------------------------------

				// Si hemos chocado
				if (bump_){
					cout << "Choque " << ult_accion << endl; // Debug
					pasos--; // No hemos dado este paso...
					switch(ult_accion){
						//A la posición que tenemos guardada no hemos podido llegar, de modo que hay que marcarla y retroceder en la memoria
						case arriba: representacion[posX][posY].first = obstaculo; 
							representacion[posX][posY].second = ignorar; 
							cout << "Arriba marcado x = " << posX << " y = " << posY << endl; // Debug
							posY--;
							break;
						case abajo: representacion[posX][posY].first = obstaculo;
							representacion[posX][posY].second = ignorar; 
							cout << "Abajo marcado x = " << posX << " y = " << posY << endl; // Debug
							posY++;
							break;
						case derecha: representacion[posX][posY].first = obstaculo;
							representacion[posX][posY].second = ignorar; 
							cout << "Derecha marcado x = " << posX << " y = " << posY << endl; // Debug
							posX--;
							break;
						case izquierda: representacion[posX][posY].first = obstaculo;
							representacion[posX][posY].second = ignorar; 
							cout << "Izquierda marcado x = " << posX << " y = " << posY << endl; // Debug
							posX++;
							break;
					}
				// Si no hemos chocado, cambiamos la prioridad y revisamos si hay trufas
				} else {

					// RASTREO revisión y establecer la casilla
					CompletarRastreo();
					Revisar();
				} 


				/// ------------------------------------ ¿A DÓNDE VAMOS AHORA? ---------------------------------

				// Si tenemos objetivo, al objetivo
				if (objetivo_fijado){


					// ¿Estamos en el objetivo?
					if (pair<int,int>(posX,posY) == id_objetivo){

						cout << posX << " == " << id_objetivo.first << endl; // Debug
						cout << posY << " == " << id_objetivo.second << endl; // Debug

						objetivo_fijado = false;
						accion = actEXTRACT; // Las vamos a coger
						ult_accion = extraer;
						trufas.erase(pair<int,int>(posX,posY)); // Lo quitamos de la lista de trufas
						cout << "¡¡Extraemos trufas objetivo!! x=" << posX << " y=" << posY << endl; //debug

						//Reseteo:
						id_objetivo.first = -1;
						id_objetivo.second = -1;
						camino_pisado.clear();

					} else {

						pasos++;
						camino_pisado[pair<int,int>(posX,posY)] = 1; // Memorizamos el camino por el que pasamos para no perdernos

						cout << "Objetivo -----------------> x=" << id_objetivo.first << "y=" << id_objetivo.second << endl;
						// Las mejores situaciones para acercarnos:
						bool arriba_recomendado = (arriba_posible_valido && // No está marcada como inválida
												((id_objetivo.second - (posY+1)) >= 0) && // Revisar que no sea dirección contraria! (<0)
												(id_objetivo.second - (posY+1) < (id_objetivo.second - (posY)))); // ¿Hay mejora?

						bool abajo_recomendado = (abajo_posible_valido && // No está marcada como inválida
												(((posY-1) - id_objetivo.second) >= 0) && // Revisar que no sea dirección contraria! (<0)
												((posY-1) - id_objetivo.second < ((posY) - id_objetivo.second))); // ¿Hay mejora?
//
						bool derecha_recomendado = (derecha_posible_valido && // No está marcada como inválida
												((id_objetivo.first - (posX+1)) >= 0) && // Revisar que no sea dirección contraria! (<0)
												(id_objetivo.first - (posX+1) < (id_objetivo.first - (posX)))); // ¿Hay mejora?
//
						bool izquierda_recomendado = (izquierda_posible_valido && // No está marcada como inválida
												(((posX-1) - id_objetivo.first) >= 0) && // Revisar que no sea dirección contraria! (<0)
												((posX-1) - id_objetivo.first < ((posX) - id_objetivo.first))); // ¿Hay mejora?		
					
//

						if (arriba_recomendado){
							cout << "Es recomendado arriba " << endl; // Debug
							accion = Arriba();
							ult_accion = arriba;
							rep_pos = 0;
							regresando = otro; // No estamos regresando
							sig_op = 0;

						} else if (abajo_recomendado){
							cout << "Es recomendado abajo " << endl; // Debug
							accion = Abajo();
							ult_accion = abajo;
							rep_pos = 0;
							regresando = otro; // No estamos regresando
							sig_op = 0;

						} else if (derecha_recomendado){
							cout << "Es recomendado derecha " << endl; // Debug
							accion = Derecha();
							ult_accion = derecha;
							rep_pos = 0;
							regresando = otro; // No estamos regresando
							sig_op = 0;

						} else if (izquierda_recomendado){
							cout << "Es recomendado izquierda " << endl; // Debug
							accion = Izquierda();
							ult_accion = izquierda;
							rep_pos = 0;
							regresando = otro; // No estamos regresando
							sig_op = 0;


						} else { // Si no podemos acercarnos, tratamos de ir por un camino nuevo
							if (arriba_posible_valido){
								accion = Arriba();
								ult_accion = arriba;
								cout << "Vamos arriba, aunque nos alejamos, es distinto " << endl; // Debug
								rep_pos = 0;
								regresando = otro; // No estamos regresando
								sig_op = 0;

							} else if(abajo_posible_valido){
								accion = Abajo();
								ult_accion = abajo;
								cout << "Vamos abajo, aunque nos alejamos, es distinto " << endl; // Debug
								rep_pos = 0;
								regresando = otro; // No estamos regresando
								sig_op = 0;

							} else if(derecha_posible_valido){
								accion = Derecha();
								ult_accion = derecha;
								cout << "Vamos derecha, aunque nos alejamos, es distinto " << endl; // Debug
								rep_pos = 0;
								regresando = otro; // No estamos regresando
								sig_op = 0;

							} else if(izquierda_posible_valido){
								accion = Izquierda();
								ult_accion = izquierda;
								cout << "Vamos izquierda, aunque nos alejamos, es distinto " << endl; // Debug
								rep_pos = 0;
								regresando = otro; // No estamos regresando
								sig_op = 0;

							} else { // Si no hay camino nuevo posible, tratamos de ir por cualquiera conocido salvo por donde hemos venido
								
								if (rep_pos < retirada && ult_accion != abajo && arriba_posible){
									accion = Arriba();
									ult_accion = arriba;
									cout << "Vamos arriba por ir a algún sitio... " << endl; // Debug
									rep_pos++;
									regresando = otro; // No estamos regresando
									sig_op = 0;

								} else if(rep_pos < retirada && ult_accion != arriba && abajo_posible){
									accion = Abajo();
									ult_accion = abajo;
									cout << "Vamos abajo por ir a algún sitio... " << endl; // Debug
									rep_pos++;
									regresando = otro; // No estamos regresando
									sig_op = 0;

								} else if(rep_pos < retirada && ult_accion != izquierda && derecha_posible){
									accion = Derecha();
									ult_accion = derecha;
									cout << "Vamos derecha por ir a algún sitio... " << endl; // Debug
									rep_pos++;
									regresando = otro; // No estamos regresando
									sig_op = 0;

								} else if(rep_pos < retirada && ult_accion != derecha && izquierda_posible){
									accion = Izquierda();
									ult_accion = izquierda;
									cout << "Vamos izquierda por ir a algún sitio... " << endl; // Debug
									rep_pos++; 
									regresando = otro; // No estamos regresando
									sig_op = 0;

								} else{ // Hay que buscar otra opción

									if (bump_ && regresando != otro){ // Si nos ha llevado a chocar y estábamos con una dirección fijada... la corregimos
										if (sig_op == arriba){
											accion = Abajo(); 
											cout << "Corregido -> abajo para regresar" << endl; 
											ult_accion = abajo; // Vamos hacia abajo
											regresando = arriba; // Regresamos desde arriba
											sig_op = derecha;
										} else if (sig_op == abajo){
											accion = Arriba(); 
											cout << "Corregido -> arriba para regresar" << endl; 
											ult_accion = arriba; 
											regresando = abajo;
											sig_op = izquierda;
										} else if (sig_op == derecha){
											accion = Izquierda(); 
											cout << "Corregido -> izquierda para regresar" << endl; 
											ult_accion = izquierda; 
											regresando = derecha;
											sig_op = abajo;
										} else if (sig_op == izquierda){
											accion = Derecha(); 
											cout << "Corregido -> derecha para regresar" << endl; 
											ult_accion = derecha; 
											regresando = izquierda;
											sig_op = arriba;
										}

									// Si no hemos fijado un camino de regreso, vamos por donde hemos venido
									// Si lo hemos fijado, vamos por ese
									} else if ((ult_accion == arriba && regresando == otro) || (regresando == arriba)){
											accion = Abajo(); 
											cout << "Abajo para regresar" << endl; 
											ult_accion = abajo; // Vamos hacia abajo
											regresando = arriba; // Regresamos desde arriba
											sig_op = izquierda;
									} else if ((ult_accion == abajo && regresando == otro) || (regresando == abajo)){
											accion = Arriba(); 
											cout << "Arriba para regresar" << endl; 
											ult_accion = arriba; 
											regresando = abajo;
											sig_op = derecha;
									} else if ((ult_accion == derecha && regresando == otro) || (regresando == derecha)){
											accion = Izquierda(); 
											cout << "Izquierda para regresar" << endl; 
											ult_accion = izquierda; 
											regresando = derecha;
											sig_op = arriba;
									} else if ((ult_accion == izquierda && regresando == otro) || (regresando == izquierda)){
											accion = Derecha(); 
											cout << "Derecha para regresar" << endl; 
											ult_accion = derecha; 
											regresando = izquierda;
											sig_op = abajo;
									}
								}


							}

						}

					}
				} else { // Y sino, pues vamos a pensar....

					// ¿Estamos sobre una trufa crítica?
					if (representacion[posX][posY].second == alerta){
						accion = actEXTRACT; // Las vamos a coger
						ult_accion = extraer;
						trufas.erase(pair<int,int>(posX,posY)); // Lo quitamos de la lista d etrufas
						cout << "¡¡Extraemos trufas críticas!! x=" << posX << " y=" << posY << "(" << representacion[posX][posY].second << ")" << endl; //debug
						representacion[posX][posY].second = prioridad_actual; // Prioridad normal
						prioridad_actual++;


					// ¿Toca rastrear? Rastreamos 1 vez sí, otra no, sucesivamente, para que pueda avanzar. Si la anterior no ha sido un choque
					} else if (!bump_ && pasos >= comienzo_rastreo && pasos%2 == 0){ // Rastreamo si es par y si es mayor que comienzo_rastreo

						accion = actSNIFF;
						cout << "Seleccionado rastrear" << endl; // Debug
						ult_accion = rastrear;
						pasos++;

					} else { // Toca avanzar
						// NOTA: No hay que preocuparse de que no decida regresar, ya que la prioridad es mayor, a no ser que lo necesite (todo muro = 9999 salvo el anterior)
						// ¿SUBIR?							
						cout << "Seleccionando..." << endl; // Debug

						if (arriba_posible && // No es el borde y no hay obstáculo
							pri_arriba <= pri_derecha && // Es mejor que ir a la derecha
							pri_arriba <= pri_izquierda && // Es mejor que ir a la izquierda
							pri_arriba <= pri_abajo){ // Es mejor que bajar

								cout << "Seleccionado arriba" << endl; // Debug
								accion = Arriba();
								ult_accion = arriba;
								pasos++;

						// ¿BAJAR?
						}else if (abajo_posible && // No es el borde y no hay obstáculo
							pri_abajo <= pri_derecha && // Es mejor que ir a la derecha
							pri_abajo <= pri_izquierda && // Es mejor que ir a la izquierda
							pri_abajo <= pri_arriba){ // Es mejor que arriba

								cout << "Seleccionado abajo" << endl; // Debug
								accion = Abajo();
								ult_accion = abajo;
								pasos++;

						// ¿DERECHA?
						}else if (derecha_posible && // No es el borde y no hay obstáculo
							pri_derecha <= pri_izquierda && // Es mejor que ir a la derecha
							pri_derecha <= pri_arriba && // Es mejor que ir a la izquierda
							pri_derecha <= pri_abajo){ // Es mejor que arriba

								cout << "Seleccionado derecha" << endl; // Debug
								accion = Derecha();
								ult_accion = derecha;
								pasos++;

						// ¿IZQUIERDA?
						}else if (izquierda_posible && // No es el borde y no hay obstáculo
							pri_izquierda <= pri_derecha && // Es mejor que ir a la derecha
							pri_izquierda <= pri_arriba && // Es mejor que ir a la izquierda
							pri_izquierda <= pri_abajo){ // Es mejor que arriba

								cout << "Seleccionado izquierda" << endl; // Debug
								accion = Izquierda();
								ult_accion = izquierda;
								pasos++;


						}
					}
				}

				break;


	}

	
	cout << "Pos actual x = " << posX << " y = " << posY << endl; // Debug
	Memoria(); // Debug
	Orientacion(); // Debug
	cout << "Pasos dados -> " << pasos << endl; // Debug



	return accion;
}

void Agent::Revisar(){
	if (!trufas.empty()){ // Si tenemos trufas memorizadas hay que actualizar la estimación

		for(map<pair<int,int>,int>::iterator it = trufas.begin(); it != trufas.end(); ++it){
			(it->second)++;
			cout << "Estimación trufa x=" << (it->first).first << " y=" << (it->first).second << " --> " << it->second << endl; //debug


			if (!objetivo_fijado && (it->second) >= rematar){ // Si hay que ir a por ella... Pero solo vamos si no estamos yendo a por otra!
				FijarObjetivo((it->first).first,(it->first).second);

			}else if((it->second) >= critico){ //Si entramos en zona ćrítica...
				representacion[(it->first).first][(it->first).second].second = alerta; // Prioridad critica
				cout << "(!) Nivel ćrítico trufa x=" << (it->first).first << " y=" << (it->first).second << " --> " << it->second << endl; //debug

			} else {
				representacion[posX][posY].second = prioridad_actual; // Prioridad baja
				prioridad_actual++;
			}

		}

	}

}

void Agent::FijarObjetivo(int posXob, int posYob){
	id_objetivo.first = posXob; // Guardamos X
	id_objetivo.second = posYob; // Guardamos Y
	objetivo_fijado = true; // Objetivo fijado (!)
	cout << "(!) OBJETIVO FIJADO trufa x=" << posXob << " y=" << posYob << endl; //debug


}


void Agent::CompletarRastreo(){
	if (ult_accion == rastrear && trufa_size_ > 0){ // Acabamos de rastrear y había algo

		trufas[pair<int,int>(posX,posY)] = trufa_size_*2;
		representacion[posX][posY].first = trufa; // Casilla con trufa

		if (trufa_size_ >= extractNow)
			FijarObjetivo(posX,posY);



	} else {
		if (representacion[posX][posY].first != trufa){ // Si no la teníamos ya registrada como trufa, es camino:
			representacion[posX][posY].first = camino; // Casilla válida
			representacion[posX][posY].second = prioridad_actual; // Prioridad normal
			prioridad_actual++;
			cout << "Prioridad cambiada x = " << posX << " y = " << posY << " -> " << prioridad_actual << endl; // Debug
		}
	}
}

Agent::ActionType Agent::Arriba(){ //1 ^
	ActionType accion;
	accion_incompleta = arriba;

	switch(orientacion){
		case arriba: accion=actFORWARD;
				/***/cout << "De arriba a frente [objetivo -> arriba]" << endl; // Debug
				posY++;
				whereAreU();
				accion_incompleta = otro;
				break;
		case abajo: accion=actTURN_L;
				/***/cout << "De abajo a izquierda [objetivo -> arriba]" << endl; // Debug
				orientacion = derecha;
				break;
		case derecha: accion=actTURN_L;
				/***/cout << "De derecha a izquierda [objetivo -> arriba]" << endl; // Debug
				orientacion = arriba;
				break;
		case izquierda: accion=actTURN_R;
				/***/cout << "De izquierda a derecha [objetivo -> arriba]" << endl; // Debug
				orientacion = arriba;
				break;
	}

	return accion;
}


Agent::ActionType Agent::Abajo(){ //2 v
	ActionType accion;
	accion_incompleta = abajo;

	switch(orientacion){
		case arriba: accion=actTURN_L;
				/***/cout << "De arriba a izquierda [objetivo -> abajo]" << endl; // Debug
				orientacion = izquierda;
				break;
		case abajo: accion=actFORWARD;
				/***/cout << "De abajo a frente [objetivo -> abajo]" << endl; // Debug
				posY--;
				whereAreU();
				accion_incompleta = otro;
				break;
		case derecha: accion=actTURN_R;
				/***/cout << "De derecha a derecha [objetivo -> abajo]" << endl; // Debug
				orientacion = abajo;
				break;
		case izquierda: accion=actTURN_L;
				/***/cout << "De izquierda a izquierda [objetivo -> abajo]" << endl; // Debug
				orientacion = abajo;
				break;
	}

	return accion;
}

Agent::ActionType Agent::Derecha(){ //3 ->
	ActionType accion;
	accion_incompleta = derecha;

	switch(orientacion){
		case arriba: accion=actTURN_R;
				/***/cout << "De arriba a derecha [objetivo -> derecha]" << endl; // Debug
				orientacion = derecha;
				break;
		case abajo: accion=actTURN_L;
				/***/cout << "De abajo a izquierda [objetivo -> derecha]" << endl; // Debug
				orientacion = derecha;
				break;
		case derecha: accion=actFORWARD;
				/***/cout << "De derecha a frente [objetivo -> derecha]" << endl; // Debug
				posX++;
				whereAreU();
				accion_incompleta = otro;
				break;
		case izquierda: accion=actTURN_R;
				/***/cout << "De izquierda a derecha [objetivo -> derecha]" << endl; // Debug
				orientacion = arriba;
				break;
	}

	return accion;

}

Agent::ActionType Agent::Izquierda(){ //4 <-
	ActionType accion;
	accion_incompleta = izquierda;

	switch(orientacion){
		case arriba: accion=actTURN_L;
				/***/cout << "De arriba a izquierda [objetivo -> izquierda]" << endl; // Debug
				orientacion = izquierda;
				break;
		case abajo: accion=actTURN_R;
				/***/cout << "De abajo a derecha [objetivo -> izquierda]" << endl; // Debug
				orientacion = izquierda;
				break;
		case derecha: accion=actTURN_L;
				/***/cout << "De derecha a izquierda [objetivo -> izquierda]" << endl; // Debug
				orientacion = arriba;
				break;
		case izquierda: accion=actFORWARD;
				/***/cout << "De izquierda a frente [objetivo -> izquierda]" << endl; // Debug
				posX--;
				whereAreU();
				accion_incompleta = otro;
				break;
	}

	return accion;

}

void Agent::Memoria(){ // Debug
	for(int i = 19; i > 0; i--){
		for(int j = 0; j < 20; j++)
			cout << representacion[j][i].first << " ";
			cout << endl;
	   }

}

void Agent::Orientacion(){ // Debug
	switch(orientacion){
		case arriba: cout << "Orientación --------> NORTE ^ " << endl;
				break;
		case abajo: cout << "Orientación --------> SUR v " << endl;
				break;
		case derecha: cout << "Orientación --------> ESTE > " << endl;
				break;
		case izquierda: cout << "Orientación --------> OESTE < " << endl;
				break;
	}

}

void Agent::whereAreU(){ // Debug
	representacion[posX][posY].first = me;
	cout << "He marcado mi posición con un " << me << endl;
}

// -----------------------------------------------------------
void Agent::Perceive(const Environment &env)
{
	trufa_size_ = env.TrufaAmount();
	bump_ = env.isJustBump();
}
// -----------------------------------------------------------
string ActionStr(Agent::ActionType accion)
{
	switch (accion)
	{
	case Agent::actFORWARD: return "FORWARD";
	case Agent::actTURN_L: return "TURN LEFT";
	case Agent::actTURN_R: return "TURN RIGHT";
	case Agent::actSNIFF: return "SNIFF";
	case Agent::actEXTRACT: return "EXTRACT";
	case Agent::actIDLE: return "IDLE";
	default: return "???";
	}
}
