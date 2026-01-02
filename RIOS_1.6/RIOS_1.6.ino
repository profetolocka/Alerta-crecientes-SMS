
/* Historial de Versiones
RIOS 1.6 (19/9/21)
Cambia los telefonos a los que reporta (saca algunos)

RIOS 1.5 (13/01/2021)
-Reporta el descenso del nivel, con su propia constante de umbral (UMBRAL_BAJANTE)


RIOS 1.4 (27-05-20)
-Al reportar que está vivo también envía el nivel del río

RIOS 1.3
-Cambian los teléfonos

RIOS 1.2
-Envia los mensajes a cuatro telefonos (no el del intendente) (Si alarmas)

RIOS 1.1 (14/01/18)
-Mide distancias con promedio (sonar.ping_median()
-Agrega ESTACION_ID
-Tiene cargados cinco numeros de telefono para reportar
-Tiene tareas cada 5 minutos y 24 horas
-Reporta ALARMA NIVEL si el nivel del rio es mayor a 0,50 metros (distancia medida menor a 1m)
-Reporta ALARMA CRECIENTE si en dos medidas consecutivas (Separadas 5 minutos) detecta una subida del nivel de
 5 cm (si el nivel baja no reporta)
 

RIOS 1.0
-Mide distancias
-Envia SMS por placa A6
-Reporta alarma si la distancia es menor a 1,5 metros


*/


#include <A6lib.h>
#include <SoftwareSerial.h>
#include <NewPing.h>          //Libreria del sensor de ultrasonido

#define TRIGGER_PIN  2        //Pin Trigger del sensor del Ultra Sonido 
#define ECHO_PIN     3        // Pin Echo del semsor del Ultra Sonnido 
#define MAX_DISTANCE 400      // Distancia Maxima del sensor del Ultra Sonido (4 mts)

#define ESTACION_ID   1       //Identificador de esta estacion (debería estar en EEPROM)

#define DISTANCIA_MIN   100     //Distancia mínima que genera una alarma
#define UMBRAL_CRECIDA    5     //Umbral de creciente para reportar alarma
#define UMBRAL_BAJANTE    5     //Umbral de bajante para reportar alarma
#define NIVEL0          150     //Distancia al lecho del rio (sin agua)

//#define DEBUG             1     //para que no mande a todos los telefonos

// Instanciar las librerias

A6lib A6modem;
SoftwareSerial Serial1(7, 8);
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);       //Crear objeto sensor de ultrasonidos

//Variables globales

//Telefonos a reportar
String NumeroTel0 = "+5491234567890";
String NumeroTel1 = "+5491234567890";
//String NumeroTel2 = "+5491234567890";
String NumeroTel3 = "+5491234567890"; 
String NumeroTel4 = "+5491234567890";
//String NumeroTel5 = "+5491234567890";
//String NumeroTel6 = "+5491234567890";
String NumeroTel7 = "+5491234567890";



//Para distribuir tareas

#ifdef DEBUG
unsigned long interval1 = 12000;  //1 minuto
#else
unsigned long interval1 = 60000;  //300000;     //5 minutos
#endif
unsigned long interval2 = 86400000;  //24 horas

int DistanciaActual, DistanciaAnterior;    //Para determinar diferencias

unsigned long previousMillis1, previousMillis2;

char AlarmaNivel = 0;               //Indica que se ha disparado una alarma por nivel

int MedirDistancia () {

  unsigned int uS;                              //Medicion de distancias en microsegundos
  int distancia;  
  
 //Medir distancia hasta el agua
  uS = sonar.ping_median(128);            //Medir en useg
  distancia = uS / US_ROUNDTRIP_CM;       //Calcular cm

  return distancia;
}

void RevisarNivel () {

  byte valorRetorno;
  char TextoSMS [30] = "";      //Texto a enviar dentro del SMS
  int NivelRio;                 //Para calcular el nivel del rio a partir de la distancia
  
 //Deteccion de crecientes + nivel

 DistanciaActual = MedirDistancia ();

 NivelRio = NIVEL0 - DistanciaActual;    //Nivel del agua
 
 Serial.println ("************************************");

 Serial.print ("Distancia Actual = ");
 Serial.println (DistanciaActual);
 Serial.print ("Distancia Anterior = ");
 Serial.println (DistanciaAnterior);
 Serial.print ("Diferencia = ");
 Serial.println (DistanciaActual-DistanciaAnterior);

 //Reporte por nivel excedido
 if (DistanciaActual < DISTANCIA_MIN) {

  if (!AlarmaNivel) {
    //Reportar alarma
    AlarmaNivel=1;                //para no reportar cada 5 minutos
    Serial.println ("Alarma Nivel > 0,5 m");
   
    sprintf (TextoSMS, "SNPSJ NIV=%03d ALERTA NIVEL", NivelRio);
 
    Serial.print ("Enviando ");
    Serial.println (TextoSMS);
    #ifdef DEBUG      //Solo a mi
      valorRetorno = A6modem.A6sendSMS(NumeroTel0, TextoSMS);
      if (valorRetorno == A6_OK)
        Serial.println("OK!!!");
      else    
        Serial.println("ERROR!!!");
    #else         //Mandar mensaje a todos
      Serial.println ("Reportando a todos");
      valorRetorno = A6modem.A6sendSMS(NumeroTel1, TextoSMS);
      delay (1000);
      //valorRetorno = A6modem.A6sendSMS(NumeroTel2, TextoSMS);
      //delay (1000);
      valorRetorno = A6modem.A6sendSMS(NumeroTel3, TextoSMS);
      delay (1000);
      valorRetorno = A6modem.A6sendSMS(NumeroTel4, TextoSMS);
      delay (1000);
      //valorRetorno = A6modem.A6sendSMS(NumeroTel5, TextoSMS);   
      //delay (1000);
      //valorRetorno = A6modem.A6sendSMS(NumeroTel6, TextoSMS);
      //delay (1000);
      valorRetorno = A6modem.A6sendSMS(NumeroTel7, TextoSMS);   
      delay (1000);
    #endif
    
  }
 
 } else {

  AlarmaNivel = 0;
  
 }

 
 //Reporte por crecientes
 if (DistanciaActual < DistanciaAnterior) {

 //El nivel está aumentando, ver cuanto (no reportar al decrecer)

    if (abs((DistanciaActual - DistanciaAnterior)) > UMBRAL_CRECIDA){
        //Crecio mas que el umbral
        //Reportar Alarma     
        Serial.println ("Alarma de crecida");

        sprintf (TextoSMS, "SNPSJ NIV=%03d ALERTA CRECIDA", NivelRio);
 
        Serial.print ("Enviando ");
        Serial.println (TextoSMS);

        #ifdef DEBUG      //Solo a mi
          valorRetorno = A6modem.A6sendSMS(NumeroTel0, TextoSMS);
          if (valorRetorno == A6_OK)
            Serial.println("OK!!!");
          else    
            Serial.println("ERROR!!!");
        #else         //Mandar mensaje a todos
          Serial.println ("Reportando a todos");
          valorRetorno = A6modem.A6sendSMS(NumeroTel1, TextoSMS);
          delay (1000);
          //valorRetorno = A6modem.A6sendSMS(NumeroTel2, TextoSMS);
          //delay (1000);
          valorRetorno = A6modem.A6sendSMS(NumeroTel3, TextoSMS);
          delay (1000);
          valorRetorno = A6modem.A6sendSMS(NumeroTel4, TextoSMS);
          delay (1000);
          //valorRetorno = A6modem.A6sendSMS(NumeroTel5, TextoSMS);   
          //delay (1000);
          //valorRetorno = A6modem.A6sendSMS(NumeroTel6, TextoSMS);
          //delay (1000);
          valorRetorno = A6modem.A6sendSMS(NumeroTel7, TextoSMS);   
          delay (1000);         
    #endif
        
    }
 }

 //Reporte por bajante (Ver 1.5)
 if (DistanciaActual > DistanciaAnterior) {

 //El nivel está disminuyendo, ver cuanto

    if (abs((DistanciaActual - DistanciaAnterior)) > UMBRAL_BAJANTE){
        //Bajo mas que el umbral
        //Reportar Alarma     
        Serial.println ("Alarma de bajante");

        sprintf (TextoSMS, "SNPSJ NIV=%03d ALERTA BAJANTE", NivelRio);
 
        Serial.print ("Enviando ");
        Serial.println (TextoSMS);

        #ifdef DEBUG      //Solo a mi
          valorRetorno = A6modem.A6sendSMS(NumeroTel0, TextoSMS);
          if (valorRetorno == A6_OK)
            Serial.println("OK!!!");
          else    
            Serial.println("ERROR!!!");
        #else         //Mandar mensaje a todos
          Serial.println ("Reportando a todos");
          valorRetorno = A6modem.A6sendSMS(NumeroTel1, TextoSMS);
          delay (1000);
          //valorRetorno = A6modem.A6sendSMS(NumeroTel2, TextoSMS);
          //delay (1000);
          valorRetorno = A6modem.A6sendSMS(NumeroTel3, TextoSMS);
          delay (1000);
          valorRetorno = A6modem.A6sendSMS(NumeroTel4, TextoSMS);
          delay (1000);
          //valorRetorno = A6modem.A6sendSMS(NumeroTel5, TextoSMS);   
          //delay (1000);
          //valorRetorno = A6modem.A6sendSMS(NumeroTel6, TextoSMS);
          //delay (1000);
          valorRetorno = A6modem.A6sendSMS(NumeroTel7, TextoSMS);   
          delay (1000);         
    #endif
        
    }
 }
 DistanciaAnterior = DistanciaActual;

  Serial.println ("***************************");
}

void ReportarSalud () {

//Avisar que está vivo cada 24 hs
  
  byte valorRetorno;
  char TextoSMS [30] = "";      //Texto a enviar dentro del SMS
  int Nivel;
  
  Serial.println ("Estoy vivo!");
  
  DistanciaActual = MedirDistancia ();
  Nivel = NIVEL0 - DistanciaActual;    //Nivel del agua
  sprintf (TextoSMS, "SNPSJ NIV=%03d FUNCIONANDO", Nivel);
  //sprintf (TextoSMS, "SNPSJ FUNCIONANDO"); anterior
 
  Serial.print ("Enviando ");
  Serial.println (TextoSMS);

  #ifdef DEBUG    //Solo a mi
  valorRetorno = A6modem.A6sendSMS(NumeroTel0, TextoSMS);
  if (valorRetorno == A6_OK)
    Serial.println("OK!!!");
  else    
    Serial.println("ERROR!!!");
  #else     //A todos menos intendente
   Serial.println ("Reportando a todos");
   valorRetorno = A6modem.A6sendSMS(NumeroTel1, TextoSMS); 
   delay (1000);
   //valorRetorno = A6modem.A6sendSMS(NumeroTel2, TextoSMS); 
   //delay (1000);  
   //valorRetorno = A6modem.A6sendSMS(NumeroTel3, TextoSMS);   Intendente
   //delay (1000);
   valorRetorno = A6modem.A6sendSMS(NumeroTel4, TextoSMS);
   delay (1000);
   //valorRetorno = A6modem.A6sendSMS(NumeroTel5, TextoSMS);
   //delay (1000);
   //valorRetorno = A6modem.A6sendSMS(NumeroTel6, TextoSMS);
   //delay (1000);
   valorRetorno = A6modem.A6sendSMS(NumeroTel7, TextoSMS);   
   delay (1000);
  #endif 
  
}

//=============================================

void setup() {

  // variables para enviar comandos al modem y recibir respuestas
  byte valorRetorno;
  String respuestaModem = "";
  String comandoModem = "";
  char TextoSMS [30] = "";      //Texto a enviar dentro del SMS
  
  // variable para nivel de señal
  int valor;

  // variables para SMS
  int SMSindice;
  String SMSnumero = "";
  String SMScontenido = "";

  // esperar conexion serial de debug (para LEONARDO)
  while (!Serial);
  // puerto serial de debug
  Serial.begin(115200);

  // hay que declarar un puerto serial (hardware o software) que se utilizará
  // para controlar el modem A6
  // puerto serial para comunicar con el modem
  Serial1.begin(9600);
  // inicializar libreria con la instancia de puerto previamente configurada
  A6modem.begin(Serial1);

  Serial.println("A6 GSM SHIELD TRCOM");
  Serial.println("");

  // Reset y encendido del modem (power cycle) SOLO SI EL MODEM NO RESPONDE
  // -----------------------------------------------
  Serial.println("- A6 Power cycle y config");
  // verificar si el modem responde a "AT"
  if ( A6modem.A6sendCommand("AT", "OK", "yy", A6_RESPONSE_TIMEOUT, 2) == A6_OK){
    // Modem ya esta inicializado, responde correctamente
    Serial.println("Modem inicializado.");        
  }
  else{
    // modem no respondio a comando, resetear
    Serial.println("Reseteando modem...");
    // resetear modem y sincronizar baudios
    valorRetorno = A6modem.A6powerCycle();
    if (valorRetorno == A6_OK)
      // se reseteo y sincronizo velocidad con el modem
      Serial.println("Reset OK!!!");
    else{    
      // error, no se logró resetear o sincronizar
      Serial.println("Reset ERROR!!!");
      //while(1);
      //Ver que hacemos si nunca arranca la placa celular. Led de ERROR?
    }
  }

  // si se logro sincronizacion con el modem, configurar
  // configurar el modem
  Serial.println("Configurando modem...");
  valorRetorno = A6modem.A6config();
  if (valorRetorno == A6_OK)
    Serial.println("Config OK!!!");
  else{    
    Serial.println("Config ERROR!!!");
    //while(1);
  }
    
  Serial.println("");
  delay(4000);
  // -----------------------------------------------
  
  // Registracion del modem en la red
  // -----------------------------------------------
  #ifdef A6_PRUEBA_CREG
  Serial.println("- A6 registracion en la red");
  valorRetorno = A6modem.A6reg();
  if (valorRetorno)
    Serial.print("Modem Registrado en la red");
  else
    Serial.print("Modem NO Registrado en la red");
  Serial.println("");
  delay(4000);
  #endif
  // -----------------------------------------------

  // Nivel de señal del modem
  // -----------------------------------------------
  #ifdef A6_PRUEBA_NIVEL_SENAL
  Serial.println("- A6 nivel de señal");
  valor = A6modem.A6getSignalStrength();
  Serial.print("Nivel de señal: ");
  Serial.println(valor);

  Serial.println("");
  delay(4000);
  #endif
  // -----------------------------------------------



  // Enviar SMS de arranque
 
  sprintf (TextoSMS, "SNPSJ ARRANQUE");
 
  Serial.print ("Enviando ");
  Serial.println (TextoSMS);
  
  #ifdef DEBUG  //solo a mi numero
  valorRetorno = A6modem.A6sendSMS(NumeroTel0, TextoSMS);
  if (valorRetorno == A6_OK)
    Serial.println("OK!!!");
  else    
    Serial.println("ERROR!!!");
  #else
  //Enviar a todos menos el intendente
    Serial.println ("Reportando a todos");
    valorRetorno = A6modem.A6sendSMS(NumeroTel1, TextoSMS);
    delay (1000);
    //valorRetorno = A6modem.A6sendSMS(NumeroTel2, TextoSMS);
    //delay (1000);
    //valorRetorno = A6modem.A6sendSMS(NumeroTel3, TextoSMS);    Intendente
    //delay (1000);
    valorRetorno = A6modem.A6sendSMS(NumeroTel4, TextoSMS);
    delay (1000);
    //valorRetorno = A6modem.A6sendSMS(NumeroTel5, TextoSMS);    
    //delay (1000);
   //valorRetorno = A6modem.A6sendSMS(NumeroTel6, TextoSMS);
   //delay (1000);
   valorRetorno = A6modem.A6sendSMS(NumeroTel7, TextoSMS);   
   delay (1000);
  
  #endif
  Serial.println("");
  delay(4000);

  //Capturar milis para las temporizaciones

  previousMillis1 = millis();
  previousMillis2 = millis();

  //Medir Nivel de agua actual

  DistanciaActual = MedirDistancia ();
  DistanciaAnterior = DistanciaActual;

  AlarmaNivel = 0;   //Bandera para no reportar cada 5 minutos

//TEST!!!
  //valorRetorno=A6modem.A6sendCommand("AT+CCLK?", "+CCLK:", "OK", 1000, 1, TextoSMS);
  //Serial.println (TextoSMS);
  
}

void loop() {
  char dato; //Variable para recibir datos desde el puerto serie
  // Verificar si hay datos disponibles en el puerto serial HARDWARE (provenientes de la PC)

  byte valorRetorno;

  unsigned long currentMillis=millis ();                 //Para las temporizaciones  


   if ((unsigned long)(currentMillis - previousMillis1) >= interval1)
   {
      //Cada 5 minutos
      RevisarNivel ();
      previousMillis1 = millis();
   }

   if ((unsigned long)(currentMillis - previousMillis2) >= interval2)
   {
      //Cada 24 horas
      ReportarSalud ();
      previousMillis2 = millis();
   }

}
