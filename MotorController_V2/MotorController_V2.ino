////////////////////////////////////////////////////////////////////////////////
#include <PID_v1.h>
#include <string.h>
#include <stdio.h>

//Configuración de pines "Monster Motor Shield"
//Fuente: https://cdn.sparkfun.com/datasheets/Dev/Arduino/Shields/MonsterMoto-Shield-v12.pdf
//Necesario usar una fuente mayor a 5.5V para la alimentación del motor. Se usará una de 12V para el proyecto

#define EN1 A0  //Hablita/Deshabilita el motor 1
#define CS1 A2  //Sensor de corriente del motor 1
#define CW1 7   //Sentido horario motor 1
#define CCW1 8  //Sentido antihorario motor 1
#define PWM1 5  //PWM motor 1

/*Tablas de verdad para el movimiento del motor
Paro
    CW1 = 0 && CCW = 0 || CW = 1 && CCW = 1
Sentido Horario (Frente)
    CW = 1 && CCW = 0
Sentido Anti-Horario (Reversa)
    CW = 0 && CCW = 1
*/

//Configuración de pines "HN3806-AB-600N Incremental Rotary Encoder 600 P/R (Quadrature)
//Fuente: https://www.microjpm.com/products/ad38348/
#define PhA 2   //Cable blanco
#define PhB 3   //Cable verde
//Con el motor que tiene el encoder integrado se hacen 0.07575 grados por pulso

////////////////////////////////////////////////////////////////////////////////
//Variables globales
int i = 0;
char comando[20];
char cadena[40];

int pwm_step = 10;  //Pasos de Aumento/Decremento
int pwm = 105;      //Velocidad incial
int lim_inf = 0;    //Limite de velocidad inferior
int lim_sup = 255;  //Límite de velocidad superior

int ppr = 2376;     //Pulsos por revolución
int PosEnc = 0;     //Posición del encoder en pulsos, se inicia en 0
int MSB = 0;        //Variable para el valor del bit más significativo del encoder
int LSB = 0;        //Variable para el valor del bit menos significativo del encoder
int lastEncoded = 0;

int moveAngle = 0;              //Grados que se van a mover el motor
int error = 50;                 //Rango minimo para comparar la posición actual y la deseada
float ratio = (float)360/ppr;   //360/ppr;  //Pulsos por grado

//Valores iniciales del PID
double kp = 3, ki = 12, kd = .24;
double input = 0, output = 0, set_point = 0;
//double kp = 3, ki = 12, kd = .25;
//Uso de la librería PID
PID myPID(&input, &output, &set_point, kp, ki, kd, DIRECT);
/* Parametros de PID
  input -----> variable que se intenta controlar
  output ----> variable que será ajustada por el controlador
  set_point -> valor que se desea mantener en la entrada(input)
*/

//Banderas
bool moving = false;
bool inverso = false;
bool entradaCompleta = false;

////////////////////////////////////////////////////////////////////////////////
//Declaración de las funciones utilizadas
void menu(void);    //Desplega el menu en la consola
void accion(void);  //Selecciona una acción según el comando recibido (String)
void controller();  //Actualiza el controlador
void moveMotor(double out, int pwm_); //Mueve el motor según el controlador
double remap(int grades);

////////////////////////////////////////////////////////////////////////////////
//Setup del microcontrolador
void setup() {
  //Inicializamos las variables correspondientes al motor 1
  pinMode(EN1, OUTPUT);
  pinMode(CS1, OUTPUT);
  pinMode(CW1, OUTPUT);
  pinMode(CCW1, OUTPUT);
  pinMode(PWM1, OUTPUT);

  //Definimos resistencias de tipo pull-up en las entradas del encoder
  pinMode(PhA, INPUT_PULLUP);
  pinMode(PhB, INPUT_PULLUP);

  //Prendemos las resistencias pull-up
  digitalWrite(PhA, HIGH);
  digitalWrite(PhB, HIGH);

  //Definimos las interrupciones que controlan el encoder
  attachInterrupt(digitalPinToInterrupt(PhA), isrEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(PhB), isrEncoder, CHANGE);

  TCCR1B = TCCR1B & 0b11111000 | 1;  // Cambiamos la frecuencia del PWM a 31KHz para prevenir ruido

  myPID.SetMode(AUTOMATIC);       //Se coloca el PID en modo automático
  myPID.SetSampleTime(10);        //Tiempo entre actualización del controlador
  myPID.SetOutputLimits(0, 75);   //Limites de velocidad (PWM) que usa el controlador

  Serial.begin(9600);
  menu();                   //Imprimimos el menú de opciones
  digitalWrite(EN1, HIGH);  //Habilitamos el motor 1

}
//Ciclo principal
void loop() {
  if(entradaCompleta){
    //Se ejecuta si se envia un comando
    accion();
    entradaCompleta = false;
  }

  if(moving){
    //Si se desea mover una cantidad de grados definida
    controller();
  }
}
//Se envian datos desde el puerto serial
void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    if(inChar > 96){
      inChar -= 32;
    }
    if (inChar != '\n') { //Terminador de cadena (enter)
      comando[i] += inChar;
      i++;
    }else{
      i = 0;
      entradaCompleta = true;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
//Interrupción del encoder
void isrEncoder(){
  MSB = digitalRead(PhA);
  LSB = digitalRead(PhB);
  //Convertir el valor de los dos pines a un solo numero
  int encoded = (MSB << 1) | LSB;
  //Se agregan al valor anterior del encoder
  int sum  = (lastEncoded << 2) | encoded;
  //Se verifica si es en sentido horario (sumar) o antihorario (restar)
  if(sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011) PosEnc ++;
  if(sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000) PosEnc --;
  //Guarda el valor para la próxima interrupción
  lastEncoded = encoded;

  //Si se llega a las ppr se reinicia el contador de posicion
  if(PosEnc > ppr - 1){
    PosEnc = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////
//Inicialización de las funciones
void menu(){
  Serial.println("Control del motor (Funciones básicas)");
  Serial.println("");
  Serial.println("Comandos:");
  Serial.println("[S]    PARO");
  Serial.println("[F]    ADELANTE (CW)");
  Serial.println("[R]    REVERSA (CCW)");
  Serial.println("[IS]   INCREMENTAR VELOCIDAD");
  Serial.println("[DS]   DISMINUIR VELOCIDAD");
  Serial.println("[MT X] MOVER X GRADOS");
  Serial.println("");
  Serial.println("Opciones:");
  Serial.println("[GP]   MOSTRAR POSICION ACTUAL DEL ENCODER (GRADOS)");
  Serial.println("[GS]   MOSTRAR VELOCIDAD (PWM)");
  Serial.println("[MN]   IMPRIMIR LISTA DE COMANDOS/OPCIONES");
  Serial.println("");
}

void accion(){
  if(!strcmp(comando, "S")) {
    Serial.println("PARO - Se ha establecido a 0 la posicion");
    PosEnc = 0;
    digitalWrite(CW1, LOW);
    digitalWrite(CCW1, LOW);
  }

  if(!strcmp(comando, "F")){
    Serial.println("ADELANTE (CW)");
    digitalWrite(CW1, HIGH);
    digitalWrite(CCW1, LOW);
  }

  if(!strcmp(comando, "R")){
    Serial.println("REVERSA (CCW)");
    digitalWrite(CW1, LOW);
    digitalWrite(CCW1, HIGH);
  }

  if(!strcmp(comando, "IS")){
    pwm += pwm_step;
    if(pwm > lim_sup) pwm = lim_sup;
    Serial.print("VELOCIDAD ");
    Serial.println(pwm);
  }

  if(!strcmp(comando, "DS")){
    pwm -= pwm_step;
    if(pwm < lim_inf) pwm = lim_inf;
    Serial.print("VELOCIDAD ");
    Serial.println(pwm);
  }

  if(!strncmp(comando, "MT", 2)){
    char aux[5];
    sscanf(comando, "%s %d", aux, &moveAngle);
    sprintf(cadena, "Se debe mover: %d grados", moveAngle);
    Serial.println(cadena);

    if (moveAngle < 0){
      //Habilita el movimiento con ángulos "negativos" (CCW)
      inverso = true;
      moveAngle = (-1)*moveAngle;
    }

    set_point = map(moveAngle, 0, 360, 0, ppr);
    moving = true;
  }

  if(!strcmp(comando, "GP")){
//    Serial.print("POSICION ACTUAL (GRADOS) ");  //Esto se comenta cuando se conecta con la interfaz de Python
    Serial.println(float(PosEnc)*ppr, 5);
  }

  if(!strcmp(comando, "GS")){
//    Serial.print("PWM ACTUAL ");  //Esto se comenta cuando se conecta con la interfaz de Python
    Serial.println(pwm);
  }

  if(!strcmp(comando, "MN")){
    menu();
  }

  memset(comando, 0, 20);
  analogWrite(PWM1, pwm);
}

void controller(){
  input = abs((double)PosEnc);
  myPID.Compute();
  Serial.print("Posición: ");
  Serial.println(PosEnc);
  moveMotor(set_point - PosEnc, output);

  if(set_point - abs(PosEnc) < error){
    moving = false;
    inverso = false;
    moveMotor(0, pwm);
    Serial.print("Posición actual: ");
    Serial.println(PosEnc);
    Serial.print("Posición deseada: ");
    Serial.println(set_point);
    Serial.print("Diferencia: ");
    Serial.println(abs(set_point - abs(PosEnc)));
    Serial.println("");
    delay(500);
    Serial.println(errorGrados(abs(set_point - abs(PosEnc))));
    PosEnc = 0;
  }
}


void moveMotor(double out, int pwm_){
  // Condición de parada
  if (out == 0){
    digitalWrite(CW1, LOW);
    digitalWrite(CCW1, LOW);
    Serial.println("MOTOR: OFF");
  } else{
    // Condiciones de movimiento
    if(!inverso){
      digitalWrite(CW1, HIGH);
      digitalWrite(CCW1, LOW);
      Serial.println("MOVIMIENTO: CW");
    } else{
      digitalWrite(CW1, LOW);
      digitalWrite(CCW1, HIGH);
      Serial.println("MOVIMIENTO: CCW");
    }
  }

  analogWrite(PWM1, abs(pwm_));
}

float errorGrados(int pulsos){
  return (float)pulsos/ratio;
}
// *FIN DEL CÓDIGO
