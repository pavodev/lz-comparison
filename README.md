# LZ-comparison - LZ77 & LZ78
The aim of this project was to compare two compression algorithms of the LZ family. 
This project has been developed by <a href="https://github.com/pavodev">pavodev</a>(LZ77) and <a href="https://github.com/eliaperrone">eliaperrone</a>(LZ78).

## Introduction
Day by day the amount of data exchanged is growing and files are increasingly sophisticated and heavy in terms of storage space.

The data compression algorithms have the task of identifying repetitions within any file and replacing these repetitions with codings that can be represented with the smallest possible number of bits while maintaining the possibility of obtaining the original state.

The two lossless compression algorithms we have chosen were LZ77 and LZ78.
This choice was made to analyze the differences and performance of two algorithms that are the basis of many data compression software used today, checking how they behave, what is their compression speed and what is the compression factor depending on the type of dictionary that is used: explicit in the case of LZ78 and implicit in that of LZ77.

This project was developed as a partial requirement for the course **Algorithms and data structures** at the <a href="http://www.supsi.ch/home_en.html">**University of Applied Sciences and Arts of Southern Switzerland**</a>.

## Additional informations
For the complete documentation please check the file LZ-Comparison-IT.pdf or LZ-Comparison-EN.pdf.
