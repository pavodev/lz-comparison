/***********************************************************************************************************************
 *
 *  LZ77_Linear.c
 *
 *  Creato il: 30 ott 2017
 *
 *  Autore: Ivan Pavic
 *
 ***********************************************************************************************************************
 *                                             ALGORITMO  DI COMPRESSIONE LZ77                                         *
 ***********************************************************************************************************************
 *
 *  L'algoritmo LZ77 è un algoritmo di compressione a dizionario implicito che fa parte della famiglia dei
 *  compressori senza perdita di informazioni (lossless).
 *  L'algoritmo si basa sullo scorrimento di una finestra che contiene i dati che sono già stati codificati e un buffer
 *  che contiene i dati da codificare.
 *  L'obbiettivo è quello di andare a cercare le sequenze più lunghe possibili che sono contenute all'interno del buffer
 *  muovendosi all'interno delle finestra. Il codice che si ottiene è il seguente:
 *
 *                      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 *                      +                         prossimo carattere           +
 *                      +                                 |                    +
 *                      +|                        |       v |                 |+
 *                      +|b c b c c c a b c a b b | a b c c | e a b b c a b c |+
 *                      +|            *           | * * *   |                 |+
 *                      +             ^                                        +
 *                      +             |                                        +
 *                      +           offset         lunghezza                   +
 *                      +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 *
 *  In questo caso si può vedere che l'offset vale 6, la lunghezza 3 e il carattere successivo alla sequenza è c.
 *
 ***********************************************************************************************************************
 *
 *  pseudo-codice dell'algoritmo LZ77:
 *
 *  1:  inizio
 *  2:     riempimento buffer da file
 *  3:     while (buffer non è vuoto) do
 *  4:        cerca la sequenza più lunga possibile del buffer nella finestra
 *  5:        o := offset
 *  6:        l := lunghezza sequenza
 *  7:        a := carattere successivo
 *  8:        scrivo nel file compresso (o, l, a)
 *  8:        aggiungi l+1 caratteri nella finestra
 *  10:      end
 *  11:  fine
 *
 ***********************************************************************************************************************
 *                                              SCRITTURA BUFFERIZZATA                                                 *
 ***********************************************************************************************************************
 *
 * La codifica generata dall'algoritmo è composta da 2 numeri interi e un char.
 * Scrivendo su file una codifica del genere, il fattore di compressione sarebbe molto basso poichè un numero intero è
 * rappresentato con 4 byte, la codifica dunque occuperebbe 4+4+1 = 9byte.
 *
 * Per ovviare a questa problematica ho implementato la scrittura bufferizzata che consiste nel trasformare i singoli
 * elementi della codifica in binario e inserirli concatenati all'interno di un array di 8 celle.
 * Le celle sono sequenze di 'zeri' e 'uno' che rappresentano 1 byte.
 * Con un contatore globale viene tenuto conto l'indice in cui inserire i bit, quando il buffer viene riempito, il
 * numero binario che rappresenta, viene convertito in decimale e scritto nel file compresso sottoforma di char.
 *
 * Grazie a questa conversione limito la dimensione con la quale vengono salvate le codifiche.
 *
 * Per limitare ulteriormente la dimensione delle codifiche, nel caso in cui la lunghezza della sequenza è 0, l'offset
 * non viene bufferizzato poichè non sarà più rilevante nella fase di decompressione.
 *
 * Maggiori informazioni nella documentazione.
 *
 * ********************************************************************************************************************/

/*******************************************************INCLUDE********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>


/*******************************************************DEFINE*********************************************************/

#define LOOKAHEAD 8
#define WINDOW 8192
#define STREAM_SIZE 4000000
#define BUFFER_SIZE 8
#define STRUCT_ARRAY_SIZE 600000

/*****************************************************STRUTTURE********************************************************/

struct code
{
    int o;		        //offset
    int l;		        //length
    unsigned char a;	//next char
};

/**************************************************VARIABILI GLOBALI***************************************************/
int bits=0;
int decimal=0;
int counter=0;
int n_bits=0;                               //vedi la funzione extractCodes
int buffer_position=BUFFER_SIZE;            //vedi la funzione extractCodes
unsigned char decompressed[STREAM_SIZE];    //bytes decompressi
clock_t begin;
/**********************************************************************************************************************/










/***********************************************************************************************************************
*                                                     COMPRESSIONE                                                     *
***********************************************************************************************************************/
/***********************************************************************************************************************
*                                                       FUNZIONI                                                       *
***********************************************************************************************************************/
void inizializeCharArray(unsigned char array[], int size)
{
    for(int i=0; i<size; i++)
        array[i]='\0';
}

void inizializeArray(int array [], int size)
{
    for(int i=0; i<size; i++)
        array[i]=0;
}

/***********************************************************************************************************************
 * int binToDec(int)
 *
 * Questa funzione converte in decimale i byte generati dalla scrittura bufferizzata. Il valore decimale ritornato
 * viene scritto nel file compresso sottoforma di char.
 *
 * @param buffer
 * @return decimal
 *
 */
int binToDec(int buffer[])
{
    int decimal=0;
    for(int i=BUFFER_SIZE-1; i>=0; i--)
        decimal = decimal + ( buffer[BUFFER_SIZE-1-i] * ((int) pow(2,i) ));
    return decimal;
}

/***********************************************************************************************************************
 * void fillBuffer(int[], int, FILE *)
 *
 * Funzione che gestisce il riempimento del buffer, il quale rappresenta 1 byte, per la scrittura bufferizzata.
 * L'argomento bit viene scritto alla posizione giusta grazie alla variabile globale bits che viene incrementata dopo
 * ogni inserimento.
 * Se il buffer viene riempito completamente, viene ricavato il suo valore decimale e questo valore viene scritto sul
 * file compresso.
 * Segue l'inizializzazione del buffer e l'azzeramento del contatore di posizione: "bits".
 *
 * @param buffer
 * @param bit
 * @param outfile
 */
void fillBuffer(int buffer[], int bit, FILE *outfile){
    if(bits<BUFFER_SIZE){
        buffer[bits] = bit;
        bits++;
    }else if(bits==BUFFER_SIZE)
    {
        decimal = binToDec(buffer);
        fputc(decimal, outfile);
        inizializeArray(buffer, BUFFER_SIZE);
        bits=0;
        buffer[bits]=bit;
        bits++;
    }
}

/***********************************************************************************************************************
 * void decToBin(int, int, int, FILE *)
 *
 * Ogni singolo elemento della codifica (struct code) viene passato a questa funzione per essere convertito in binario.
 * La funzione ricava i bit degli elementi di codifica e li passa uno ad uno alla funzione fillBuffer.
 *
 * @param buffer
 * @param n         --> elemento della codifica da convertire in binario
 * @param b_size    --> numero di bit necessari per rappresentare l'elemento di codifica
 * @param outfile
 */
void decToBin(int buffer[], int n, int b_size, FILE  *outfile)
{
    for(int i=b_size-1; i>=0; i--)
    {
        if(n >=((int)pow(2,i)) )
        {
            fillBuffer(buffer, 1, outfile);
            n = n - ((int) pow(2, i));
        }else
            fillBuffer(buffer, 0, outfile);
    }
}

/***********************************************************************************************************************
 * int bufferizedWriting(struct code *, int *, FILE *)
 *
 * Se la lunghezza della sequenza è nulla la codifica bufferizzata è la seguente: (length, nextchar)
 * Altrimenti: (length, offset, nextchar)
 *
 * @param code      --> struttura contenente la codifica da bufferizzare
 * @param buffer    --> byte buffer
 * @param outfile
 * @return          --> check value
 */
int bufferizedWriting(struct code *code, int *buffer, FILE *outfile)
{
    if(code->l==0)
    {
        //printf("\n%d) (%d, 0, %c)", counter, code->l, code->a);
        decToBin(buffer, code->l, ((int) log2(LOOKAHEAD)), outfile);
        decToBin(buffer, code->a, sizeof(unsigned char)*8, outfile);
        //counter++;
    }else
    {
        //printf("\n%d) (%d, %d, %c)", counter, code->l, code->o, code->a);
        decToBin(buffer, code->l, ((int) log2(LOOKAHEAD)), outfile);
        decToBin(buffer, code->o-1, ( (int) log2(WINDOW)), outfile);
        decToBin(buffer, code->a, sizeof(unsigned char)*8, outfile);
        //counter++;
    }

    return 1;
}

/***********************************************************************************************************************
 * int LZ77_compressor(FILE*, FILE*)
 *
 * La ricerca delle sequenze avviene all'interno di questa funzione.
 * Il file "infile" passato come argomento viene letto con la funzione di libreria fread che riempie un buffer con
 * i byte da comprimere.
 * La ricerca della sequenza più lunga è svolta muovendo una serie di puntatori all'interno del buffer.
 * Un puntatore tiene conto dell'inizio dell'ultima sequenza più lunga trovata.
 *
 * Dopo aver controllato tutta la finestra, il codice legato all'ultima sequenza più lunga trovata viene mandato alle
 * funzioni per la scrittura bufferizzata.
 * A questo punto viene effettuato uno "sliding" dei puntatori di lunghezza+1 per caricare il look-ahead buffer con i
 * nuovi byte da comprimere e espellere lo stesso numero di byte dal searchbuffer,
 *
 * Queste operazioni vengono effettuate finchè fread è in grado di riempire il buffer.
 *
 *
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *                          + window                  lookahead                           +
 *                          +    |                        |                               +
 *                          +    v                        v                               +
 *                          +                                                             +
 *                          +    |                        |         |                 |   +
 *                          +    |b c b c c c a b c a b b | a b c c | e a b b c a b c |   +
 *                          +    |            *           | * * *   |                 |   +
 *                          +                                                             +
 *                          +                 ^                 ^                         +
 *                          +                 |                 |                         +
 *                          +              w_cursor           l_cursor                    +
 *                          +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *
 * Puntatori utilizzati:
 *
 * lookahead        inizio lookahead buffer, da qui partono tutte le ricerche.
 * window           inizio della finestra.
 * l_cursor         puntatore che si muove nel lookahead.
 * w_cursor         puntatore che si muove nella finestra per segnare l'inizio della sequenza.
 * w_cursor_last    puntatore che tiene conto dell'ultima sequenza più lunga trovata.
 * endOfBuffer      puntatore all'ultima cella del buffer.
 *
 * @param infile
 * @param outfile
 * @return
 */
int LZ77_compressor(FILE *infile, FILE *outfile){

    //VARIABLES
    unsigned char bytes_from_file[STREAM_SIZE];         //Array che contiene i byte letti dal file di input
    inizializeCharArray(bytes_from_file, STREAM_SIZE);

    int l_counter=0;
    int w_counter=0;
    int longest_seq=0;
    int longest_seq_offset=0;
    int bytes_readed=0;         //variabile che tiene traccia del numero di byte letti da file
    unsigned char temp='\0';

    //Buffered Writing Array
    int buffer[BUFFER_SIZE];
    inizializeArray(buffer,BUFFER_SIZE);

    //ALLOCAZIONE MEMORIA STRUTTURA PER CODIFICHE
    struct code *code = malloc(sizeof(struct code));

    //POINTERS
    unsigned char *endOfBuffer;     //puntatore a ultimo elemento di bytes_from_file
    unsigned char *lookahead;       //inizio look-ahead
    unsigned char *window;          //inizio search-buffer
    unsigned char *l_cursor;        //puntatore che si muove nel look-ahead buffer
    unsigned char *w_cursor;        //puntatore che si muove nel search-buffer
    unsigned char *w_cursor_last;   //puntatore che segna l'inizio dell'ultimo prefisso più lungo trovato


    //ALGORITMO DI RICERCA SEQUENZE

    while(bytes_readed+=fread(bytes_from_file, sizeof(char), STREAM_SIZE, infile)) {

        endOfBuffer = &bytes_from_file[bytes_readed]+1;     //aggiornamento puntatore fine buffer
        bytes_readed=0;

        //La ricerca parte dal nuovo buffer
        lookahead = &bytes_from_file[0];
        window=lookahead;
        l_cursor=lookahead;
        w_cursor=lookahead;

        //Finchè non riaggiunge la fine del buffer l'algoritmo continua la ricerca
        while (l_cursor != endOfBuffer && l_cursor+1 != endOfBuffer) {

            //Se non viene trovata alcuna sequenza codice: (0, 0, valore lookahead)
            if (longest_seq == 0) {
                code->o = 0;
                code->l = 0;
                code->a = *lookahead;

                bufferizedWriting(code, buffer, outfile);       //bufferizzazione della codifica

                //SLIDING FINESTRA E LOOKAHAED
                lookahead++;
                if ((lookahead - window) > WINDOW)
                    window++;

            } else if (longest_seq > 0) {
                code->o = longest_seq_offset;
                code->l = longest_seq;
                code->a = temp;
                bufferizedWriting(code, buffer, outfile);       //bufferizzazione della codifica

                //SLIDING FINESTRA E LOOKAHAED
                lookahead = lookahead + (longest_seq + 1);
                longest_seq = 0;

                if ((lookahead - window) > WINDOW) {
                    window = lookahead - WINDOW;
                }
            } else {
                printf("!WARNING! Encoding error, negative length value.");
                break;
            }

            //La ricerca riparte dalla nuova posizione del puntatore lookahead
            l_cursor = lookahead;
            w_cursor = lookahead;
            l_counter = 0;
            w_counter = 0;


            /* RICERCA PREFISSI */

            do{
                //controllo che la cella che segue il lookahead non è l'ultima
                if((lookahead + 1) != endOfBuffer){
                    w_cursor--;
                    w_counter++;
                    if (*w_cursor == *l_cursor) {
                        w_cursor_last = w_cursor;
                        do {
                            w_cursor++;
                            l_cursor++;
                            l_counter++;

                            //muovo in parallelo l_cursor e w_cursor finchè il loro valore è uguale
                        } while (*w_cursor == *l_cursor && l_cursor != lookahead + (LOOKAHEAD - 1) &&
                                 (*(l_cursor+1) != *endOfBuffer));

                        if(l_counter >= longest_seq){           //controllo se nuova sequenza è > di vecchia sequenza.
                            longest_seq = l_counter;
                            longest_seq_offset = w_counter;
                            temp = *l_cursor;
                        }
                        l_counter = 0;
                        l_cursor = lookahead;
                        w_cursor = w_cursor_last;               //Reset posizione del puntatore a finestra
                    }
                } else {
                    code->o = 0;
                    w_cursor = window;
                }
            } while (w_cursor != window && (*(l_cursor+1) != *endOfBuffer));  //w_cursor si muove fino a quando incontra window
        }
    }

    //Scrittura dell'ultimo buffer della scrittura bufferizzata
    if(bits!=0 && bits!=8){
        decimal = binToDec(buffer);
        fputc(decimal, outfile);
    }

    free(code);

    return 1;
}











/***********************************************************************************************************************
                                                   DECOMPRESSIONE
***********************************************************************************************************************/

void initializeCharArray(unsigned char *array, int size){

    for(int i=0; i<size; i++)
    {
        array[i]='\0';
    }
}

void initializeIntArray(int *array, int size){

    for(int i=0; i<size; i++)
        array[i]=0;
}

/***********************************************************************************************************************
 * void fillBuffer_d(int [], int)
 *
 * questa funzione riempie il buffer per la lettura bufferizzata, l'unica differenza rispetto alla funzione fillBuffer
 * della scrittura bufferizzata è il fatto che non è necessario gestire la conversione da binario a decimale visto che
 * il buffer non viene usato per scrivere codici su file ma solo per ricavare i bit delle codifiche.
 *
 * @param buffer //buffer contentente i bit debufferizzati
 * @param bit    //bit bufferizzato da inserire
 */
void fillBuffer_d(int buffer[], int bit){
    if(bits<BUFFER_SIZE)
    {
        buffer[bits] = bit;
        bits++;
    }
}

int getNextCode(FILE **infile){
    return fgetc(*infile);
}

/***********************************************************************************************************************
 * void decToBin_d(int [], int, int)
 *
 * Stesso funzionamento di decToBin.
 *
 * @param buffer    //buffer in cui inserire i bit debufferizzati
 * @param n         //numero decimale in cui sono bufferizzati gli elementi di codifica
 * @param b_size
 */
void decToBin_d(int buffer[], int n, int b_size){
    for(int i=b_size-1; i>=0; i--)
    {
        if(n >=((int)pow(2,i)) )
        {
            fillBuffer_d(buffer, 1);
            n = n - ((int) pow(2, i));
        }else
            fillBuffer_d(buffer, 0);
    }
    bits=0;
}

/***********************************************************************************************************************
 * int extractCodes(int [], FILE *)
 *
 * Funzione fondamentale per la decompressione.
 * Il buffer che contiene i bit debufferizzati viene passato come argomento e la variabile globale n_bits indica quanti
 * bit è necessario estrapolare dal buffer.
 * Con la variabile globale buffer_position si tiene conto della posizione in cui bisogna prendere il bit, una volta
 * presi tutti i bit (buffer_position = 0) viene letto il nuovo byte da debufferizzare e viene effettuata la conversione
 * decimale a binario per caricare il buffer con i nuovi bit.
 *
 * Ad ogni ciclo viene aggiornato il valore di decimal sommando la nuova potenza di 2.
 *
 * Esempio:
 * buffer[] = 0 0 0 1 0 1 1 0           ----> decimal = 2^4 + 2^2 + 2^1 = 20
 *
 * Chiaramente il peso di ogni bit non dipende da buffer_position ma viene ricalcolato ogni volta in base a n_bits.
 *
 * n_bits varia in base all'elemento di codifica che vogliamo estrapolare:
 *
 * -offset (o): log2(WINDOW)    --> dipende dalla grandezza del searchbuffer
 * -length (l): log2(LOOKAHEAD) --> dipende dalla grandezza del look-ahead buffer
 * -nextchar (a): 8             --> caratteri ASCII sono rappresentabili con 1byte
 *
 *
 *
 * @param buffer    --> buffer contenente i bit debufferizzati
 * @param infile
 * @return          --> ritorna il decimale che rappresenta l'elemento di codifica che si vuole estrarre
 */
int extractCodes(int buffer[], FILE *infile){
    n_bits--;
    int decimal = 0, c=0;

    while(n_bits>=0){

        if(buffer_position==0) {
            buffer_position=8;

            c=getNextCode(&infile);
            //se i codici sono finiti ritorna EOF
            if(c!=EOF) {
                decToBin_d(buffer, c, BUFFER_SIZE);
            }else {
                return EOF;
            }
        }

        //conversione binario/decimale
        decimal = decimal + ( (buffer[8-buffer_position]) * ((int) pow(2,n_bits) ));

        n_bits--;
        buffer_position--;
    }
    return decimal;
}

/***********************************************************************************************************************
 * int LZ77_decompressor(FILE *infile, FILE *outfile
 *
 * La funzione di decompressione si occupa di "pilotare" la lettura bufferizzata e di scrivere a blocchi i byte che
 * che vengono decompressi.
 *
 * Inizialmente viene letto il primo byte dal file compresso e la prima operazione che si fa è estrapolare i bit che
 * rappresentano la lunghezza del prefisso poichè come spiegato nella scrittura bufferizzata, length è il primo elemento
 * delle codifiche che viene bufferizzato.
 *
 * A questo punto il programma in base al valore di length decide se leggere solamente il nextchar oppure prima l'offset
 * e poi il nextchar.
 *
 * Per dire al programma quanti bit deve estrapolare dai codici bufferizzati è sufficiente modificare il valore della
 * variabile globale n_bits (vedi funzione extractCodes)
 *
 * Ogni volta che si ottiene un elemento di codifica lo si assegna alla variabile giusta della struttura code.
 * Una volta che si ha a disposzione la tripla, essa viene inserita nell'array di strutture d[].
 *
 * Questo processo viene ripetuto fino a quando d[] risulta pieno. A questo punto il vero e proprio algoritmo di
 * decompressione prende codifica per codifica e in base ai valori di length, offset e nextchar scrive i byte nell'
 * array decompressed.
 * Per effettuare la decompressione vengono spostati i puntatori d_window e d_lookahead (vedi documentazione).
 *
 * Se l'array decompressed viene riempito completamente si effettua la scrittura su file di quest'ultimo con la funzione
 * di libreria fwrite.
 *
 * A questo punto è necessario reinizializzare l'array decompressed copiando gli ultimi n byte dell'array appena scritto
 * sul file. Mi servono gli ultimi n byte poichè il decompressore si basa sui byte che sono gia stati decompressi.
 * Una volta copiati i byte, il puntatore lookahead viene riposizionato sull'elemento n-1 dell'array.
 *
 * n = grandezza del search buffer.
 *
 * Il processo viene ripetuto fino a quando non vengono letti tutti i byte dal file compresso.
 *
 * @param infile
 * @param outfile
 * @return
 */
int LZ77_decompressor(FILE *infile, FILE *outfile){

    //Variabili
    int c, i=0;
    int buffer[BUFFER_SIZE];
    int s=0;
    int decimal = 0;
    int n = 0;
    n_bits=(int)log2(LOOKAHEAD);

    initializeCharArray(decompressed, STREAM_SIZE);

    //PUNTATORI
    unsigned char *last_element = &decompressed[STREAM_SIZE-1]; //fine array bytes decompressi
    unsigned char *d_lookahead = decompressed;                  //inizio look-ahead buffer
    unsigned char *d_window = d_lookahead;                      //inizio search-buffer
    unsigned char *w_cursor;                                    //inizio prefisso

    initializeIntArray(buffer, BUFFER_SIZE);

    //INIZIALIZZAZIONE PUNTATORI
    last_element = &decompressed[STREAM_SIZE-1];
    d_lookahead = decompressed;

    struct code d[STRUCT_ARRAY_SIZE]; //array di strutture che contenente le codifiche

    //Leggo il primo byte dal file compresso
    c=fgetc(infile);
    decToBin_d(buffer, c, BUFFER_SIZE);
    decimal = extractCodes(buffer, infile);

    //DEBUFFERIZZAZIONE
    while(decimal!=EOF){
        while (s < STRUCT_ARRAY_SIZE) {
            if(decimal == 0){
                d[s].l = decimal;
                d[s].o = decimal;
                n_bits = 8;
                decimal = extractCodes(buffer, infile);
                d[s].a = (unsigned char) decimal;
                //printf("\n(%d, %d, %c)", d[s].o, d[s].l, d[s].a);
            }else{
                d[s].l = decimal;
                n_bits = (int)log2(WINDOW);
                decimal = extractCodes(buffer, infile);
                decimal = decimal + 1; //Sommo 1 perchè nella scrittura bufferizzata toglievo 1 per poterlo rappresentare al massimo
                d[s].o = decimal;
                n_bits = 8;
                decimal = extractCodes(buffer, infile);
                d[s].a = (unsigned char) decimal;
                //printf("\n(%d, %d, %c)", d[s].o, d[s].l, d[s].a);
            }

            n_bits = (int)log2(LOOKAHEAD);

            if(decimal!=EOF)
                decimal = extractCodes(buffer, infile);
            else
                break;

            s++;
        }

        n = 0;

        //AGGIORNO D_WINDOW --> puntatore che si muove all'interno del searchbuffer

        d_window = d_lookahead;

        //DECOMPRESSIONE
        while(n < s){
            w_cursor = d_lookahead;

            //SE LENGTH = 0 copio semplicemente nextchar
            if (d[n].l == 0) {
                *d_lookahead = d[n].a;
                d_lookahead++;

                //Reinizializzazione array decompressed
                if(d_lookahead==last_element){
                    fwrite(decompressed, sizeof(unsigned char), STREAM_SIZE, outfile);
                    //stampaArray(decompressed, STREAM_SIZE);
                    for(int j=WINDOW, k=0; j>-1; j--, k++) {
                        decompressed[k] = *(last_element - j);
                    }
                    d_lookahead=&decompressed[WINDOW];
                    d_window = d_lookahead;
                }

                if((d_lookahead - d_window) > WINDOW)
                    d_window++;
            } else {    //SE LENGTH != 0 torno indietro di offset e faccio una copia parallela con i 2 puntatori
                i = d[n].o;
                while (i > 0) {
                    i--;
                    w_cursor--;
                }
                i = 0;
                while(i < d[n].l){
                    *d_lookahead = *w_cursor;
                    d_lookahead++;

                    //Reinizializzazione array decompressed
                    if(d_lookahead==last_element){
                        //stampaArray(decompressed, STREAM_SIZE);
                        fwrite(decompressed, sizeof(unsigned char), STREAM_SIZE, outfile);
                        for(int j=WINDOW, k=0; j>-1; j--, k++) {
                            decompressed[k] = *(last_element - j);
                        }
                        d_lookahead=&decompressed[WINDOW];
                        d_window = d_lookahead;
                    }

                    w_cursor++;
                    if ((d_lookahead - d_window) > WINDOW)
                        d_window++;

                    i++;
                }
                *d_lookahead = d[n].a;
                d_lookahead++;

                //Reinizializzazione array decompressed
                if(d_lookahead==last_element){
                    fwrite(decompressed, sizeof(unsigned char), STREAM_SIZE, outfile);
                    //stampaArray(decompressed, STREAM_SIZE);
                    for(int j=WINDOW, k=0; j>-1; j--, k++) {
                        decompressed[k] = *(last_element - j);
                    }
                    d_lookahead=&decompressed[WINDOW];
                    d_window = d_lookahead;
                }
            }

            n++;
        }

        //scrittura blocco di byte decompressi
        fwrite(decompressed, sizeof(unsigned char), d_lookahead-(&decompressed[0]), outfile);
        s=0;
    }

    return 0;
}

/***********************************************************************************************************************
 * void file_size(FILE, FILE)
 *
 * funzione che stampa il peso dei file di input e di output
 *
 * @param infile
 * @param outfile
 */
void file_size(FILE *infile, FILE *outfile){
    fseek(infile, 0L, SEEK_END);
    long isz = ftell(infile);
    fseek(infile, 0L, SEEK_SET);
    rewind(infile);
    printf("\nInput file size: %lu\n", isz);

    fseek(outfile, 0L, SEEK_END);
    long osz = ftell(outfile);
    fseek(outfile, 0L, SEEK_SET);
    rewind(outfile);
    printf("Output file size: %lu\n", osz);
}

void time_start(){
    clock_t begin = clock(); //start measuring time
}

void time_stop(){
    clock_t end = clock(); // stop measuring time
    printf("\nExecution Time:  ");
    printf("%f", ((double) (end - begin) / CLOCKS_PER_SEC));
    printf(" [seconds]:  ");
}

/***********************************************************************************************************************
                                                        MAIN
***********************************************************************************************************************/
int main(int argc, char *argv[]) {
    printf("\n/************************************************************************************/\n");
    printf("ALGORITMO LZ77\nSviluppato da: Ivan Pavic\nUltima modifica: 19.01.2018\n");

    FILE *infile = fopen(argv[2], "rb");
    FILE *outfile;


    if (argc < 4) {
        printf("!WARNING! Too little arguments detected (%d) in documentation file.\n", argc);
    } else if (argc > 4) {
        printf("!WARNING! Too many arguments detected (%d) in documentation file.\n", argc);
    } else {
        if (infile == NULL) {
            printf("!WARNING! Input file doesn't exists!");
        } else if ((outfile = fopen(argv[3], "wb")) == NULL) {
            printf("!WARNING! Output file doesn't exists!");
        } else{
            if (!strcmp(argv[1], "-c")) {

                printf("\n/*************************************COMPRESSOR*************************************/\n");
                printf("\nCHEKING FILES VALIDITY\n");

                printf("\nFILES OK.\n");

                time_start();
                LZ77_compressor(infile, outfile);
                time_stop();

                /*FILE SIZE PRINTING*/
                file_size(infile, outfile);

            } else if (!strcmp(argv[1], "-d")) {
                printf("\n/*************************************DECOMPRESSOR*************************************/\n");

                time_start();
                LZ77_decompressor(infile, outfile);
                time_stop();

            } else {
                printf("!WARNING! Wrong first argument (%s), must be [-c] or [-d]\n", argv[1]);
            }

            printf("\n/*****************************************END******************************************/\n");

        }
    }
    fclose(infile);
    fclose(outfile);
    return 0;
}

/***********************************************************************************************************************
                                                     END OF CODE
***********************************************************************************************************************/
