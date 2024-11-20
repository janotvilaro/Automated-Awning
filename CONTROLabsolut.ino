#include <Stepper.h>; //INSTALAR LIBRARY ABANS DE COMENÇAR


// MOTOR
const int ONEA = 2;
const int ONEB = 3;
const int TWOA = 4;
const int TWOB = 5;
// Els quatre pins connectats al motor
const int passos = 30;  //Passos que fara cada cop, el núm. de moment és per provar
bool tendal = 0; //Si està estés o recollit
Stepper motor(48,ONEA,ONEB,TWOA,TWOB); //48 passos per volta

//BOTO I MODE
const int buttonPin = 5; // Pin connectat al motor
int previosbuttonstate = 0;
int buttonstate = 0;
bool mOde = false; //Si està ences o no el LED, en quin mode està
unsigned long ultimRebot = 0; //Temps des de l'últim contacte (debouncing)
unsigned long IntervalRebot = 50; //Temps que ha de passar per a produir-se una transició (ms) (debouncing)
void CANVIestat(int boto); //Funció de canvi d'estat a l'apretar el botó

// VENT
const int pinVent = 7; //Pin connectat al sensor de vent
unsigned long IntervalVent = 1000000; //Temps sobre el que es promitja la velocitat (en microsegons)
float Velocity; // Velocitat en m/s (és un valor decimal)
int Velocity1 = 10; // Velocitat vent per a estendre el tendal
int Velocity2 = 15; // Velocitat vent per a recollir el tendal
bool perillVent; //Indica si hi ha perill per fort vent
float CONTROLVent(int pinV, unsigned long IntervalV); //Funció que mesura la velocitat del vent
//Ni idea dels valors de vent

// TEMPERATURA
const int tempPin = A0; // Pin connectat al sensor de temperatura
int Vtemp = 0; //Tensió de temperatura rebuda
int Vtemp1 = 200; // Tensio de temperatura per estendre el tendal
int Vtemp2 = 150; // Tensio de temperatura per recollir el tendal
//Ni idea dels valors de temperatura

// LLUM
const int llumPin = A1; // Pin connectat al fototransistor
int Vllum = 0; //Tensió de llum rebuda
int Vllum1 = 200; // Tensio de llum per estendre el tendal
int Vllum2 = 150; // Tensio de llum per recollir el tendal
//Ni idea dels valors de llum


void setup() {
  motor.setSpeed(100);  

  pinMode(buttonPin,INPUT_PULLUP);  // input pullup, així no cal tant de circuit
  pinMode(13,OUTPUT);   // LED de la placa

  pinMode(pinVent,INPUT); // potser cal input pullup, s'haurà de veure amb el setup
}

void loop() {

  CANVIestat(buttonPin); //Mirem en quin mode està
  Velocity = CONTROLVent(pinVent, IntervalVent); //Mesurem velocitat del vent

  while (perillVent == LOW){

    if (mOde == true){ //Mode calor
      Vtemp = analogRead(tempPin); //Llegim temperatura
      Vllum = analogRead(llumPin); //Llegim llum

      if ((Vtemp > Vtemp1) && (tendal == LOW) && (Vllum > Vllum1)){ //Tendal està recollit
        motor.step(passos); //en un sentit, STEP ES FUNCIO BLOQUEJANT
        tendal = HIGH; //Tendal està estés
      } 
      else if ((Vtemp < Vtemp2) && (tendal == HIGH) && (Vllum < Vllum2)){ // Tendal està estés
        motor.step(-passos); //en l'altre sentit
        tendal = LOW; //Tendal està recollit
      }
    }
    else if (mOde == false){ //Mode llum
      Vllum = analogRead(llumPin); //Llegim llum

      if ((tendal == LOW) && (Vllum > Vllum1)){ //Tendal està recollit
        motor.step(passos); //en un sentit, STEP ES FUNCIO BLOQUEJANT
        tendal = HIGH; //Tendal està estés
      } 
      else if ((tendal == HIGH) && (Vllum < Vllum2)){ // Tendal està estés
        motor.step(-passos); //en l'altre sentit
        tendal = LOW; //Tendal està recollit
      }
    }
  }
  //CONTROL SEGURETAT VENT
  if (Velocity < Velocity1){ //Vent per sota de la cota inferior, tendal funciona
    perillVent = LOW;
  }
  if (Velocity > Velocity2){ //Vent per sobre de la cota superior, tendal es para
    perillVent = HIGH;
    if (tendal == HIGH){
      motor.step(-passos); 
      tendal = LOW; //Tendal està recollit
    } 
  }
}

//CANVI DE MODE
void CANVIestat(int boto){
  buttonstate = digitalRead(boto);
  if(millis() - ultimRebot > IntervalRebot){
    if ((buttonstate == LOW) && (previosbuttonstate == HIGH)) {
      previosbuttonstate = LOW;
      mOde = !mOde; //canviem de mode
      digitalWrite(13,mOde); // canviem color LED
      ultimRebot = millis(); // actualitzem temps de l'últim contacte
      }
    else if (buttonstate == HIGH){
        previosbuttonstate = HIGH;
      }
    }
  }

//MESURA VELOCITAT VENT
float CONTROLVent(int pinV, unsigned long IntervalV){
  int VentState;
  int PREVVentState = LOW;
  int cont = 0; // Comptador de polsos de vent
  unsigned long prevVentTime = 0;
  float Velocity = 0;
  
  while (micros()-prevVentTime < IntervalV){
    VentState = digitalRead(pinV);
    if ((VentState == HIGH) && (PREVVentState == LOW)){
      cont++;
      PREVVentState = HIGH;
    }
    else if (VentState == LOW){
      PREVVentState = LOW;
    }
    }
  if (micros()-prevVentTime == IntervalV){
    Velocity = (0.0875*cont)/(IntervalVent/1000000);  // 0.0875 és la resolució del sensor (m/s)
    cont = 0;
    prevVentTime = micros();
  }
  return Velocity;
}
