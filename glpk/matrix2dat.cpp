#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char *argv[]) {
	int n;
	cin>>n;
	vector<int> I;
	vector<int> J;
	int elementos = 0;
	int buf;
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			cin>>buf;
			if (buf != 0) {
				elementos++;
				if (i < j)
					swap(i,j);
				I.push_back(i);
				J.push_back(j);
			}
		}
	}
	int i;
	printf("data;\nparam NVALUES := %d;\n",elementos);
	//for (i=0; i<elementos; i++)
	//	printf("%d ",i+1);
	printf("\nparam Size:= %d;\n",n);
	printf("param InitialLine:=\n");
	for (i=0; i<elementos; i++)
		printf("%d %d\n",i+1,I[i]+1);
	printf(";\n\nparam InitialColumn:=\n");
	for (i=0; i<elementos; i++)
		printf("%d %d\n",i+1,J[i]+1);
	printf(";\n");
}
