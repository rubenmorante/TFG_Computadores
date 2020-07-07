#include <GSM.h>

GSM gsmAccess(true);
GSM_SMS sms;

//datos de la configuracion
const long TIEMPO_PRIMERA_FASE = 1800; //constante que almacena el tiempo de la primera fase en segundos
const long TIEMPO_SEGUNDA_FASE = 60; //constante que almacena el tiempo de la segunda fase en segundos
const long TIEMPO_CUARTA_FASE = 300; //constante que almacena el tiempo de la cuarta fase en segundos
const long UN_SEGUNDO = 1000; //constante que almace el tiempo de un segundo en milisegundos

//declaración e inicialización de las variables de los pines de los botones
const byte SOS = 4; //constante del pin del botón de emergencia
const byte TENSION = 5; //constante del pin de la bateria
const byte TIEMPO = 6; //constante del pin del botón de tiempo
const byte TAMPER = 7; //constante del pin del tamper

//declaración e inicialización de las variables de los pines de los LEDs y del altavoz
const byte BUZZER = 8; //constante del pin del altavoz
const byte LED_VERDE_1 = 9; //constante del pin del LED verde 1
const byte LED_VERDE_2 = 10; //constante del pin del LED verde 2
const byte LED_VERDE_3 = 11; //constante del pin del LED verde 3
const byte LED_AMARILLO = 12; //constante del pin del LED amarillo
const byte LED_ROJO = 13; //constante del pin del LED rojo

//declaración de las variables de los tiempos
unsigned long tiempo_contador; //variable que guarda el numero de milisegundos que han pasado desde que empezo el programa
unsigned long tiempoPrimeraFase; //variable del tiempo del primer periodo
unsigned long tiempoSegundaFase; //variable del tiempo del segundo periodo
unsigned long tiempoCuartaFase; //variable del tiempo del cuarto periodo

//otras variables
byte fase; //variable que guarda la fase actual

//Definición e inicialización de las cadenas de mensaje y número de teléfono 
const char PIN_NUMBER[4] = ""; //PIN de la tarjeta SIM 
const char REMOTE_NUM[20] = "629023159";//"695667661"; //"629023159"; // Numero de telefono entre comillas al que se le va a enviar
const char MSG_TIEMPO[200] = "Tiempo agotado";  // mensaje del tiempo limite
const char msg_sos[200] = "S.O.S";  // mensaje de sos
const char msg_tamper[200] = "Tamper activado";  // mensaje de caja abierta
const char msg_tension[200] = "Fallo de tensión";  // mensaje de cable desconectado

//--------------------------------------------------------------------------------

//Inicializar la variable que guarda el tiempo transcurrido
void inicializarCuentaAtras(unsigned long &crono, unsigned long tiempo){
  crono = millis() + tiempo;  
}

//altavoz
void altavoz(int fase){
  switch (fase){
    case 2: case 3: case 4: /*Serial.println(F("Altavoz Encendido"));*/ tone(BUZZER,294); break;
    default: /*Serial.println(F("Altavoz Apagado"));*/ noTone(BUZZER); break;
  }
}

//Piloto rojo
void pilotoRojo(int fase){
  switch (fase){
    case 2: case 3: case 4: /*Serial.println(F("Piloto rojo Encendido"));*/ digitalWrite(LED_ROJO,HIGH); break;
    default: /*Serial.println(F("Piloto rojo Apagado"));*/ digitalWrite(LED_ROJO,LOW); break;
  }
}
    
//Pone todas las variables a su estado inicial
void inicializarValoresDeRearme(){
  inicializarCuentaAtras(tiempo_contador, tiempoPrimeraFase);
  fase = 1;  
  altavoz(0);
  pilotoRojo(0);
}

//---------------------------SETUP-----------------------------------------------------

void setup()
{  
  Serial.begin(9600); // Inicializa el serial Serial en 9600 baud
  while (!Serial){;} // wait for serial port to connect. Needed for native USB port only

  //inicialización de las variables de los tiempos   
  tiempo_contador = 0; //variable que guarda el numero de milisegundos que han pasado desde que empezo el programa
  tiempoPrimeraFase = TIEMPO_PRIMERA_FASE * UN_SEGUNDO; //variable del tiempo del primer periodo
  tiempoSegundaFase = TIEMPO_SEGUNDA_FASE * UN_SEGUNDO; //variable del tiempo del segundo periodo
  tiempoCuartaFase = TIEMPO_CUARTA_FASE * UN_SEGUNDO; //variable del tiempo del cuarto periodo

  //Definición de las entradas
  pinMode(TAMPER,INPUT); //pin del tamper como entrada
  pinMode(TENSION,INPUT); //pin de la bateria como entrada
  pinMode(SOS,INPUT); //pin de boton llamada de emergencia como entrada
  pinMode(TIEMPO,INPUT); //pin de boton de tiempo como entrada
  
  //Definición de las salidas
  pinMode(BUZZER,OUTPUT); //pin del altavoz como salida
  pinMode(LED_VERDE_1,OUTPUT); //pin del LED verde 1 como salida
  pinMode(LED_VERDE_2,OUTPUT); //pin del LED verde 2 como salida
  pinMode(LED_VERDE_3,OUTPUT); //pin del LED verde 3 como salida
  pinMode(LED_AMARILLO,OUTPUT); //pind del LED amarillo como salida
  pinMode(LED_ROJO,OUTPUT); //pin del LED rojo como salida

  //Inicialización de las salidas
  digitalWrite(BUZZER,LOW); //señal del altavoz
  digitalWrite(LED_VERDE_1,LOW); //señal del LED verde 1
  digitalWrite(LED_VERDE_2,LOW); //señal del LED verde 2
  digitalWrite(LED_VERDE_3,LOW); //señal del LED verde 3
  digitalWrite(LED_AMARILLO,LOW); //señal del LED amarillo
  digitalWrite(LED_ROJO,LOW); //señal del LED rojo
  
  
  //GSM
  /*boolean notConnected = true;
  Serial.println(F("GSM intentando conectar"));
  while(notConnected){
    if(gsmAccess.begin(PIN_NUMBER) == GSM_READY){
      notConnected = false;
      Serial.println(F("Connected"));
    }else{
      Serial.println(F("Not connected"));
      delay(1000);
    }
  }
  Serial.println(F("GSM initialized"));*/

  inicializarValoresDeRearme();
}

//-----------------------LOOP---------------------------------------------------------

void loop()
{
  switch(fase){
    case 1: alarma(tiempoSegundaFase, tiempo_contador, fase); break;
    case 2: alarma(tiempoCuartaFase, tiempo_contador, fase); break;
    case 3: enviarSMS(REMOTE_NUM, MSG_TIEMPO); faseSiguiente(fase); break;
    case 4: alarma(0, tiempo_contador, fase); break;
    case 5: break;
  }
  
  botones();
}

//--------------------------------------------------------------------------------

//Coge el valor de los botones  
void botones(){
  botonTension();    
  botonTamper();
  botonTiempo();
  botonSOS();
}

//boton tamper
void botonTamper(){
  int disparo_tamper = digitalRead(TAMPER); //guarda la señal actual del botón tamper
  if(disparo_tamper == HIGH){  //CAMBIAR CABLES CUANDO SEA DEFINITIVO    
    Serial.println(F("boton tamper Encendido"));
    enviarSMS(REMOTE_NUM, msg_tamper);      
    //Serial.println(F("mensaje tamper Enviado"));
  }
}

//boton tiempo    
void botonTiempo(){
  int disparo_tiempo = digitalRead(TIEMPO); //guarda la señal actual del botón tiempo  
  if(disparo_tiempo == HIGH){      
    Serial.println(F("boton tiempo Encendido"));
    inicializarValoresDeRearme();
  }
}

//boton sos  
void botonSOS(){
  int disparo_sos = digitalRead(SOS); //guarda la señal actual del botón de emergencia SOS
  if(disparo_sos == HIGH){
    //Serial.println(F("boton SOS Encendido"));
    enviarSMS(REMOTE_NUM, msg_sos);   
    //Serial.println(F("mensaje SOS Enviado"));
  }  
}

//boton tension
void botonTension(){
  int disparo_tension = digitalRead(TENSION); //guarda la señal actual de la bateria
  if(disparo_tension == HIGH){
    //Serial.println(F("boton tension Encendido"));
    enviarSMS(REMOTE_NUM, msg_tension);   
    //Serial.println(F("mensaje tension Enviado"));
  }  
}

//manda un sms
void enviarSMS(const char num[20], const char txt[200]){
    /*sms.beginSMS(num); //numero de telefono donde se envía
    sms.print(txt); //mensaje de texto que se va a enviar
    sms.endSMS(); //se envía el mensaje*/
}

//Comprueba si el tiempo transcurrido es menor al tiempo esperado
unsigned long comprobarCuentaAtras(unsigned long &crono){
  if (crono > millis()){
    return (crono - millis());
  } else {
    return 0;
  }
}

//Pasa de una fase a la consecutiva
void faseSiguiente(byte &fase){
  fase = fase + 1;
}

void alarma(unsigned long tiempo, unsigned long &crono, byte &fase){
  //Serial.println(F(comprobarCuentaAtras(crono)));
  if (0 == ((comprobarCuentaAtras(crono) > 0 ? 1 : 0))){
    inicializarCuentaAtras(crono, tiempo);
    faseSiguiente(fase);
    //altavoz(fase);
    pilotoRojo(fase);
  }
}
