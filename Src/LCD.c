#include <stm32f4xx.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include "LCD.h"

#define LF (10)
#define TAB (9)
#define MAX_PRECISION_FLOAT (7)
#define INT_MAX_DIGIT (10)
#define FLOAT_MAX_DIGIT (40)

/*
  La funzione writeNibble invia 4bit al modulo LCD "distribuendo" ogni bit del nibble al pin a cui � destinato 
  sul modulo LCD, senza modificare gli altri bit del registro ODR.
*/

LCD_Pin lcd;

void writeNibble(uint8_t nib, uint8_t cmd){
    uint32_t odrreg = lcd.GPIOx->ODR;                           //salvo il valore attuale di ODR
    odrreg &= ~__PIN_MASK(lcd.D7,lcd.D6,lcd.D5,lcd.D4,lcd.RS);  //imposto a 0 solo i bit che voglio modificare
    odrreg |= (__NIB_TO_PIN(nib,lcd.D7,lcd.D6,lcd.D5,lcd.D4) | \
              (cmd<<lcd.RS));                                   //modifico i bit che mi interessano
    lcd.GPIOx->ODR = odrreg;                                    //scrivo su ODR
    HAL_Delay(1);
    lcd.GPIOx->BSRR |= __SET_E(lcd.E);
    lcd.GPIOx->BSRR |= __RESET_E(lcd.E);
}

void LCD_putcmd(uint8_t cmd_h,uint8_t cmd_l){
    writeNibble(cmd_h,CMD);
    writeNibble(cmd_l,CMD);
}

/*
  LCD_Init inizializza il display indicando quante righe usare (rowNumb), le dimensioni del font (fontSize),
  la direzione del cursore (curDir) e se � il testo a dover shiftare o il cursore (textShift).
  I valori di queste variabili sono contenuti del file LCD.h
*/

void LCD_Init(uint8_t rowNumb, uint8_t fontSize, uint8_t curDir, uint8_t textShift){
    
    HAL_Delay(16);
    
    writeNibble(0x3, CMD);
    
    HAL_Delay(5);
    
    writeNibble(0x3, CMD);
    
    HAL_Delay(1);
    
    LCD_putcmd(0x3,0x2);
    
    LCD_putcmd(__DATA_INTERFACE(DL_0),__DIMENSIONS(rowNumb,fontSize));
    
    LCD_putcmd(0x0,__DISPLAY_ON_OFF(D_0,C_0,B_0));

    LCD_putcmd(0x0,CLEAR_DISPLAY);

    LCD_putcmd(0x0,__ENTRY_MODE_SET(curDir,textShift));

    LCD_putcmd(0x0,__DISPLAY_ON_OFF(D_1,C_1,B_1));

}

int LCD_curSetHome(void){
    LCD_putcmd(0x0,RETURN_HOME);
    return 0;
}

int LCD_curSet(uint8_t row, uint8_t col){
    LCD_putcmd(__SET_DDRAM_ADDR_H(__GET_ADDR(row,col)),__SET_DDRAM_ADDR_L(__GET_ADDR(row,col)));
    return col;
}

int LCD_putch(char c){
    writeNibble((c&0xF0)>>4,CHR);
    writeNibble(c,CHR);
    return 1;
}

int LCD_print(char *string){
    int i=0;
    while(string[i]){
      LCD_putch(string[i]);
      i++;
    }
    
    return i;
}

int LCD_putfloat(float num, uint32_t precision){
    char numstr[FLOAT_MAX_DIGIT];
    char prc[5];
    int i=0;

    if(precision>MAX_PRECISION_FLOAT) precision=MAX_PRECISION_FLOAT;

    sprintf(prc,"%%.%luf",precision);
    sprintf(numstr,prc,num);
    
    while(numstr[i]){
      LCD_putch(numstr[i]);
      i++;
    }
 
    return i;
}

int LCD_putint(int num){
    char numstr[INT_MAX_DIGIT];
    int i=0;
    
    sprintf(numstr,"%d",num);
    
    while(numstr[i]){
      LCD_putch(numstr[i]);
      i++;
    }
    
    return i;
}

void LCD_curMode(uint8_t onoff, uint8_t blink){
    LCD_putcmd(0x0,__DISPLAY_ON_OFF(D_1,onoff,blink));
}

void LCD_displayON(void){
    LCD_putcmd(0x0,__DISPLAY_ON_OFF(D_1,C_1,B_1));
}

void LCD_displayOFF(void){
    LCD_putcmd(0x0,__DISPLAY_ON_OFF(D_0,C_0,B_0));
}

void LCD_curShift(uint8_t direction){
    LCD_putcmd(0x1,__CURSOR_DISP_SHIFT(SC_0,direction));
}

void LCD_curShiftOf(uint8_t posNum, uint8_t direction, uint32_t delay){
    while(posNum){
        LCD_curShift(direction);
        posNum--;
        HAL_Delay(delay);
    }
}
void LCD_displayShift(uint8_t direction){
    LCD_putcmd(0x1,__CURSOR_DISP_SHIFT(SC_1,direction));
}

void LCD_displayShiftOf(uint8_t posNum, uint8_t direction, uint32_t delay){
    while(posNum){
        LCD_displayShift(direction);
        posNum--;
        HAL_Delay(delay);
    }
}

void LCD_displayClear(void){
    LCD_putcmd(0x0,CLEAR_DISPLAY);
}

void LCD_createChar(uint8_t addr, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7){
    LCD_putcmd(__SET_CGRAM_ADDR_H(addr),__SET_CGRAM_ADDR_L(addr));
    LCD_putch(b0);
    LCD_putch(b1);
    LCD_putch(b2);
    LCD_putch(b3);
    LCD_putch(b4);
    LCD_putch(b5);
    LCD_putch(b6);
    LCD_putch(b7);
    LCD_displayClear();
}

int LCD_printf(char* fmt, ...){
    
    va_list args;
    va_start(args,fmt);

    float flt;
    int integer;
    char ch;
    char *str;
    uint32_t prec;
    uint8_t colCounter=0;
    uint8_t rowCounter=0;

    LCD_curSet(0,0);
    
    while(*fmt){
        
        switch(*fmt){
            
          case ('%'):
            fmt++;
            switch(*fmt){
              case '%':
                colCounter+=LCD_putch('%');
                break;
              case 'c':
                ch=va_arg(args,int);
                colCounter+=LCD_putch(ch);
                break;
              case 's':
                str=va_arg(args,char*);
                colCounter+=LCD_print(str);
                break;
              case 'd':
                integer = va_arg(args,int);
                colCounter+=LCD_putint(integer);
                break;
              case 'f':
                flt = va_arg(args,double);
                colCounter+=LCD_putfloat(flt,4);
                break;
              case '.':
                fmt++;
                prec=(uint32_t)(*fmt-48);
                fmt++;
                flt = va_arg(args,double);
                colCounter+=LCD_putfloat(flt,prec);
                break;
            }
            break;
            
          case (LF):
            rowCounter++;
            if(rowCounter>MAX_ROW) rowCounter=MAX_ROW;
            else colCounter=LCD_curSet(rowCounter,0);
            break;
            
          case (TAB):
            colCounter=LCD_curSet(rowCounter,colCounter+4);
            break;
              
          default:
            colCounter+=LCD_putch(*fmt);
            
        }
        
        fmt++;
        
        if(colCounter>MAX_COL && rowCounter!=MAX_ROW){
            rowCounter++;
            colCounter=LCD_curSet(rowCounter,0); 
        }
    }
    
    va_end(args);
    
    return colCounter;
}

/*
      Di seguito si riportano le stesse funzioni ma che richiamano tutte la stessa funzione writeNibble.
      Si pu� utilizzare queste funzioni al posto di quelle precedenti nel caso ci fossero problemi con
      il numero di chiamate a funzioni. (Il codice viene per� pi� lungo)
*/
//
//void LCD_Init(uint8_t rowNumb, uint8_t fontSize, uint8_t curDir, uint8_t textShift){
//    
//    HAL_Delay(16);
//    
//    writeNibble(0x3, CMD);
//    
//    HAL_Delay(5);
//    
//    writeNibble(0x3, CMD);
//    
//    HAL_Delay(1);
//    
//    writeNibble(0x3, CMD);    
//    writeNibble(0x2, CMD);
//    
//    writeNibble(__DATA_INTERFACE(DL_0), CMD);
//    writeNibble(__DIMENSIONS(rowNumb,fontSize), CMD);
//    
//    writeNibble(0x0, CMD);
//    writeNibble(__DISPLAY_ON_OFF(D_0,C_0,B_0),CMD);
//    
//    writeNibble(0x0, CMD);
//    writeNibble(CLEAR_DISPLAY,CMD);
//    
//    writeNibble(0x0, CMD);
//    writeNibble(__ENTRY_MODE_SET(curDir,textShift), CMD);
//    
//    writeNibble(0x0, CMD);
//    writeNibble(__DISPLAY_ON_OFF(D_1,C_1,B_1),CMD);
//}
//
//int LCD_curSetHome(void){
//    writeNibble(0x0, CMD);
//    writeNibble(RETURN_HOME, CMD);
//    return 0;
//}
//
//int LCD_curSet(uint8_t row, uint8_t col){
//    writeNibble(__SET_DDRAM_ADDR_H(__GET_ADDR(row,col)),CMD); //converte prima la coppia (row,col) in un indirizzo
//    writeNibble(__SET_DDRAM_ADDR_L(__GET_ADDR(row,col)),CMD); //DDRAM e poi invia prima il nibble alto e poi quello basso
//    return col;
//}
//int LCD_print(char *string){
//    int i=0;
//    while(string[i]){
//      writeNibble(((string[i]&0xF0)>>4),CHR);   //invia prima il nibble alto del carattere
//      writeNibble(string[i],CHR);        // e poi quello basso
//      i++;
//    }
//    
//    return i;
//}
//int LCD_putfloat(float num, uint32_t precision){
//    char numstr[FLOAT_MAX_DIGIT];
//    char prc[5];
//    int i=0;
//
//    if(precision>MAX_PRECISION_FLOAT) precision=MAX_PRECISION_FLOAT;
//
//    sprintf(prc,"%%.%df",precision);
//    sprintf(numstr,prc,num);
//    
//    while(numstr[i]){
//      writeNibble(((numstr[i]&0xF0)>>4),CHR);   //invia prima il nibble alto del carattere
//      writeNibble(numstr[i],CHR);               // e poi quello basso
//      i++;
//    }
// 
//    return i;
//}
//int LCD_putint(int num){
//    char numstr[INT_MAX_DIGIT];
//    int i=0;
//    
//    sprintf(numstr,"%d",num);
//    
//    while(numstr[i]){
//      writeNibble(((numstr[i]&0xF0)>>4),CHR);   //invia prima il nibble alto del carattere
//      writeNibble(numstr[i],CHR);        // e poi quello basso
//      i++;
//    }
//    
//    return i;
//}
//void LCD_curShift(uint8_t direction){
//    writeNibble(0x1, CMD);
//    writeNibble(__CURSOR_DISP_SHIFT(SC_0,direction),CMD);    
//}
//
//void LCD_curShiftOf(uint8_t posNum, uint8_t direction, uint32_t delay){
//    while(posNum){
//        writeNibble(0x1, CMD);
//        writeNibble(__CURSOR_DISP_SHIFT(SC_0,direction),CMD);
//        posNum--;
//        HAL_Delay(delay);
//    }
//}
//
//void LCD_displayShift(uint8_t direction){
//    writeNibble(0x1, CMD);
//    writeNibble(__CURSOR_DISP_SHIFT(SC_1,direction),CMD);
//}
//
//void LCD_displayShiftOf(uint8_t posNum, uint8_t direction, uint32_t delay){
//    while(posNum){
//        writeNibble(0x1, CMD);
//        writeNibble(__CURSOR_DISP_SHIFT(SC_1,direction),CMD);
//        posNum--;
//        HAL_Delay(delay);
//    }
//}
//void LCD_createChar(uint8_t addr, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4, uint8_t b5, uint8_t b6, uint8_t b7){
//    writeNibble(__SET_CGRAM_ADDR_H(addr),CMD);
//    writeNibble(__SET_CGRAM_ADDR_L(addr),CMD);
//    writeNibble((b0&0xF0)>>4,CHR);
//    writeNibble(b0,CHR);
//    writeNibble((b1&0xF0)>>4,CHR);
//    writeNibble(b1,CHR);
//    writeNibble((b2&0xF0)>>4,CHR);
//    writeNibble(b2,CHR);
//    writeNibble((b3&0xF0)>>4,CHR);
//    writeNibble(b3,CHR);
//    writeNibble((b4&0xF0)>>4,CHR);
//    writeNibble(b4,CHR);
//    writeNibble((b5&0xF0)>>4,CHR);
//    writeNibble(b5,CHR);
//    writeNibble((b6&0xF0)>>4,CHR);
//    writeNibble(b6,CHR);
//    writeNibble((b7&0xF0)>>4,CHR);
//    writeNibble(b7,CHR);
//    writeNibble(0x0, CMD);
//    writeNibble(CLEAR_DISPLAY,CMD);
//}
