Example:
���룺mpicc sat.c -o sat
��1.������Ϊ5���Ӿ���ĿΪ3���Ӿ���������ĿΪ3
		(X1 V X2 V X3) ^ (~X1 V X2 V X4) ^ (X0 V ~X2 V ~X4) 
�����ļ�sample1����Ϊ
����1 2 3 0 -1 2 4 0 0 -2 -4 0
�������У�
����mpirun -np 3 sat sample1
����NVARS=10 NCLAUSES=3 LENGTH_CLAUSE=3
������Satisfied! time0.024580
��2.������Ϊ5���Ӿ���ĿΪ3���Ӿ���������ĿΪ3
		(X1 V X2 V X3) ^ (~X1 V X2 V X4) ^ (X0 V ~X2 V ~X4) 
�����ļ�sample2����Ϊ
����1 2 3 0 -1 2 4 0 0 -2 -4 0
�������У�
����mpirun -np 4 test sample
����NVARS=10 NCLAUSES=3 LENGTH_CLAUSE=3
����Satisfied! time0.018852
��3.������Ϊ5���Ӿ���ĿΪ3���Ӿ���������ĿΪ3
		(X1 V X1 V X1) ^ (~X1 V ~X1 V ~X1) ^ (X1 V X1 V X1) 
�����ļ�sample3����Ϊ
		1 1 1 0 -1 -1 -1 0 1 1 1 0
�������У�
����mpirun -np 6 test error
������NVARS=10 NCLAUSES=3 LENGTH_CLAUSE=3
������Unsatisfied!
������
