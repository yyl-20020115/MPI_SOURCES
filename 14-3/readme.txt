Example:
���룺mpicc app_match.c �Co app_match
���У�����ʹ������ mpirun �Cnp SIZE app_match m n k�����иô�ƥ���������SIZE����ʹ�õĴ�����������m��ʾ�ı������ȣ�nΪģʽ�����ȣ�kΪ�������ȡ���ʵ����ʹ����SIZE=3����������m=7��n=2��k=1��
����mpirun �Cnp 3 app_match 7 2 1
���н����
on node 0 the text is as
on node 0 the pattern is as
Total 2 matched on node 0
on node 1 the text is as
on node 1 the pattern is as
Total 2 matched on node 1
on node 2 the text is bsb
on node 2 the pattern is as
Total 2 matched on node 2
˵����������ʵ���У����ı�������Ϊ7������������ı���Ϊasasbsb���ֲ���3���ڵ��ϣ�ģʽ������Ϊ2�����������ģʽ��Ϊas����󣬽ڵ�0��1��2�Ϸֱ�õ���������ƥ��λ�á�

