/*v10*/

#include <avr/io.h>
#include <avr/interrupt.h>
#include "lcd.h"
#include <stdio.h>
#include <stdlib.h>
#include <util/delay.h>
#define LCD_BLINK  0x01
#include <avr/eeprom.h>

#define LCD_START_LINE1  0x00     /* DDRAM address of first char of line 1 */
#define LCD_START_LINE2  0x40     /* DDRAM address of first char of line 2 */


/*prototyypit*/
int read_btn(void); 
int move_xy(int,int,int);
void game(void);
void place_ship(int,int);

/*globaalit muuttujat ja funktiot*/	

int ammo=10;

int nappi=5; /* 0-4 vastaavat painetun napin numeroa, 5 k‰ytet‰‰n nollana*/
int t=0; /* looppeihin */



int x=0;
int y=0;
char teksti[31]="Paina nappia    aloittaaksesi  ";
int ascii_to_dec[]={48,49,50,51,52,53,54,55,56,57}; /*k‰yt‰nnˆss‰ kyll‰ dec to ascii... mutta tulostaessa vastaa nime‰‰n*/
char kirjaimet[]="abcdefghijklmnopqrstuvwxyz"; 
int dec_to_char[]={97,98,99,100,101,102,103,104,105,106,107};

int alotus = 1;
int valikkoruutu_max = 4;

int highscore_ruudussa = 0;
int nimen_kysely_ruudussa = 0;
int vaikeustaso = 3;
int vaikeustason_kysely_ruudussa = 0;
int vaikeustaso_ruutu = 1;
int vaikeustaso_ruutu_max = 3;
int nimen_kirjain = 0;
char pelaajan_nimi[] = "          ";
int i = 0;
char highscore_pelaaja[]="          ";
int highscore=0;
char valikko_1[] = "Uusi peli>>";
char valikko_2[] = "<<Highscore>>";
char valikko_3[] = "<<Anna nimi>>";
char valikko_4[] = "<<Vaikeustaso";
char vaikeustaso_1[] = "Normaali>>";
char vaikeustaso_2[] = "<<Aika vaikia>>";
char vaikeustaso_3[] = "<<Mahoton";
int enter=11;
int apumuuttuja=0;
int uponneet=0; 
int aani=0;
unsigned int aika=0;

void lcd_gotoxy (unsigned char x, unsigned char y) 
	{
    if ( y==0 ) {
           lcd_write_ctrl(LCD_DDRAM | (LCD_START_LINE1+x));
    } 
	else {
           lcd_write_ctrl(LCD_DDRAM | (LCD_START_LINE2+x));
   		}
	}

int read_btn(void)
		{ /*nappien painamisen rekisterˆiv‰ funktio, luuppaa arvolla 5 napin painamiseen asti, antaa arvot0-4 nappuloiden mukaan*/
			nappi=5;
			_delay_ms(10);
			while(nappi==5){
				if(!(PINA & (1<< PA0))){
					nappi=0;
				}

				
				if(!(PINA & (1<< PA1))){	
					nappi=1;
				}

				
				if(!(PINA & (1<< PA2))){	
					nappi=2;
				}

				
				if(!(PINA & (1<< PA3))){	
					nappi=3;
				}

				
				if(!(PINA & (1<< PA4))){	
					nappi=4;
				}

			}
			
			return nappi;
		}


int move_xy(int nappi, int xmax, int ymax)
	{ /* muuttaa x ja y koordinaatteja annetun arvon mukaan, y koordinaatti nousee alasp‰in ment‰ess‰*/
	
	enter=0;
	switch(nappi)
		{

		case 2:
			{
			enter=1;
			return enter;
			}
				break;
		case 0:
			if(y>0){
				y--;				
				}
				break;
		case 1:
			if(x>0){
				x--;				
				}
				break;	
		case 3:
			if(x<xmax){
				x++;				
				}
				break;
		case 4:
			if(y<ymax){
				y++;				
				}
				break;
		return 0;
		}
	}

void trisound(int nuotti1, int nuotti2, int nuotti3)
	{
				
			ETIMSK|=(1<<OCIE3A);
			OCR3AL = nuotti1;
			_delay_ms(150);
			TCNT3=0;
			OCR3AL = nuotti2;
			_delay_ms(150);
			TCNT3=0;
			OCR3AL = nuotti3;
			_delay_ms(150);
			TCNT3=0;
			ETIMSK &= ~(1<<OCIE3A);
	
	}





int main(void){

   		
		
		

		cli();
        /* kaiutin pinnit ulostuloksi */
        DDRE  |=  (1 << PE4) | (1 << PE5);
        /* pinni PE4 nollataan */
        PORTE &= ~(1 << PE4);
        /* pinni PE5 asetetaan */
        PORTE |=  (1 << PE5);   
        
        /* ajastin nollautuu, kun sen ja OCR1A rekisterin arvot ovat samat */
        TCCR1A &= ~( (1 << WGM11) | (1 << WGM10) );
        TCCR1B |=    (1 << WGM12);
        TCCR1B &=   ~(1 << WGM13);

 

        /* asetetaan OCR1A rekisterin arvoksi 15625, sekuntilaskuria varten */
        OCR1AH = 0x3d;
        OCR1AL = 0x09;




        /* k√§ynnist√§ ajastin ja k√§yt√§ kellotaajuutena (16 000 000 / 1024) Hz */
        TCCR1B |= (1 << CS12) | (1 << CS10);

		/* n√§pp√§in pinnit sis√§√§ntuloksi */
		DDRA &= ~(1 << PA0);
		DDRA &= ~(1 << PA1);
		DDRA &= ~(1 << PA2);
		DDRA &= ~(1 << PA3);
		DDRA &= ~(1 << PA4);

		/* rele/led pinni ulostuloksi */
		DDRA |= (1 << PA6);

		/* lcd-n√§yt√∂n alustaminen */
		lcd_init();
		lcd_write_ctrl(LCD_ON);
		lcd_write_ctrl(LCD_CLEAR);

		/* ajastin 3 prescaletetaan 1024:ll‰*/
	    TCCR3A &= ~( (1 << WGM31) | (1 << WGM30) );
        TCCR3B |=    (1 << WGM32);
        TCCR3B &=   ~(1 << WGM33);
		
		TCCR3B |= (1 << CS12) | (1 << CS10);
		
		/*1000hz*/
		OCR3AH = 0x00; 
        OCR3AL = 0x10; 
		
		TIMSK |= (1 << OCIE1A);/*sallitaan kekskeytykset rekisterin arvolla*/

		SREG |= (1 << 7);



	

	trisound(50,35,20);
	trisound(20,35,50);
	
	y=0;	
	x=0;
while(1){    
	_delay_ms(250);
	enter=0;
	apumuuttuja=0;
	if(alotus!=1)apumuuttuja=read_btn();
	alotus=0;
	move_xy(apumuuttuja,3,0);
	if(apumuuttuja==2)
		enter=1;	
		
		
		
		

	switch(x){
	case 0:
		aani=0;
		lcd_write_ctrl(LCD_CLEAR);
		for(t=0; t<11; t++){
			lcd_write_data(valikko_1[t]);
			
		}

		if (enter == 1){

			_delay_ms(250);
			game();	            /*itse peli k‰yntiin t‰ss‰*/
			lcd_write_ctrl(LCD_CLEAR);
		
			_delay_ms(500);
			for(t=0; t<10; t++){

					
				lcd_write_data(pelaajan_nimi[t]);
					
					
			}
			lcd_write_data('t');
			if(aika>99){lcd_write_data(ascii_to_dec[aika/100]);
						lcd_write_data(ascii_to_dec[(aika%100)/10]);
			}			
			else{
				
				lcd_write_data(ascii_to_dec[aika/10]);
			}
			lcd_write_data(ascii_to_dec[aika%10]);
			if(uponneet!=6){
				lcd_gotoxy(0,1);
				lcd_write_data('f');
				lcd_write_data('a');
				lcd_write_data('i');
				lcd_write_data('l');	
				
				trisound(20,40,60);
			
			
			}
			else{
				lcd_gotoxy(0,1);
				lcd_write_data('w');
				lcd_write_data('i');
				lcd_write_data('n');
				lcd_gotoxy(12,0);
			
				lcd_write_data(ascii_to_dec[ammo/10]);
				lcd_write_data(ascii_to_dec[ammo%10]);
				if ((ammo > highscore)&&(vaikeustaso==1)){ 
					highscore = ammo;
					for(t=0; t<10; t++){
						highscore_pelaaja[t] = pelaajan_nimi[t];
					}
				}
				trisound(50,35,20);
			
			}
			
			
			read_btn();	       
			lcd_write_ctrl(LCD_CLEAR);
			enter=0;
			
			
			alotus=1;
			x=0;
			y=0;
		
		}
        

		break;
	case 1:
		aani=1;
		lcd_write_ctrl(LCD_CLEAR);
		for(t=0; t<13; t++){
			lcd_write_data(valikko_2[t]);
			
		}

		
			
		
		lcd_gotoxy(0,1);
		for(t=0; t<10; t++){
			lcd_write_data(highscore_pelaaja[t]);
	

		}
			
		
		lcd_write_data(ascii_to_dec[highscore/10]);	
		lcd_write_data(ascii_to_dec[highscore%10]);
		   
		_delay_ms(250);	
			
		if(highscore_ruudussa == 1 && (read_btn() == 2)){
			highscore_ruudussa = 0;
			lcd_write_ctrl(LCD_CLEAR);
			
		_delay_ms(250);
		
		}
		_delay_ms(250);
		enter=0;        
		
		break;	
		
    case 2:
		lcd_write_ctrl(LCD_CLEAR);
		for(t=0; t<13; t++){
			lcd_write_data(valikko_3[t]);
				
		}

		if (enter==1) {
			nimen_kysely_ruudussa = 1;
			nimen_kirjain = 0;
			_delay_ms(250);
			for(t=0; t<10; t++){
				lcd_write_data(pelaajan_nimi[t]=' ');
			}
			while(nimen_kysely_ruudussa == 1){

		



			lcd_write_ctrl(LCD_CLEAR);
				
				
				lcd_write_data(kirjaimet[i]);
				lcd_gotoxy(0,1);
				for(t=0; t<10; t++){
					lcd_write_data(pelaajan_nimi[t]);
				}
				if(nimen_kirjain == 10){
					nimen_kysely_ruudussa = 0;
				}
				
				else if(i>0 && (read_btn() == 1)){
					i--;
					_delay_ms(250);
				}
    			else if(i<25 && (read_btn() == 3)){
    				i++;
					_delay_ms(250);
				}
				else if (read_btn() == 2) { 
					pelaajan_nimi[nimen_kirjain] = kirjaimet[i];
					nimen_kirjain++;

					_delay_ms(250);
				}
				else if (read_btn() == 0){
					nimen_kysely_ruudussa = 0;
					_delay_ms(250);
				}
				else if (read_btn() == 4){
					nimen_kysely_ruudussa = 0;
					_delay_ms(250);
				}
			}
			alotus=1;
			_delay_ms(250);
		
		}
		
		enter=0;         
		
		break;
	case 3:   					/*vaikeustasokyselyruutu*/
		lcd_write_ctrl(LCD_CLEAR);
		for(t=0; t<13; t++){
			lcd_write_data(valikko_4[t]);
			
		}

		if (enter==1) {
			vaikeustason_kysely_ruudussa = 1;
			_delay_ms(250);
			while(vaikeustason_kysely_ruudussa == 1){/*luuppaa siihen asti ett‰ entteri‰ painettu ja muuttaa vaikeustason arvon sen mukaan*/
				switch(vaikeustaso_ruutu){
				case 1:
					lcd_write_ctrl(LCD_CLEAR);
					for(t=0; t<10; t++){
						lcd_write_data(vaikeustaso_1[t]);
						
					}
					lcd_gotoxy(0,1);
					lcd_write_data('4');
					lcd_write_data('0');
					if(vaikeustaso_ruutu>1 && (read_btn() == 1)){
						vaikeustaso_ruutu--;
						_delay_ms(250);
					}
					else if(vaikeustaso_ruutu<vaikeustaso_ruutu_max && (read_btn()==3)){
						vaikeustaso_ruutu++;
						_delay_ms(250);
					}
					else if (read_btn() == 2){
						vaikeustaso = 1;
						vaikeustason_kysely_ruudussa = 0;
						_delay_ms(250);
					}break;
						
				case 2:
					lcd_write_ctrl(LCD_CLEAR);
					for(t=0; t<15; t++){
						lcd_write_data(vaikeustaso_2[t]);
						
					}
					lcd_gotoxy(0,1);
					lcd_write_data('2');
					lcd_write_data('5');
					if(vaikeustaso_ruutu>1 && (read_btn() == 1)){
						vaikeustaso_ruutu--;
						_delay_ms(250);
					}
					else if(vaikeustaso_ruutu<vaikeustaso_ruutu_max && (read_btn()==3)){
						vaikeustaso_ruutu++;
						_delay_ms(250);
					}
					else if (read_btn() == 2){
						vaikeustaso = 2;
						vaikeustason_kysely_ruudussa = 0;
						_delay_ms(250);
					}break;
					
				case 3:
					lcd_write_ctrl(LCD_CLEAR);
					for(t=0; t<9; t++){
						lcd_write_data(vaikeustaso_3[t]);
						
					}
					lcd_gotoxy(0,1);
					lcd_write_data('1');
					lcd_write_data('0');
					if(vaikeustaso_ruutu>1 && (read_btn() == 1)){
						vaikeustaso_ruutu--;
						_delay_ms(250);
					}
					else if(vaikeustaso_ruutu<vaikeustaso_ruutu_max && (read_btn()==3)){
						vaikeustaso_ruutu++;
						_delay_ms(250);
					}
					else if (read_btn() == 2){
						vaikeustaso = 3;
						vaikeustason_kysely_ruudussa = 0;
						_delay_ms(250);
					}break;


				}
			}
			_delay_ms(250);
			enter=0;         
			alotus=1;
		}

	}
	}
}

void game(void) {
	/*muuttujat x ja y pidet‰‰n arvoissa 0-9 n‰ytelle tulostusta varten, joihin lis‰t‰‰n 1 tarkastellessa pelilautaa*/
	/* salli keskeytys, jos ajastimen ja OCR1A rekisterin arvot ovat samat */
    
	
	
	switch(vaikeustaso){
	case 1:
		ammo = 40;
		break;
	case 2:
		ammo = 25;
		break;
	case 3:
		ammo = 10;
		break;
	}
	
	lcd_write_ctrl(LCD_CLEAR);	
	
	_delay_ms(1000);
	for(t=0;t<31;t++){ /*pelin aloitus napinpainalluksella*/
	lcd_write_data(teksti[t]);
		if(t==15)lcd_gotoxy(0,1);
	}

	read_btn();
	srand(TCNT1);/*seedi randille*/

	
	/* Alustetaan eri alusten "osumapisteet" */
  
   int carrier = 5;
   int battleship = 4;
   int cruiser_1 = 3;
   int cruiser_2 = 3;
   int destroyer = 2;
   int submarine = 1;
   int suunta=0;
   int tilaa=0;
   int x_koord=0;
   int y_koord=0;
   int tunnus =0;
	

	char pelimerkki[]={32,42,35,33};/*n‰ill‰ muutetaan pelilaudan 0-3 pelaajalle n‰kyviksi merkeiksi*/

void sivunaytto(int x, int y, int ammo){ /*tulostaa kaiken tarvittavan n‰ytˆn sivupuolelle*/
		lcd_gotoxy(10,0);
		lcd_write_data(':')	;	
		lcd_write_data(dec_to_char[x]);
		lcd_write_data(',');
		if(y==9){
			lcd_write_data('1');
			lcd_write_data('0');
		}
		else{
			lcd_write_data(ascii_to_dec[y+1]);
		}
		lcd_gotoxy(10,1);


		lcd_write_data(':');
		lcd_write_data(ascii_to_dec[ammo/10]);
		lcd_write_data(ascii_to_dec[ammo%10]);
		
		if(aika>99){

			
			lcd_write_data(ascii_to_dec[aika/100]);
			lcd_write_data(ascii_to_dec[(aika%100)/10]);
		
		}			
		else{
			

			lcd_write_data('t');
			lcd_write_data(ascii_to_dec[aika/10]);
			}
		lcd_write_data(ascii_to_dec[aika%10]);
}

	uponneet=0;


	int peli[12][12]; /*m‰‰ritet‰‰n pelilauta jolle laivat sijoitetaan (t‰ynn‰ nollia)*/
	for(y=0;y<12;y++){
		for(x=0; x<12; x++){
			peli[y][x]=0;
		}
	}
	x=0;
	y=0;

	int ui[12][12]; /*m‰‰ritet‰‰n pelaajalle n‰kyv‰ pelilauta (user interface)*/
	for(y=0;y<12;y++){
		for(x=0; x<12; x++){
			ui[y][x]=0;
		}
	}
	for(t=0;t<12;t++){ /*tehd‰‰n kehykset pelilaudalle*/
		peli[t][0]=8;
		peli[t][11]=8;
		peli[0][t]=8;
		peli[11][t]=8;
		ui[t][0]=8;
		ui[t][11]=8;
		ui[0][t]=8;
		ui[11][t]=8;
		
	}
	 



	


	enter=0;


	

	lcd_write_ctrl(LCD_CLEAR);


	x=0;
	y=0;

 
 

 
					
					

					

					
					
					

   

   
	   
	   /* SUUNNAT:
	   0 = PYSTY (laiva sijoitetaan random xy-koordinaatista etel??n suuntaan)
	   1 = VAAKA (laiva sijoitetaan random xy-koordinaatista id??n suuntaan) */

	   

void place_ship(int pituus, int tunnus){/*laivansijoitusfunktio, laivan numero kent‰ll‰ tunnukseen*/
	int sijoitettu=0;	
	int tilaa=0;

 while(sijoitettu==0){	
	y_koord=(1+(rand()%10));
	x_koord=(1+(rand()%10));	
	tilaa=0;
	suunta = rand() % 2;
	switch(suunta){
	case 0:/*pystyyn*/
		
			
		
		for(t=0;t<(pituus);t++){ 			/*katsotaan onko tilaa, laivan pituuden mukaan*/
			if(peli[y_koord+t][x_koord]==0){
				tilaa++;
 
			}
		}
		if(tilaa==pituus){	/*jos laivalle on tilaa niin sijoitetaan laiva tarkistettuun tilaan*/
			for(t=0;t<(pituus);t++){
				peli[y_koord+t][x_koord]=tunnus;/*merkit‰‰n laivan numero kent‰lle ja sitten suojaverkko*/
				for(y=-1;y<2;y++){
					if(peli[y_koord+t+y][x_koord]==0){
						peli[y_koord+t+y][x_koord]=9;
					}									
				}
				for(x=-1;x<2;x++){
					if(peli[y_koord+t][x_koord+x]==0){
						peli[y_koord+t][x_koord+x]=9;
					}
				}														
			}
			sijoitettu=1;
		}

	break;
	case 1:	/*sama homma vaakasuoraan*/
		

		
		for(t=0;t<(pituus);t++){
			if(peli[y_koord][x_koord+t]==0){
				tilaa++;

			}
		}
		if(tilaa==pituus){
			for(t=0;t<(pituus);t++){
				peli[y_koord][x_koord+t]=tunnus;
				for(y=-1;y<2;y++){
					if(peli[y_koord+y][x_koord+t]==0){
						peli[y_koord+y][x_koord+t]=9;
					}									
				}
				for(x=-1;x<2;x++){
					if(peli[y_koord][x_koord+t+x]==0){
						peli[y_koord][x_koord+t+x]=9;
					}
				}			
				
			}
			sijoitettu=1;
		}
	break;
	}
 	
 }

 


			
		


}
place_ship(5,6);
place_ship(4,5);
place_ship(3,4);
place_ship(3,3);
place_ship(2,2);
place_ship(1,1);


	   
	   
	   



	       /* muutetaan numero 9:t kartalla 0:ksi*/
	   for(y = 0; y < 12; y++) {
	       for (x = 0; x < 12; x++){
	           if (peli[y][x] == 9){
	               peli[y][x] = 0;}

	           }
	   }

   
	/*pelaamisvaihe*/
	x=0;
	y=0;
	aika=0;
	lcd_write_ctrl(LCD_CLEAR);
	_delay_ms(200);
	trisound(50,35,20);
	trisound(20,35,50);
	while(uponneet!=6 && ammo>0){   /*<- pelin p????silmukka*/

	

	enter=0;				/* x,y koordinaattien valinta nappuloilla */
	while(enter==0)
		{
		enter=0;
		
		lcd_write_ctrl(LCD_CLEAR); /*p‰ivitet‰‰n n‰yttˆ uusien koordinaaattien mukaan, laitetaan n‰ytˆlle muutettuna pelimerkki[] mukaan*/
		
		for(t=1;t<11;t++){
			lcd_write_data(pelimerkki[ui[1+y][t]]);	
			}
		lcd_gotoxy(0,1);
		if(y<9)
		
			{
			for(t=1;t<11;t++){			
				lcd_write_data(pelimerkki[ui[y+2][t]]);
				}
			}
		sivunaytto(x,y,ammo);
		lcd_gotoxy(x,0);	/*siirret‰‰n kursori nykyiseen paikkaan*/
		lcd_write_ctrl(LCD_ON | LCD_BLINK); /* kursori p‰‰lle*/
	
		nappi=5;
		if((1==move_xy(read_btn(),9,9))&& (!(PINA & (1<< PA2))))enter=1;
		else enter=0;

		lcd_write_ctrl(LCD_OFF | LCD_BLINK);
		_delay_ms(200);
		/* kun move_xy palauttaa ykkˆsen = ammuttu, kun nollan= takaisin looppiin liikkumaan lis‰‰ */
		
		if(ui[y+1][x+1]!=0){ /*niin ei ammuta kahdesti samaan paikkaan*/
			enter=0;
			}

		
	}
	TCNT3=0;
	OCR3AL = 33;
	ETIMSK|=(1<<OCIE3A);
	
	_delay_ms(100);
	TCNT3=0;
    

	/* koordinaatit valittu, ammuntaosa ja osuman tarkistus alkaa*/

	if (peli[y+1][x+1]==0){ /*katsotaan pelilaudalta osuiko*/
		ui[y+1][x+1]=1;     /* 1 pelilaudalla on huti, 2 osuma, 3 upposi*/
		ammo--;
		
		OCR3AL = 49;

		}
	else{ /* tarkistetaan mihin laivaan osui ja upposiko, v‰hennet‰‰n laivan hpta*/
			
		
		OCR3AL = 16;

		switch(peli[y+1][x+1]){
		case 1:
			ui[y+1][x+1]=3;
			uponneet++;
		
			submarine=0;
			break;
		case 2:
	
			destroyer--;
				if(destroyer==0){
					ui[y+1][x+1]=3;
					uponneet++;
				}
				else{
					ui[y+1][x+1]=2;
				}
			break;
		case 3:

			cruiser_2--;
				if(cruiser_2==0){
					ui[y+1][x+1]=3;
					uponneet++;
				}
				else{
					ui[y+1][x+1]=2;
				}
			break;
		case 4:

			cruiser_1--;
				if(cruiser_1==0){
					ui[y+1][x+1]=3;
					uponneet++;
				}
				else{
					ui[y+1][x+1]=2;
				}
				break;
		case 5:

			battleship--;
				if(battleship==0){
					ui[y+1][x+1]=3;
					uponneet++;
				}
				else{
					ui[y+1][x+1]=2;
				}
				break;
		case 6:
		
			carrier--;
				if(carrier==0){
					ui[y+1][x+1]=3;
					uponneet++;
				
				}
				else{
					ui[y+1][x+1]=2;
				}
				break;
		}
	}
	_delay_ms(100);
	
	ETIMSK &= ~(1<<OCIE3A);
	
	
	

		

	}/*pelin p‰‰silmukka loppuu*/								      				
	

		lcd_write_ctrl(LCD_ON);	
		lcd_write_ctrl(LCD_CLEAR);
}

ISR(TIMER1_COMPA_vect) {/*ajalaskentaan sekunneissa*/
	aika++;
	
}
ISR((TIMER3_COMPA_vect)){/*‰‰nt‰*/
	PORTE ^= (1 << PE4) | (1 << PE5);
	
				
}



