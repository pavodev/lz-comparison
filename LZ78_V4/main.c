/*
 * Titolo: Algoritmo LZ78
 *
 * Corso: Algoritmi e strutture dati
 * Autore: Elia Perrone
 * Classe: I2A
 *
 * Descrizione: Progetto inerente gli algoritmi di compressione
 * Inizio:  18.09.2017
 * Fine:    19.01.2018
 *
 *
 * Descrizione:
 * LZ78 è un'algoritmo di compressione che abbandona il concetto di scorrimento della finestra, utilizzato nell'algorit-
 * mo LZ77, e che implementa l'ideaologia di compressione sui dati futuri, e non sui dati già passati.
 * Inoltre, LZ78 deve implementare la gestione di un dizionario.
 * Di volta in volta, scorrendo l'intero file da comprimere, l'algoritmo LZ78 ricerca all'interno del suo dizionario,
 * stringhe composte da sottostringhe, dove quest'ultime sono seguite da un carattere che rompe la sequenza di matching.
 * Fattore importante di questo algoritmo, in confronto con l'LZ77, è che non abbiamo una restrizione su quando indietro
 * puo essere ricercato un match. Data la gestione di un dizionario possiamo affermare che, se una stringa è memoriz-
 * zata nel dizionario allora lo è anche un suo prefisso.
 * Inoltre, il dizionario alla fine della compressione non viene inviato dunque, grazie alla flessibilità dell'algorit-
 * mo possiamo ricostruirlo passo a passo durante la decompressione in maniera automatica.
 *
 *
 * Pseudo-codice:
 *
 *   1:  begin
 *   2:     initialize a dictionary by empty phrase P
 *   3:     while (not EOF) do
 *   4:      begin
 *   5:        readSymbol(X)
 *   6:        if (F.X> is in the dictionary) then
 *   7:           F = F.X
 *   8:        else
 *   9:         begin
 *  10:           output(pointer(F),X)
 *  11:           encode X to the dictionary
 *  12:           initialize phrase F by empty character
 *  13:         end
 *  14:        end
 *  15:     end
 *
 *
 * Scrittura bufferizzata:
 *
 *      **************************             Per ogni indice dell'elemento del dizionario conosco la grandezza, ovvero
 *      *   INDICE   *     bit   *             quanti bit occurrono, per rappresentarlo.
 *      **************************             La scrittura bufferizzata mi permette di usufruire di una grandezza
 *      *      1      *    1     *             variabile dell'indice in maniera tale da poter risparmiare bit da scrivere
 *      *      2      *    2     *             sul file compresso.
 *      *      3      *    2     *
 *      *      4      *    3     *
 *      *      5      *    3     *             Al contrario durante la decompressine saprò che grandezza ha l'indice
 *      *     ...     *   ...    *             dell'elemento da aggiungere nel dizionario dunque potro estrarre facil-
 *      *      n      * log2(n)  *             mente le codifiche dal file compresso.
 *      **************************
 *
 *
 * Implementazioni mancanti:    1. scrittura bufferizzata per la compressione
 *                              2. lettura bufferizzata per la decompressione
 *                              3. gestione del programma tramite comandi da terminale
 *
 *
 * Problemi nel codice:         1. compressione: strutture dati non efficienti
 *                              2. decompressione: gestire gli indici con lettura bufferizzata
 *
 */

/*********************************************** LIBRERIE *************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

/************************************************ DEFINE **************************************************************/

#define VALUE_SIZE 100              // grandezza valore all'interno del dizionario (100 byte max)
#define BUFFER_SIZE 1000            // grandezza buffer d'inserimento del file da comprimere (1000 byte max)
#define DICTIONARY_SIZE 10000       // quantità di elementi nel dizionario (10000 elementi)
#define BITBUFFER_SIZE 16           // grandezza in bit dei campi dell'output (fisso 16 bit max) (grandezza usata senza scrittura bufferizzata)
#define WINDOW_SIZE 10              // finestra di ricerca valore (grandezza della sottostringa ricercata) (10 byte max)

/**********************************************  STRUTTURE  ***********************************************************/

// Struttura che rappresenta la codifica generata dall'algoritmo di compressione (indice,carattere successivo)
typedef struct _output{
    unsigned int index;                     // indice che fa riferimento ad un elemento all'interno del dizionario
    unsigned char next_value;               // carattere successivo che interrompe la sequenza ricercata all'interno del dizionario
}Output;

// Struttura che rappresenta l'elemento all'interno del dizionario (indice,valore)
typedef struct _element{
    unsigned int index;                     // indice di riferimento di un elemento all'interno del dizionario
    unsigned char value[VALUE_SIZE];        // valore dell'elemento nel dizionario
}Element;

/********************************************* VARIABILI GLOBALI ******************************************************/

// indice generale per l'aggiunta di un elemento nel dizionario
unsigned int global_index = 0;

// variabile utilizzata per la conversione decimale - binaria
unsigned int bit = 0;

/***********************************************************************************************************************
                                                  FUNZIONI
***********************************************************************************************************************/

/*
 * void inizialize_dictionary(Element d[], unsigned int dictionary_size, unsigned int value_size)
 *
 * Inizializzazione del dizionario (tutti i campi sono settati al valore nullo del tipo di dato corrispondente)
 *
 */

void inizialize_dictionary(Element d[], unsigned int dictionary_size, unsigned int value_size){
    for (int i = 0; i < dictionary_size; i++) {
        d[i].index=0;
        for (int j = 0; j < value_size; j++) {
            d[i].value[j] = '\0';
        }
    }
}

/**********************************************************************************************************************/

/*
 * void inizialize_buffer(unsigned char v[], unsigned int buffer_size)
 *
 * Inizializzazione di un buffer:
 * utilizzo di buffer all'interno della compressione che della decompressione (popolamento con file o sottostrighe)
 *
 */

void inizialize_buffer(unsigned char v[], unsigned int buffer_size){
    for (int i = 0; i < buffer_size; ++i) {
        v[i] = '\0';
    }
}

/**********************************************************************************************************************/

/*
 * void add_element(Element d[], unsigned int global_index, unsigned char value[], unsigned int value_size)
 *
 * Aggiunta di un'elemento nel dizionario:
 *
 * Tramite un indice globale gestisco l'aggiunta di elementi all'interno del dizionario.
 * Ogni volta che viene riscontrata una sequenza non presente nel dizionario essa viene aggiunta.
 *
 */

void add_element(Element d[], unsigned int global_index, unsigned char value[], unsigned int value_size){
    d[global_index].index = global_index+1;
    for (int i = 0; i < value_size; i++) {
        d[global_index].value[i] = value[i];
    }
}

/**********************************************************************************************************************/

/*
 * unsigned int search_element_by_value(Element d[], unsigned char value[], unsigned int dictionary_size, unsigned int value_size)
 *
 * Ricercare la presenza di un elemento all'interno del dizionario avendo a disposizione il suo valore
 *
 * @return  0 -> se non è stato trovato nessun elemento corrispodennte al valore richiesto
 *          n -> dove n è il valore dell'indice dell'elemento nel dizionario corrispondente al valore richiesto
 *
 */

unsigned int search_element_by_value(Element d[], unsigned char value[], unsigned int dictionary_size, unsigned int value_size){
    unsigned int index=0,count=0;
    for (int i = 0; i < dictionary_size; i++) {
        for (int j = 0; j < value_size; j++) {
            if(d[i].value[j]==value[j]) {           // incremento un contatore se ogni posizione dell'array contenente il
                count++;                            // valore dell'elemento nel dizionario corrisponde con il valore richiesto
            }
            else{
                count=0;                            // in caso contrario resetto il contatore
                break;
            }
        }
        if(count==value_size){                      // se l'intero valore dell'elemento del dizionario corrisponde con
            index = d[i].index;                     // il valore richiesto allora estraggo l'indice dell'elemento
        }
    }
    return index;                                   // ritorno l'indice dell'elemento (0 se non è stato trovato)
}

/**********************************************************************************************************************/

/*
 * void search_element_by_index(Element d[], unsigned int index, unsigned char value[], unsigned int dictionary_size, unsigned int value_size)
 *
 * Ricercare la presenta di un elemento all'interno del dizionario avendo a disposizione il suo indice
 * Una volta trovata la corrispondenza dell'indice nel dizionario la funzione mi associa il valore estratto dall'elemento del
 * dizionario ad un valore (array di char)
 *
 */

// Ricerca la presenza di un elemento all'interno del dizionario avendo a disposizione il suo indice
void search_element_by_index(Element d[], unsigned int index, unsigned char value[], unsigned int dictionary_size, unsigned int value_size){
    for (int i = 0; i < dictionary_size; i++) {
        if(d[i].index==index) {
            for (int j = 0; j < value_size; j++) {
                value[j]=d[i].value[j];
            }
        }
    }
}

/***********************************************************************************************************************
                                               SCRITTURA SU FILE
 **********************************************************************************************************************/

/*
 * void fill_buffer(unsigned int buffer[], unsigned int value, unsigned int buffer_size)
 *
 * Riempimento di un buffer con grandezza variabile (BUFFER_SIZE)
 *
 */

void fill_buffer(unsigned int buffer[], unsigned int value, unsigned int buffer_size){
    if(bit<buffer_size){
        buffer[bit] = value;
    } else {
        bit=0;
    }
    bit++;
}

/**********************************************************************************************************************/

/*
 * void decimal_binary(unsigned int decimal, unsigned int buffer[], unsigned int buffer_size)
 *
 * Conversione di un valore da decimale a binario (valore inserito all'interno di un buffer)
 *
 */

void decimal_binary(unsigned int decimal, unsigned int buffer[], unsigned int buffer_size) {
    for (int i = buffer_size-1; i >= 0; i--) {
        if(decimal >= ((int)pow(2,i))){
            fill_buffer(buffer,1,BITBUFFER_SIZE);
            decimal = decimal-((int)pow(2,i));
        } else {
            fill_buffer(buffer,0,BITBUFFER_SIZE);
        }
    }
}

/**********************************************************************************************************************/

/*
 * unsigned int binary_decimal(unsigned int buffer[], unsigned int buffer_size)
 *
 * Conversione di un valore da binaria a decimale (valore inserito in una variabile)
 *
 * @return decimal
 *
 */

unsigned int binary_decimal(unsigned int buffer[], unsigned int buffer_size){
    unsigned int decimal=0;
    for(int i=0; i<buffer_size; i++){
        decimal = decimal+buffer[BITBUFFER_SIZE-1-i]*((int)pow(2,i));
    }
    return decimal;
}

/**********************************************************************************************************************/

/*
 * void output_writing_file(Output *output, unsigned char buffer[], FILE *output_file)
 *
 * Una volta creato il codice ne eseguo una scrittura sul file compresso
 *
 */

void output_writing_file(Output *output, unsigned char buffer[], FILE *output_file){
    unsigned char bits_value='\0';
    decimal_binary(output->index,buffer,BITBUFFER_SIZE);
    bits_value = binary_decimal(buffer,BITBUFFER_SIZE);
    fprintf(output_file, "%d", bits_value);
    bits_value='\0';
    decimal_binary(output->next_value, buffer,BITBUFFER_SIZE);
    bits_value = binary_decimal(buffer,BITBUFFER_SIZE);
    fprintf(output_file, "%c", bits_value);
}

/***********************************************************************************************************************
Attenzione: Le 4 funzioni riportate in questa sezione (scrittura bufferizzata) andrebbero migliorate implementando il
            sistema di bufferizzazione argomentato all'inizio del file (metodo degli indici).
***********************************************************************************************************************/

/*
 * void append(unsigned char s[], unsigned char c)
 *
 * Concatenazione di un char alla fine di una stringa (utilizzata per la decompressione)
 *
 */

void append(unsigned char s[], unsigned char c) {
    int len = strlen(s);
    s[len] = c;
    s[len+1] = '\0';
}

/**********************************************************************************************************************/

/*
 * void cleaning_memory(Element d[])
 *
 * Cancellazione del dizionario (da eseguire a fine compressio)
 */

void cleaning_memory(Element d[]){
    inizialize_dictionary(d,DICTIONARY_SIZE,VALUE_SIZE);
}

/***********************************************************************************************************************
                                                    MAIN
***********************************************************************************************************************/

int main() {

/***************************************** VARIABILI PER LA COMPRESSIONE  *********************************************/

    // File
    FILE *input_file;       // File da comprimere
    FILE *output_file;      // File compresso
    FILE *output2_file;     // File decompresso (uguale al File da comprimere)

    // Buffer
    unsigned char buffer[BUFFER_SIZE];          // usato per il riempimento del File da comprimere
    inizialize_buffer(buffer,BUFFER_SIZE);
    unsigned char subbuffer[BUFFER_SIZE];       // usato per la ricerca di valori dell'input all'interno del dizionario
    unsigned int bitbuffer[BITBUFFER_SIZE];     // usato per la grandezza dell'output sul File compresso

    // Oggetti
    Output *output = malloc(sizeof(Output));                            // elemento output (... , ...)
    Element dictionary[DICTIONARY_SIZE];                                // elemento dizionario (... = ...)
    inizialize_dictionary(dictionary,DICTIONARY_SIZE,VALUE_SIZE);

/************************************************ COMPRESSIONE ********************************************************/

    // Inizio algoritmo di compressione
    printf("\n");
    printf("COMPRESSIONE -> ");

    // Inizio calcolo tempo di compressione
    clock_t begin = clock();

    // Apertura del file da comprimere (va inserito il percorso del file che si vuole comprimere)
    input_file = fopen("percorso file da comprimere", "rb");
    if(input_file==NULL) printf("Errore nell'apertura del file input");

    // Apertura del file compresso (va inserito il percorso del file compresso che verrà generato dalla compressione)
    output_file = fopen("percorso del file compresso", "wb");
    if(output_file==NULL) printf("Errore nell'apertura del file output");


    // Algoritmo di compressione
    while(fread(buffer, sizeof(unsigned char), BUFFER_SIZE, input_file)) {          // riempio buffer con i primi 1000 byte del file da comprimere
        unsigned int i = 0, j = 1;                                                  // i e j sono due variabili per muovermi all'interno dei buffer
        while(buffer[i]!='\0') {                                                    // finchè non mi trovo alla fine del buffer
            inizialize_buffer(subbuffer,BUFFER_SIZE);
            memcpy(subbuffer, &buffer[i], j);                                                           // inserisco una sottostringa di buffer in subbuffer
            if (search_element_by_value(dictionary,subbuffer,DICTIONARY_SIZE,VALUE_SIZE) != 0) {        // controllo se la sottostringa è presente nel dizionario
                j++;                                                                                    // se è presente incremente j (aumento la sottostringa in subbuffer)
            } else {
                add_element(dictionary,global_index,subbuffer,VALUE_SIZE);                              // aggiungo il valore ricercato all'intenro di un nuovo elemento del dizionario
                global_index++;
                if(global_index==DICTIONARY_SIZE) {                                         // se ho raggiunto 10000 elementi all'interno del dizionario lo inizializzo e parto con un nuovo dizionario (aumenta la velocità di compressione)
                    global_index=0;
                    inizialize_dictionary(dictionary,DICTIONARY_SIZE,VALUE_SIZE);
                }
                j--;
                if (j == 0) {                               // se è un carattere nuovo j = 0
                    output->index = 0;
                    output->next_value = buffer[i];
                    output_writing_file(output,bitbuffer,output_file);      // scrivo al codifica ottenuta nel file compresso
                    i++, j = 1;                                             // aggionro gli indici di spostamento dei buffer
                } else {
                    inizialize_buffer(subbuffer,WINDOW_SIZE);
                    memcpy(subbuffer, &buffer[i], j);                                                                   // torno alla sottostringa ricercata precedentemente
                    output->index = search_element_by_value(dictionary,subbuffer,DICTIONARY_SIZE,VALUE_SIZE);
                    output->next_value = buffer[i + j];
                    output_writing_file(output,bitbuffer,output_file);                                                  // scrivo al codifica ottenuta nel file compresso
                    i = i + j + 1, j = 1;                                                                               // aggionro gli indici di spostamento dei buffer
                }
            }
        }
    }
    fclose(input_file);         // chiudo il file da comprimere
    fclose(output_file);        // chiudo il file compresso

    // Cancellazione dizionario
    cleaning_memory(dictionary);
    global_index=0;

    // Fine calcolo del tempo di compressione
    clock_t end = clock();
    printf("Execution Time:  ");
    printf("%f", ((double) (end - begin) / CLOCKS_PER_SEC));
    printf(" [seconds]\n ");

/***********************************************************************************************************************
Attenzione: La compressione allo stato attuale è funzionante solamente per i file di testo (questo perchè vengono utili-
            zzate funzioni che possono essere eseguite unicamente su stringhe (string.h)).
            Inoltre modificando le grandezze delle strutture (dizionario e buffer prevalentemente) la compressione non
            funziona perchè gli array non accedono correttamente alle posizoini richieste. Questo problema è dato sop-
            rattuto dal tipo di strutture utilizzata (non efficienti per un algoritmo di questo genere).
            Un passo successivo sarebbe sicuramente implementare una struttura ad albero in maniera tale da poter gesti-
            re in maniera piu ottimale le varie grandezze in gioco e soprattutto per migliore i tempi di compressione.
***********************************************************************************************************************/

/****************************************** VARIABILI PER LA DECOMPRESSIONE  ******************************************/

    // Buffer
    unsigned char debuffer[BUFFER_SIZE];            // usato per il riempimento del File da decomprimere
    inizialize_buffer(debuffer,BUFFER_SIZE);
    unsigned char next_char_value[VALUE_SIZE];      // usato per l'inserimento del carattere successivo
    unsigned char value[VALUE_SIZE];                // usato per l'inserimento del valore del dizionario

    unsigned int index_value,k=0;

/************************************************* DECOMPRESSIONE *****************************************************/

    printf("\n");
    printf("DECOMPRESSIONE -> ");

    // Inizio calcolo tempo di decompressione
    clock_t begin2 = clock();

    // Apertura del file da decomprimere (va inserito il percorso del file che si vuole decomprimere)
    output_file = fopen("percorso file da decomprimere", "rb");
    if(output_file==NULL) printf("Errore nell'apertura del file output");

    // Apertura del file decompresso (va inserito il percorso del file decompresso che verrà generato dalla decompressione)
    output2_file = fopen("percorso file decompresso", "wb");
    if(output2_file==NULL) printf("Errore nell'apertura del file");

    // Algoritmo di decompressione
    while (k<BUFFER_SIZE) {
        debuffer[k] = (unsigned char) fgetc(output_file);   // popolo il debuffer con i codici da decodificare
        k++;
    }
    unsigned int i=0,j=0;
    for (int l = 0; l < k; l++) {
        inizialize_buffer(value,VALUE_SIZE);
        inizialize_buffer(next_char_value,VALUE_SIZE);
        index_value = debuffer[i];                          // estraggo l'indice
        next_char_value[j] = debuffer[i + 1];               // estraggo il carattere successivo
        if (index_value == '0') {                                                                       // se l'indice è 0
            add_element(dictionary, global_index, next_char_value,VALUE_SIZE);                          // aggiungo il carattere successivo al dizionario
            global_index++;
            fputc(debuffer[i + 1], output2_file);           // scrivo il carattere successivo sul file
            i = i + 2;                                      // aggiorno gli indici
        } else {
            search_element_by_index(dictionary, index_value-'0', value,DICTIONARY_SIZE,VALUE_SIZE);             // ricerco il valore all'interno del dizionario tramite l'indice
            append(value, next_char_value[j]);                                                                  // concateno il valore del dizionario con il carattere successivo
            add_element(dictionary, global_index, value,VALUE_SIZE);                                            // aggiungo l'occorenza al dizionario
            global_index++;
            fwrite(value, sizeof(unsigned char), VALUE_SIZE, output2_file);         // scrivo su file il valore del dizionario + il carattere successivo
            i = i + 2;                                                              // aggiorno gli indici
        }
    }
    fclose(output_file);    // chiudo il file da decomprimere
    fclose(output2_file);   // chiudo il file decompresso

    // Fine calcolo tempo di decompressione
    clock_t end2 = clock();
    printf("Execution Time:  ");
    printf("%f", ((double) (end2 - begin2) / CLOCKS_PER_SEC));
    printf(" [seconds]\n ");

    free(output);       // libero la memoria occupata dalla codifica

/***********************************************************************************************************************
Attenzione: La decompressione allo stato attuale è funzionanete solo per gli indici degli elementi del diziona-
            rio minori di 10, questo perchè non viene gestita la lunghezza dell'indice (l'idea era di implementare la
            lettura bufferizzata ma data l'assenza della scrittura bufferizzata non è stato possibile implementarla).
***********************************************************************************************************************/

    return 0;
}

/**********************************************************************************************************************/