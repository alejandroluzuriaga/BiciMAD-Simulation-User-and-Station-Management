# BiciMAD Simulation

## _Implemented in C_
*Created by myself and @DarkS34 for our 'Operating Systems' course at University using Windows 10*

> [!TIP]  
> This program is runnable in Windows.

This C program simulates the operation of BiciMAD, a bike-sharing system, using threading and interactions between users and stations.

## Usage
Create a file using this syntax (only integer numbers):
```
400 // Users
12 // Stations
20 // Slots per station
1 // Minimum waiting time to decide to pick up a bike (in seconds)
4 // Maximum waiting time to decide to pick up a bike (in seconds)
1 // Minimum waiting time riding a bike (in seconds)
6 // Maximum waiting time riding a bike (in seconds)
```
Compile the code using:

```
gcc -Wall -Wextra -pthread biciMAD.c -o biciMAD
```
Execute the program:
```
./biciMAD [input_file] [output_file]
```
Follow the next table to run .exe file:

| Arguments | Input | Output |
| ------ | ------ | ------ |
| N/A| _'entrada_BiciMAD.txt'_ | _'salida_sim_BiciMAD<<date>date>.txt'_
| 1| _first argument_| _'salida_sim_BiciMAD<<date>date>.txt'_ |
| 2 |_first argument_ | _second argument_ |
> [!CAUTION]  
> Implementation only allows .txt files (input and output)

After execution, the simulation results will be saved in the output file according to the previous table.

