// MLHW7.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <iostream>
#include<fstream>
#include<math.h>
#include<time.h>


using namespace std;

void BubbleSort(double list[], int size);

struct TreeNode{
	TreeNode *leftnode;
	TreeNode *rightnode;
	double branch[3];
	int nodenumber;
	int content; // 1 or -1: value, 2: branch 
};

TreeNode *FindNode (double data[][3], double datasize, int number);
int TreeError(double data[], TreeNode *tree);

int _tmain(int argc, _TCHAR* argv[])
{
	char theline [100] ;
	
	double data[100][3];	// train data with size 100
	double test[1000][3];	// test data with size 1000
	double baggingdata[100][3];
	TreeNode *tree[30000];
	for(int i=0; i<30000; i++){
		tree[i] = new TreeNode;
	}

	//read file
	fstream fin;
	fin.open("train.txt",ios::in);

	//分割字串存入二維陣列
	char *token = NULL;
	char *next_token = NULL;
	char seps[]   = " ,\t\n";
	int a=0;
	while(fin.getline(theline,sizeof(theline),'\n')){
		int b = 0;
		token = strtok_s( theline, seps, &next_token);
		while (token != NULL){
			data[a][b] = atof(token);
            token = strtok_s( NULL, seps, &next_token);
			b++;
		}
		a++;
	}
	fin.close();

	fin.open("test.txt",ios::in);
	char *token2 = NULL;
	char *next_token2 = NULL;
	a=0;
	while(fin.getline(theline,sizeof(theline),'\n')){
		int b = 0;
		token2 = strtok_s( theline, seps, &next_token2);
		while (token2 != NULL){
			test[a][b] = atof(token2);
            token2 = strtok_s( NULL, seps, &next_token2);
			b++;
		}
		a++;
	}
	fin.close();


	//open file
	char filename[]="Q20eout.txt";
	fstream fein;
	fein.open(filename, ios::out);//開啟檔案
	if(!fein){//如果開啟檔案失敗，fp為0；成功，fp為非0
        cout<<"Fail to open file: "<<filename<<endl;
    }

	// random choice data to baggingdata
	srand(time(NULL));

	// set 30000 trees first
	for(int t=0; t<30000; t++){
		if(t%300==0){
			cout<<t/300<<endl;
		}
		// get baggingdata
		for(int i=0; i<100; i++){
			int index = rand() % 100;
			for(int j=0; j<3; j++){
				baggingdata[i][j] = data[index][j];
			}
		}
		tree[t] = FindNode(baggingdata,100,1);
	}	// end t
	cout<<"end setting"<<endl;
	
	double error_out[30000];
	for(int i=0; i<30000; i++){
		error_out[i] = 0;
	}

	// set random forest
	int gt[1000];
	for(int i=0; i<1000; i++){
		gt[i] = 0;
	}
	for(int t=0; t<30000; t++){
		if(t%300==0){
			cout<<t/300<<endl;
		}
		
		int forest_output[1000];
		for(int i=0; i<1000; i++){
			forest_output[i] = TreeError(test[i],tree[t]); // use origin data to get g(t)
			gt[i] = gt[i] + forest_output[i];
			if(gt[i]<0){
				forest_output[i] = -1;
			}else{
				forest_output[i] = 1;
			}
		}
		//calculate eout
		double error_per_t = 0;
		for(int i=0; i<1000; i++){
			if(forest_output[i]!=test[i][2]){
				error_per_t++;
			}
		}
		error_out[t] = error_per_t/(double)1000;
		fein<<error_out[t]<<endl;
	}
	cout<<error_out[29999]<<endl;
	system("pause");
	return 0;
}

void BubbleSort(double list[],int size){
	for(int i=0; i<size; i++){
		for(int j=0; j<size-i-1; j++){
			if(list[j]>list[j+1]){
				double temp = list[j];
				list[j] = list[j+1];
				list[j+1] = temp;
			}
		}
	}
}


int TreeError(double data[], TreeNode *tree){
	if(tree->content == 2){
		// internal node
		int xi = tree->branch[0];
		double threshold = tree->branch[1];
		if(data[xi]<threshold){
			TreeError(data,tree->leftnode);
		}else{
			TreeError(data,tree->rightnode);
		}
	}else{
		// leaf
		return tree->content;
	}
}


TreeNode *FindNode (double data[][3], double datasize, int number){
	TreeNode *tree = new TreeNode;
	tree->nodenumber = number;
	// terminal
	int terminal = 1;	// 0:end 1:continue
	if(number>1){
		terminal = 0;
	}
	
	if(terminal==0){
		int rightcount = 0;
		int wrongcount = 0;
		for(int i=0; i<datasize; i++){
			if(data[i][2]==1){
				rightcount++;
			}else{
				wrongcount++;
			}
		}
		if(rightcount>wrongcount){
			tree->content = 1;
		}else{
			tree->content = -1;
		}
		tree->leftnode = NULL;
		tree->rightnode = NULL;
		for(int i=0; i<3; i++){
			tree->branch[i] = 0;
		}
	}else{
		tree->content = 2;
		double x[2][100] ;
		for(int i=0; i<2; i++){
			for(int j=0; j<datasize; j++){
				x[i][j] = data[j][i];
			}
			for(int j=datasize; j<100; j++){
				x[i][j] = 10000;			//sort to the end if datasize<100
			}
		}

		double best[3]={0,0,10000};  // 0:xi  1:theda  2:gini index

		// learn branching criteria
		for(int xi=0; xi<2; xi++){
			BubbleSort(x[xi],datasize);	// first, sort
			for(int i=0; i<datasize-1; i++){
				// set theda
				double median = (double)(x[xi][i]+x[xi][i+1])/(double)2;

				// calculate impurity of D1, D2
				double D1size = 0;
				double D2size = 0;
				double D1right = 0;
				double D1wrong = 0;
				double D1impurity = 0;
				double D2right = 0;
				double D2wrong = 0;
				double D2impurity = 0;
				for(int j=0; j<datasize; j++){
					if(data[j][xi]<median){
						D1size++;
						if(data[j][2]==1){
							D1right++;
						}else{
							D1wrong++;
						}
					}else{
						D2size++;
						if(data[j][2]==1){
							D2right++;
						}else{
							D2wrong++;
						}
					}
				}	// end j
				
				D1right = D1right/D1size;
				D1wrong = D1wrong/D1size;
				D2right = D2right/D2size;
				D2wrong = D2wrong/D2size;
				D1impurity = 1 - (D1right*D1right)- (D1wrong*D1wrong);
				D2impurity = 1 - (D2right*D2right) - (D2wrong*D2wrong);

				// calculate b(x)
				double bx = D1size*D1impurity + D2size*D2impurity;
				if(bx<best[2]){
					best[0] = xi;
					best[1] = median;
					best[2] = bx;
				}
			}
		}	// end learn branching criteria, save the criteria in best

		// save the branch
		for(int i=0; i<3; i++){
			tree->branch[i] = best[i];
		}

		// split data to two parts
		double tree1[100][3];
		double tree2[100][3];
		// initial
		for(int i=0; i<100; i++){
			for(int j=0; j<3; j++){
				tree1[i][j] = 0;
				tree2[i][j] = 0;
			}
		}
		double tree1index = 0;
		double tree2index = 0;
		for(int i=0; i<datasize; i++){
			int xi = best[0];
			if(data[i][xi]<best[1]){
				int index = tree1index;
				for(int j=0; j<3; j++){
					tree1[index][j] = data[i][j];
				}
				tree1index++;
			}else{
				int index = tree2index;
				for(int j=0; j<3; j++){
					tree2[index][j] = data[i][j];
				}
				tree2index++;
			}
		}
		// build subtree
		
		if(tree1index==0){

		}else{
			int numberleft = number * 2 ;
			tree->leftnode = FindNode(tree1, tree1index, numberleft);
		}
		if(tree2index==0){

		}else{
			int numberright = number * 2 + 1;
			tree->rightnode = FindNode(tree2, tree2index, numberright);
		}
		
	}
	return tree;
}