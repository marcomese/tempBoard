//Numero di righe e colonne per il display in uso

#define DISP_COL (16)
#define DISP_ROW (2)

#define MAX_COL  (DISP_COL-1)
#define MAX_ROW  (DISP_ROW-1)

#define CLEAR_DISPLAY   (1)
#define RETURN_HOME  (1<<1)
#define ID_1    (1<<1)
#define ID_0    (0)
#define S_1     (1)
#define S_0     (0)
#define DL_1    (1)
#define DL_0    (0)
#define D_1    (1)
#define D_0    (0)
#define C_1    (1)
#define C_0    (0)
#define B_1    (1)
#define B_0    (0)
#define SC_0   (0)
#define SC_1   (1)
#define RL_0   (0)
#define RL_1   (1)

//Possibili valori da utilizzare per le variabili di onoff e blink LCD_cursorMode

#define CURSOR_ON  (1)
#define CURSOR_OFF (0)
#define BLINK_ON   (1)
#define BLINK_OFF  (0)

//Possibili valori da utilizzare per la variabile direction di LCD_curShift ed LCD_displayShift

#define RIGHT (1)
#define LEFT  (0)

//Possibili valori da utilizzare per le variabili rowNumb, fontSize, curDir e textShift di LCD_Init

#define TEXT_RIGHT_TO_LEFT (0)
#define TEXT_LEFT_TO_RIGHT (1)
#define TEXT_SHIFT_NO (0)
#define TEXT_SHIFT_YES (1)
#define SINGLE_ROW (0)
#define DUAL_ROW (1)
#define NORMAL_FONT (0)
#define BIG_FONT (1)

//Possibili valori da utilizzare per la variabile cmd di writeNibble

#define CMD     (0)
#define CHR     (1)

typedef struct {
    GPIO_TypeDef* GPIOx;
    uint8_t D7;
    uint8_t D6;
    uint8_t D5;
    uint8_t D4;
    uint8_t RS;
    uint8_t E;
} LCD_Pin;

extern LCD_Pin lcd;

/*
    __PIN_MASK serve nella funzione writeNibble per mascherare i bit del registro
    ODR che non si vogliono modificare
*/

#define __PIN_MASK(__D7__,__D6__,__D5__,__D4__,__RS__)\
            ((1<<__D7__) | \
             (1<<__D6__) | \
             (1<<__D5__) | \
             (1<<__D4__) | \
             (1<<__RS__))
/*
    __NIB_TO_PIN DIVIDE I VARI BIT DEL NIBBLE DA SCRIVERE NELLE VARIE PORTE DI USCITA
*/

#define __NIB_TO_PIN(__NIB__,__D7__,__D6__,__D5__,__D4__)\
            ((((__NIB__&(1<<3))>>3)<<__D7__) | \
            (((__NIB__&(1<<2))>>2)<<__D6__) | \
            (((__NIB__&(1<<1))>>1)<<__D5__) | \
            (((__NIB__&(1<<0))>>0)<<__D4__))

/*
    __SET_E(__E__) e __RESET_E(__E__) impostano ad 1 il bit nella posizione __E__.
    Queste due macro vanno usate per scrivere nel registro BSRR che utilizza i
    primi 16 bit per settare la porta corrispondente e i 16 bit più significativi
    per resettarla (per questo il valore di __E__ viene incrementato di 16 nella
    macro __RESET_E.
*/
                
#define __SET_E(__E__) (1<<__E__)
                
#define __RESET_E(__E__) (1<<(__E__+16))

/*
    Tutte le macro seguenti corrispondono ai comandi riconosciuti dal modulo LCD.
    Impostano solo i bit necessari (i meno significativi) ma per essere riconosciuti
    è necessario inviare prima writeNibble(0x0,CMD). Le uniche eccezione sono costituite
    dalla coppia __DATA_INTERFACE e __DIMENSIONS, che vanno eseguite in coppia perchè
    appartengono alla stessa istruzione (corrispondente all'istruzione indicata come
    Function Set nel datasheet del modulo LCD) e da __SET_DDRAM_ADDR_H e __SET_DDRAM_ADDR_L
    che si riferiscono alla parte alta e bassa della stessa istruzione.
    La macro __GET_ADDR permette di convertire le coordinate (row,col) in un indirizzo valido
    della DDRAM che potrà essere usato nelle macro __SET_DDRAM_ADDR_H e __SET_DDRAM_ADDR_L.
*/
                
#define __DATA_INTERFACE(__DL__) ((1<<1) | __DL__)              
#define __DIMENSIONS(__N__,__F__) ((__N__<<3) | (__F__<<2))
                
#define __ENTRY_MODE_SET(__ID__,__S__) ((1<<2) | (__ID__<<1) | __S__)
                
#define __DISPLAY_ON_OFF(__D__,__C__,__B__) ((1<<3) | (__D__<<2) | (__C__<<1) | __B__)

#define __CURSOR_DISP_SHIFT(__SC__,__RL__) ((__SC__<<3) | (__RL__<<2))

#define __SET_DDRAM_ADDR_H(__ADDR__) ((1<<3) | ((__ADDR__&(1<<6))>>4) | ((__ADDR__&(1<<5))>>4) | ((__ADDR__&(1<<4))>>4))
#define __SET_DDRAM_ADDR_L(__ADDR__) ((__ADDR__&(1<<3)) | (__ADDR__&(1<<2)) | (__ADDR__&(1<<1)) | (__ADDR__&1))

#define __SET_CGRAM_ADDR_H(__ADDR__) ((1<<2) | (((__ADDR__*8)&(1<<5))>>4) | (((__ADDR__*8)&(1<<4))>>4))
#define __SET_CGRAM_ADDR_L(__ADDR__) (((__ADDR__*8)&(1<<3)) | ((__ADDR__*8)&(1<<2)) | ((__ADDR__*8)&(1<<1)) | ((__ADDR__*8)&1))
                
#define __GET_ADDR(__ROW__,__COL__) ( (__ROW__)*0x40 + __COL__ )

void LCD_Init(uint8_t rowNumb, uint8_t fontSize, uint8_t curDir, uint8_t textShift);
void LCD_displayON(void);
void LCD_displayOFF(void);
void LCD_displayClear(void);

int  LCD_curSetHome(void);
int  LCD_curSet(uint8_t row, uint8_t col);
void LCD_curMode(uint8_t onoff, uint8_t blink);
void LCD_curShift(uint8_t direction);
void LCD_curShiftOf(uint8_t posNum, uint8_t direction, uint32_t delay);

void LCD_displayShift(uint8_t direction);
void LCD_displayShiftOf(uint8_t posNum, uint8_t direction, uint32_t delay);

int  LCD_putch(char c);
int  LCD_putfloat(float num, uint32_t precision);
int  LCD_putint(int num);
int  LCD_print(char* string);
int  LCD_printf(char *fmt,...);

void LCD_createChar(uint8_t addr, uint8_t b0, uint8_t b1, uint8_t b2, uint8_t b3, uint8_t b4,\
                    uint8_t b5, uint8_t b6, uint8_t b7);

//Etichette grafiche da usare per le variabili b0-b7 di LCD_createChar

#define _____  (0)
#define ____O  (1)
#define ___O_  (2)
#define ___OO  (3)
#define __O__  (4)
#define __O_O  (5)
#define __OO_  (6)
#define __OOO  (7)
#define _O___  (8)
#define _O__O  (9)
#define _O_O_  (10)
#define _O_OO  (11)
#define _OO__  (12)
#define _OO_O  (13)
#define _OOO_  (14)
#define _OOOO  (15)
#define O____  (16)
#define O___O  (17)
#define O__O_  (18)
#define O__OO  (19)
#define O_O__  (20)
#define O_O_O  (21)
#define O_OO_  (22)
#define O_OOO  (23)
#define OO___  (24)
#define OO__O  (25)
#define OO_O_  (26)
#define OO_OO  (27)
#define OOO__  (28)
#define OOO_O  (29)
#define OOOO_  (30)
#define OOOOO  (31)