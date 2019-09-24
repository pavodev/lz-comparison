LZ77:

Compilare il file main.c su terminale con il comando: 	--> gcc main.c -lm -o main


Per usare il compressore usare il comando: --> ./main -c inputfile outputfile 

Per usare il decompressore usare il comando: --> ./main -d inputfile outputfile 

Per modificare grandezza di searchbuffer e look-ahead buffer aprire il file main.c e modificare le seguenti definizioni:

#define LOOKAHEAD 8
#define WINDOW 8192



Se i file si trovano nella stessa cartella del file main.c NON è necessario specificare il percorso assoluto.