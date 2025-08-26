//Declaración de variables

// -------- Variables para la lectura de Mitutoyo
int req; //Pin Req de Mitutoyo
int dat; //Pin dat (datos) de Mitutoyo
int clk; //Pin clk (reloj) de Mitutoyo
int i = 0, j = 0, k = 0; //Variables de control

int signo = 0; //Signo de la medida obtenido por el Mitutotoyo (Positivo o Negativo)
int decimal;   //Valor de la medida obtenida por el Mitutoyo
float dpp;     //Valor decimal de la medida obtendida por el Mitutoyo
int units;

byte mydata[14];
String value_str; // Valor en Cadena de la medida obtenida por el Mitutoyo
long value_int;   // Valor en Entero (Long) de la medida obtenida por Mitutoyo
double value;      // Valor flotante de la medida obtenida por el Mitutoyo

// -------- Variables para filtro (Mitutoyo)
long TiempoInicial;
int Muestras;
double sum1,sum2,ValorProm;

// -------- Variables para la lectura de LVDT - 1
boolean band1 = false;
long ValInicio1 = 0, resultLVDT1, sum3;

// -------- Variables para la lectura de LVDT - 2
boolean band2 = false;
long ValInicio2 = 0, resultLVDT2, sum4;

char DePython; // Lectura de caracter enviado desde Python

// -------------------------------------------

// Configuración del Arduino...
void setup(){
  //Configuración de la comunicación serial... --- 9600 Baudios
  Serial.begin(9600);

  //Configuración Pines de captura Mitutoyo - 1
  pinMode(5, OUTPUT);       //Req
  pinMode(3, INPUT_PULLUP); //CLK
  pinMode(2, INPUT_PULLUP); //Data

  //Configuración Pines de captura Mitutoyo - 2
  pinMode(6, OUTPUT);       //Req
  pinMode(9, INPUT_PULLUP); //CLK
  pinMode(4, INPUT_PULLUP); //Data

  band1 = false;
  band2 = false;
}

//Lectura de los dispositivos de medición...
void loop() {
  while (Serial.available()) {
    if (Serial.available() > 0) {
      //Leyendo de Python
      DePython = Serial.read();
      delay(100);

      if (DePython == 'a') {
        // ------- Lectura del Mitutoyo 1
        sum1 = 0.0;
        TiempoInicial = millis();
        Muestras = 0;

        while (millis() - TiempoInicial < 100) {
          Muestras++;
          sum1 += LectMitutoyo(5, 3, 2);
        }
        ValorProm = sum1 / Muestras;
        Serial.println(ValorProm, 3);
        // -------------------------------------------

        // ------- Lectura del Mitutoyo 2
        sum2 = 0.0;
        TiempoInicial = millis();
        Muestras = 0;

        while (millis() - TiempoInicial < 100) {
          Muestras++;
          sum2 += LectMitutoyo(6, 9, 4); //random(-13,13);
        }
        ValorProm = sum2 / Muestras;
        Serial.println(ValorProm, 3);
        // -------------------------------------------

        // ------- Lectura del LVDT 1
        sum3 = 0;
        TiempoInicial = millis();
        Muestras = 0;

        while (millis() - TiempoInicial < 100) {
          Muestras++;
          sum3 += LectLVDT1();
        }
        ValorProm = sum3 / Muestras;
        Serial.println(ValorProm);
        // -------------------------------------------

        // ------- Lectura del LVDT 2
        sum4 = 0;
        TiempoInicial = millis();
        Muestras = 0;

        while (millis() - TiempoInicial < 100) {
          Muestras++;
          sum4 += LectLVDT2();
        }
        ValorProm = sum4 / Muestras;
        Serial.println(ValorProm);
        // -------------------------------------------
      }
    }
  }
}

//Funcion para la lectura de Mitutoyos
float LectMitutoyo(int req, int clk, int dat) {
  digitalWrite(req, HIGH); // Generando el pulso (disparo) de lectura
  for (i = 0; i < 13; i++) {
    k = 0;
    for (j = 0; j < 4; j++) {
      // Mantener la lectura hasta que el reloj (de Mitutoyo) esté en alto
      while (digitalRead(clk) == LOW) {}
      // Mantener la lectura hasta que el reloj (de Mitutoyo) esté en bajo
      while (digitalRead(clk) == HIGH) {}
      bitWrite(k, j, (digitalRead(dat) & 0x1));
    }
    mydata[i] = k;
  }

  signo = mydata[4];
  value_str = String(mydata[5]) + String(mydata[6]) + String(mydata[7]) + String(mydata[8]) + String(mydata[9]) + String(mydata[10]);
  decimal = mydata[11];
  units = mydata[12];

  value_int = value_str.toInt();
  if (decimal == 0) dpp = 1.0;
  if (decimal == 1) dpp = 10.0;
  if (decimal == 2) dpp = 100.0;
  if (decimal == 3) dpp = 1000.0;
  if (decimal == 4) dpp = 10000.0;
  if (decimal == 5) dpp = 100000.0;

  value = value_int / dpp;

  if (signo == 0) {
    return value;
  }

  if (signo == 8) {
    return value * -1.0;
  }
}

//Función para la lectura de LVDT - 1
float LectLVDT1() {
  if (!band1) {
    ValInicio1 = balanceador(analogRead(A0), 0, 1023, 0, 100);
    band1 = true;
  }

  resultLVDT1 = balanceador(analogRead(A0), 0, 1023, 0, 100);
  resultLVDT1 = resultLVDT1 - ValInicio1;

  return resultLVDT1;
}

//Función para la lectura de LVDT - 2
float LectLVDT2() {
  if (!band2) {
    ValInicio2 = balanceador(analogRead(A1), 0, 1023, 0, 100);
    band2 = true;
  }

  resultLVDT2 = balanceador(analogRead(A1), 0, 1023, 0, 100);
  resultLVDT2 = resultLVDT2 - ValInicio2;

  return resultLVDT2;
}

float balanceador(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
