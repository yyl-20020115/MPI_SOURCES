1. compile:
mpicc ludep.c -o ludep

2. run:
mpirun -np 3 ludep

3. result:
Input of file "dataIn.txt"
3        3
2.000000        1.000000        2.000000
0.250000        1.250000        1.500000
1.000000        -1.333334       -0.666667

Output of LU operation
Matrix L:
1.000000        0.000000        0.000000
0.250000        1.000000        0.000000
1.000000        -1.333334       1.000000
Matrix U:
2.000000        1.000000        2.000000
0.000000        1.250000        1.500000
0.000000        0.000000        -0.666667
