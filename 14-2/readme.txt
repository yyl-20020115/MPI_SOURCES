Example:
���룺mpicc rand_match.c �Co rand_match
���У�����ʹ������ mpirun �Cnp SIZE rand_match m n�����иô�ƥ���������SIZE����ʹ�õĴ�����������m��ʾ�ı������ȣ�nΪģʽ�����ȡ���ʵ����ʹ����SIZE=3����������m=7��n=2��
����mpirun �Cnp 3 rand_match 7 2
���н����
one node 0 n=7,m=2,p=41
one node 0 Text=00
one node 0 Pattern=01
on node 0 match pos is 2
one node 1 n=7,m=2,p=41
one node 1 Text=10
one node 1 Pattern=01
on node 1 match pos is 2
one node 2 n=7,m=2,p=41
one node 2 Text=100
one node 2 Pattern=01
˵����������ʵ���У����ı�������Ϊ7������������ı���Ϊ0010100���ֲ���3���ڵ��ϣ�ģʽ������Ϊ2�����������ģʽ��Ϊ01����󣬽ڵ�0��1�Ϸֱ�õ�һ��ƥ��λ�á�
