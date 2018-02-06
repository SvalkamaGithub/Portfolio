//v1.1 timer fix, antihotwire
//v1.2 safemodefailure fix, resetointi loppuun
//v2.0 14.10 "alipaine" paine-eroksi selvyyden vuoksi, kahden MISO:n SPI, selvennetyt muuttujannimet, testitavaran pois karsiminen, uusittu outputin formaatti, LÄMPÖOHJAUS
//v2.1 15.10 slaven resetti ja tauko eivät muuta myös asetuksia, en- ja dekoodauksen välimerkkikirjaimien uusinta
//v2.2 18.10 loppu ja alkumerkki outputtiin
//v2.3 25.10 kalibrointi 

SYSTEM_MODE(MANUAL);

#include <ESP8266WiFi.h>

int binhelp = 1024;
int i=0;


String progtimein = "0"; //vastaanotettu ajastus
int progtime = 0; //ohjelman aika
int timepast = 0;
int minsleft = 0;
int hleft = 0;




String progpressurein = "0"; //vastaanotettu paine 
int progpressure = 0;
int pressure = 0;
String maxdin = "0"; //hystereesi
int maxdeviation = 10;
int outpressure = 0;//mbaareiksi muutettu



String progtemperaturein = "0";
int progtemperature = 0; //lämpötilatavoite
int temperature = 0;
String maxtdin ="0";//hystereesilämpö
int maxtdeviation = 10;
int outtemp = 0;


int pause =0;//myös ohjelman status, 0=toiminnassa, 1=tauko, 2=ohjelma loppu
int tinyrst=0; //slaven resetointisignaali


int cycles = 0; //pumppaussylien laskenta, pussin vuodon vakavuuden määritykseen
int cycleen = 0;

String input = ""; //inputti formaattiin android softassa: paineasetus a painehystereesi b lämpöasetus c lämpöhystereesi d aikaasetus e tinyn resetointi f  keskeytys g
char *output = ""; //output????????????
char outhelp[60];

char SSID[] = "panoulu";    // SSID
char passwd[] = ""; // network password

void setup(){
  pinMode(1, OUTPUT); //sisäinen ledi, ei saa olla maissa käynnistyksessä


  pinMode(3, OUTPUT); //paine aktivointi 
  pinMode(4, OUTPUT); //ulkoinen ledi 
  pinMode(5, OUTPUT); //lämmitys aktivointi

pinMode(6, OUTPUT); //slave rst
 pinMode(7, INPUT); //"mosi"/ miso2
  pinMode(8, INPUT); //miso
  pinMode(9, OUTPUT); //clk spi

   
  Particle.function("settings_in", settings); //saapuva yhteys
  Particle.variable("info_out", outhelp); //lähtevä yhteys

  WiFi.persistent(false);
  WiFi.begin_internal(SSID, passwd, 0, NULL);
  
    while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(1, HIGH);
    delay(100);
    digitalWrite(1, LOW);
    delay(100);
  }
  Particle.connect();

  
 // slaven "alustus" resetti
 digitalWrite(6, LOW);
 digitalWrite(9, LOW);
 delay(10);
 digitalWrite(6, HIGH);


 
}//setup loppu---------------------------------------------
//funktiot--------



void reon(){
                    //rele päälle paine

    digitalWrite(3, HIGH);


}
void reoff(){
                     //rele pois paine

    digitalWrite(3, LOW);

}

void heaton(){
  digitalWrite(5, HIGH); //lämmityksen rele
}

void heatoff(){
  digitalWrite(5, LOW); //lämmityksen rele
}

int settings(String arg){  //UI inputin dekoodaus, pilven kautta kutsuttava funktio, joka vastaanottaa puhelimen lähetyksen-----------------------------------------
  input = arg;  
  if(1 == (input.substring((input.indexOf('e')+1),input.indexOf('f'))).toInt()){//pelkkä tinyrst
    tinyrst=1;

  }
  else{ 
      if(1==(input.substring((input.indexOf('f')+1),input.indexOf('g'))).toInt()){//pelkkä pause
         if(pause!=1) pause=1;
         else pause=0;   
      }
      else{

        progpressurein = input.substring(0,input.indexOf('a')); 
        maxdin = input.substring((input.indexOf('a')+1),input.indexOf('b')); 
        progtemperaturein = input.substring((input.indexOf('b')+1),input.indexOf('c')); 
        maxtdin = input.substring((input.indexOf('c')+1),input.indexOf('d'));
        progtimein = input.substring((input.indexOf('d')+1),input.indexOf('e'));
      
        
        progtime = progtimein.toInt();
        progpressure = progpressurein.toInt();
        progtemperature =progtemperaturein.toInt();
        maxdeviation = maxdin.toInt();
        maxtdeviation = maxtdin.toInt();
        
      
        progtime = progtime*36000; //huom! 1 = 100ms
      }
  }
}


void loop() { //mainloop----------------------------------------------------------------------------------------------------------------------------------------------------------------
  pause=0;
 if (Particle.connected() == false) {//automaattinen yhdeyden uudelleenmuodostus
    
    WiFi.begin_internal(SSID, passwd, 0, NULL);  //yhteys todennäköisemmin poikki wlaniin, palauta yhteys
  }
 if (Particle.connected() == false) {
    
    Particle.connect(); //yhteys poikki particleen, palauta yhteys
 }
 Particle.process(); //tärkeä! prosessoi käyttöliittymän viestit




 
//custom "SPI"-----------------------------------------------------------------------------------------------------------

delay(52); //slavelle aikaa tehdä mittaukset sekä syklin kesto------------------
temperature=0;
pressure=0;
binhelp=1024;
for (i = 0; i < 10; i = i + 1){ //10 pulssia sisältäen mittaustulokset
 binhelp=binhelp/2;
 digitalWrite(9,HIGH);
 delay(2);//odotetaan slaven reaktiota(hitaampikelloinen) + varmistusaika



 digitalWrite(9,LOW);
 delay(2); 
 if(digitalRead(7)==HIGH) temperature=temperature+binhelp;
 if(digitalRead(8)==HIGH) pressure=pressure+binhelp;
}
for (i = 0; i < 2; i = i + 1){//pari lisäpulssia, desyncin varalle, slaven ei pitäisi välittää näistä ollessa syncissä
digitalWrite(9,HIGH);
 delay(2);



 digitalWrite(9,LOW);
 delay(2); 
  
}

//delay(100);
//kalibroinnit
//pressure=analogRead(A0); //aktivoi tämä sensoritestiä varten

pressure=1024-pressure; //full scale 0.2v-4.6v 1,15bar-0bar 40-942 eli full scale=902,1,15bar, tässä käännetty: isompi luku on isompi paine-ero
pressure=pressure-247; //paine-ero 0 bar offsetattu nollaan
pressure=pressure*1.7; // 1.546*1.1 korjaus 10%yli oikean lukeman
outpressure=pressure; 

temperature=1024-temperature;//100k vastuksella full scale 2.2v 5c, 0,5v 60c, 450-102, full scale 348, 55c, 0,158c/1 käännetty: isompi luku on isompi lämpö
temperature=temperature-542;//-574+32, 32 arvioitu lukemaero jos sensori pääsee nollaan, offsetattu arvioituun nollaan
temperature=temperature*0.158; //kalibrointi
  outtemp = temperature; 
 
 
 
 if(progtime>timepast){//käyttöloop, aktiivinen kun ohjelmassa aikaa jäljellä
  digitalWrite(4,HIGH);//merkkiledi päällä
   
  //----------------------------------------------------------- käytönaikaiset on ja off paineelle
  if(pressure> progpressure){
   reoff(); //paine tavoitteessa
   
    cycleen=0;

  }
  
  else if(pressure < (progpressure-maxdeviation)){
     reon(); //pumpun käynnistys
   
    if(cycleen==0){ //pumppaussyklien laskenta
      cycles = cycles+1;
    }
    cycleen=1;
    

  }//-------------------------------------------------------lämmityksen aktivointi
  
  if(temperature<(progtemperature-maxtdeviation)){
    heaton();
            
  }
  else if(temperature>progtemperature){
    heatoff();
  }
  //------------------------------------------------------ajanlasku, huom! 1=100ms, 100ms syklinkesto
  timepast= timepast + 1;
  //------------------------------------------------------------yhteenveto ja output
  minsleft = (progtime-timepast)/600;
  hleft=minsleft/60;
  minsleft=minsleft%60;


  


 }
 else{//työn lopetus, nollaus
  reoff();
  heatoff();
  timepast=0;
  progtime=0;
  pause=2;
  cycles=0;
  Particle.process();
  digitalWrite(4,HIGH);
  delay(500);
  digitalWrite(4,LOW);
  delay(500);
  
 }
 ( 'x' + progpressurein + 'a' + maxdin + 'b' + outpressure + 'c' + progtemperaturein + 'd' + maxtdin + 'e' + outtemp + 'f' + progtimein + 'g' + hleft + 'h' + minsleft + 'i' + cycles + 'j' + pause + 'z').toCharArray(outhelp,60);
 //käyttöliittymälle lähtevä info, järjestys on tärkeä
 while(pause==1){ //tauko
  heatoff();
  reoff();
 Particle.process();  
   digitalWrite(4,HIGH);
  delay(100);
  digitalWrite(4,LOW);
  delay(100);
 
 }
 if(tinyrst==1){
      digitalWrite(6, LOW);
     digitalWrite(9, LOW);
     delay(10);
     digitalWrite(6, HIGH);
     delay(100);
     tinyrst=0;
 }

}
