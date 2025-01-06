 #include <ACS712.h>

/* Connection de la carte BTS7960: 
 BTS7960 Pin 1 (RPWM) vers Arduino pin 5(RPWM)vert 4 de H8
 BTS7960 Pin 2 (LPWM) vers Arduino pin 6(LPWM)blanc 3 de H8
 
 BTS7960 Pin 3 (R_EN) vers Arduino pin 5v (R_EN)rouge
 BTS7960 Pin 4 (L_EN) vers Arduino pin 5v (L_EN)rouge
 BTS7960 Pin 5 (R_IS)NC
 BTS7960 Pin 6 (L_IS)NC
 BTS7960 Pin 7 (VCC) vers Arduino pin 5V rouge
 BTS7960 Pin 8 (GND) vers Arduino pin GND noir
 
*/

//*****************************************************************

#include <ACS712.h>

float Courant_inf=0.5;
float Val_Acs=0,Echantillon=0,Moy_Val_Acs=0,Acs_712=0;
unsigned long timer;
const float sensibilite=0.100;//capteur 5A/185mv  20A/100mv  30A/66mv

//*********************************variables à parametrer****************************
int compteur_vitesse=20;// definit le temps d'ouverture/fermeture à grande vitesse
int compteur_courant=50;// definit le temps de calcul du courant
float Courant_sup;// définit le courant maximum dans le moteur
int attente;// definit le temps d'attente avant l'ouverture/fermeture
int impulsion_ouv=500;//definit l'impulsion à donner après contact sur le FDCO
int impulsion_fer=200;//definit l'impulsion à donner après contact sur le FDCF

//************************************fin *********************************************
int x;
int i;
int adc;
float sensor;
int ValVitesse;
int lumiere=7;//pin commande auto de l'eclairage à l'ouverture et fermeture
int RPWM= 5;//pin commande moteur ouverture 
int LPWM= 6;//pin commande moteur fermeture
int Bp_Ouv=11;//pin commande manuelle ouv bouton poussoir ouverture
int Bp_Fer=4;//pin commande manuelle fer bouton poussoir fermeture
int Bp_lumiere_auto=3;//pin commande lumiere auto
int Bp_lumiere_manu=10;//pin commande manuelle lumiere bouton poussoir à la fermeture
int Cde_lum_auto;
int Cde_lum_manu;
int Etat_lum=0;
int FDC_ouv=8;//pin fin de course ouverture
int FDC_fer=9;//pin fin de course fermeture
int Cde_Ouv; //bouton commande ouverture
int Cde_Fer;// bouton commande fermeture
int Cde_FDC_ouv;
int Cde_FDC_fer;


//******************************************************void setup***************************************
void setup()
{
// initialisation de la communication série
 Serial.begin(9600);
 
 pinMode(A0, INPUT);
 pinMode(lumiere, OUTPUT);
 pinMode(Bp_Ouv, INPUT_PULLUP);
 pinMode(Bp_Fer, INPUT_PULLUP);
 pinMode(Bp_lumiere_manu, INPUT_PULLUP);
 pinMode(Bp_lumiere_auto,INPUT_PULLUP);
 pinMode (FDC_ouv, INPUT_PULLUP);
 pinMode (FDC_fer, INPUT_PULLUP);
 pinMode(RPWM, OUTPUT);
 pinMode(LPWM, OUTPUT);
//init du moteur à l'arret 
 analogWrite(RPWM,0); 
 analogWrite(LPWM,0);
 digitalWrite(lumiere,0);
}
//***********************************************************void loop************************************************************  
void loop()
{
  Cde_Ouv= digitalRead (Bp_Ouv);
  Cde_Fer= digitalRead (Bp_Fer);  
  
  while ((Cde_Ouv ==1)and (Cde_Fer==1)){
    Cde_Ouv= digitalRead (Bp_Ouv);
    Cde_Fer= digitalRead (Bp_Fer);
    Serial.print("Attente ouv : ");
    Serial.println(Cde_Ouv);
    Serial.print("Attente fer : ");
    Serial.println(Cde_Fer);
    Cde_lum_auto= digitalRead (Bp_lumiere_auto);
    Cde_lum_manu= digitalRead (Bp_lumiere_manu);
    Cde_FDC_ouv= digitalRead (FDC_ouv);
    Cde_FDC_fer= digitalRead (FDC_fer);
    if (Cde_lum_manu==0){
     Etat_lum=1;
     digitalWrite(lumiere,Etat_lum);}
     else if (Cde_lum_manu==1 && Cde_lum_auto==1 ){
     Etat_lum=0;}
    digitalWrite(lumiere,Etat_lum);
    
  }
   //***************************************************************************************************************************  
  Cde_Ouv= digitalRead (Bp_Ouv);
  if (Cde_Ouv ==0){
 Serial.print("Cde_Ouv : ");
 Serial.println (Cde_Ouv);
 Cde_ouverture_porte();
}
  //***************************************************************************************************************************
Cde_Fer= digitalRead (Bp_Fer);
if (Cde_Fer==0){
 Serial.print("Cde_Fer : ");
 Serial.println (Cde_Fer);
Cde_fermeture_porte();
}
} 
//**************************************************commande ouverture grande vitesse **********************
void Cde_ouverture_porte(){
  Cde_FDC_ouv= digitalRead (FDC_ouv);
  if (Cde_FDC_ouv==0){
  analogWrite(RPWM,0);
  return;
  }
  delay (attente);
 Cde_lum_auto= digitalRead (Bp_lumiere_auto);
  if (Cde_lum_auto==0){
  Etat_lum=1;
  digitalWrite(lumiere,Etat_lum); 
  }
 
   ValVitesse=255;
   Courant_sup=3.0;
 for (i = 0; i <= compteur_vitesse; i++){
  analogWrite(RPWM,ValVitesse);
  Serial.println("ouverture de porte");
  delay (30);
  Calcul_Courant();
  Serial.print ("grande vitesse  ");
  Serial.println( ValVitesse);
  }
 //*******************************************commande ouverture petite vitesse**********************************************
  ValVitesse=100;
  Courant_sup=1.3;
  analogWrite(RPWM,ValVitesse);
 Cde_FDC_ouv= digitalRead (FDC_ouv);
 while (Cde_FDC_ouv==1){
  analogWrite(RPWM,ValVitesse);
  Calcul_Courant();
  Serial.print( "petite vitesse : ");
  Serial.println( ValVitesse);
  Cde_FDC_ouv= digitalRead (FDC_ouv);
  Serial.println(Cde_FDC_ouv);
  }
 Serial.println("la porte est ouverte ");
 analogWrite(RPWM,0);
 Cde_lum_auto= digitalRead (Bp_lumiere_auto);
// Cde_FDC_ouv= digitalRead (FDC_ouv);
 delay (impulsion_ouv);
  if(Cde_lum_auto==0){
    Etat_lum=1;
    digitalWrite(lumiere,Etat_lum);}
 }

//**************************************************commande fermeture grandevitesse**********************²
void Cde_fermeture_porte(){
Cde_lum_auto= digitalRead (Bp_lumiere_auto);
 if(Cde_lum_auto==0){
  Etat_lum=0;
  digitalWrite(lumiere,Etat_lum);}
 Cde_FDC_fer= digitalRead (FDC_fer);
  if (Cde_FDC_fer==0){
  analogWrite(LPWM,0);
  return;
  }
  delay(attente);
  ValVitesse=255;
  Courant_sup=3.0;
  for (i = 0; i <= compteur_vitesse; i++){
  analogWrite(LPWM,ValVitesse);
  Serial.println("Fermeture de porte"); 
  delay(30);
  Calcul_Courant();
  Serial.print ("grande vitesse  ");
  Serial.println( ValVitesse);
  }   
 //*******************************************commande fermeture petite vitesse**********************************************
  ValVitesse=100;
  Courant_sup=1.3;
  analogWrite(LPWM,ValVitesse);
 Cde_FDC_fer= digitalRead (FDC_fer);
  while (Cde_FDC_fer==1){
  analogWrite(LPWM,ValVitesse);
  Calcul_Courant();
  Serial.print( "petite vitesse : ");
  Serial.println( ValVitesse);
  Cde_FDC_fer= digitalRead (FDC_fer);
  Serial.println(Cde_FDC_fer);
  }
 delay(impulsion_fer);
 Serial.println("la porte est fermee ");
 analogWrite(LPWM,0);
  }
//************************************************************calcul du courant**********************************************

void Calcul_Courant(){

for ( x = 0; x <= compteur_courant; x++){ //boucle de  50 Echanillons
  adc = analogRead (A0);//lecture du courant sur A0
  delay (3); // attente avant prochain    
  Echantillon = Echantillon + adc; 
   }
  adc=Echantillon/compteur_courant;//moyenne des echantillons
  Acs_712 = ((2.5 - (adc * (5.0 / 1023.0))) /sensibilite);// calcul du courant
  Acs_712=abs(Acs_712);
  Echantillon=0,adc=0;

  if (Acs_712 < Courant_inf){
    Acs_712=0.0;
    }
  Serial.print( "courant ");
  Serial.println( Acs_712);
  if ((Acs_712 >Courant_sup)) {
    i=compteur_vitesse;
    analogWrite(LPWM,0);
    analogWrite(RPWM,0);
  Serial.print( "depassement_courant  Moteur arreté ");
  delay(5000);
  Serial.println( Acs_712);
   }
     }

 
