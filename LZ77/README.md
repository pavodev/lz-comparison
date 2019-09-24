# LZ77 - Run instructions
These commmands have been tested on a Unix based system.

* Compile the file main.c with the following command : 	
	
```sh 
gcc main.c -lm -o main
```

* To run the compressor use: 
	
```sh 
./main -c inputfile outputfile
```

* To run the decompressor use:
```sh
./main -d inputfile outputfile
```

* To set the size of the search buffer and the look-ahead buffer, open main.c and modify the following definitions:
  * #define LOOKAHEAD 8
  * #define WINDOW 8192

If the uncompressed or compressed files are located in the same folder as main.c, it is NOT necessary the absolute path in the commands.
