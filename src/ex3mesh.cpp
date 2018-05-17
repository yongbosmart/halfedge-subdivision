//============================================================================
// Name        : ex3mesh.cpp
// Author      : yongbo
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================
#include<windows.h>//eclipse上一定要有这句话
#include "stdafx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>
#include <time.h>
#include <math.h>
#include<string>
#include<vector>
#include <iostream>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#define pi 3.141592653
#define _USE_MATH_DEFINES
using namespace std;



typedef struct Facep//面的结构
{
	int num=0;
	int edge=0;
	int order[3];//面会有3个点

	Facep(){

	}
	Facep(int a,int b,int c,int d,int e){
		num=a;
		edge=b;
		order[0]=c;
		order[1]=d;
		order[2]=e;
	}
}Facep;
//static char *filename="D:\\FC\\work\\ex3mesh\\icosahedron.off";
static char *filename="D:\\FC\\work\\ex3mesh\bird.off";
//static char *filename="D:\\FC\\work\\ex3mesh\\octahedron.off";


Facep fap[200000];
Facep fapnew[200000];
int i,j,k,n_node,n_face,n_edge;
static int window_height = 800;
static int window_width = 800;
float scale=1,spin=0;
float xx=0.0,yy=0.0,zz=1.0;
float wx=0.0,wy=0.0,wz=0.0;
float tx=0.0,ty=0.0;//鼠标位置
bool rotate=false,translate=false,scaled=false;
bool xsame=false,ysame=false,zsame=false;
int edgetmp=0;
struct Vertex;
struct face;
struct edge
   {
	   int svert=-1;//起始点
	   int dvert=-1;   //终点
       int pair=-1;   // 相反边，对边
       int facea=-1;   // 对应面
       int next=-1;   // 下一条边
       int mid=-1;
       edge(){}
//       	edge(Vertex *dv,Vertex *sv) {
//       		dvert = dv;
//       		svert = sv;
//       	}
       	edge(int a, int b, int c, int d) {
       	     svert = a;
       	     dvert=b;
//       	     pair=p;
       	     facea=c;
       	     next=d;
      }
};
edge edgel[200000];
edge edgenew[200000];
struct Vertex//点的结构
{
	float x=0,y=0,z=0;
	int edge=NULL;//起始点的边
};
Vertex ver[1000000];
struct face
    {

        edge* edge=NULL;  // one of the half-edges bordering the face
//        face(edge* ev) {
//             edge = ev;
//        }
        face(){

        }
    };
//struct vert
//{
//
//        float x=0.0;
//        float y=0.0;
//        float z=0.0;
//
//        edge* edge=NULL;  // one of the half-edges emantating from the vertex
//
//};


int readoff(const char* filename)
{
	FILE *fp;

	if(!(fp=fopen(filename,"r")))
	{
		fprintf(stderr,"load失败");
		return 0;
	}
    char buffer[1024];
	if(fgets(buffer,1023,fp))
	{
		if(!strstr(buffer,"OFF"))
			{
				printf("read失败");
				return 0;
		}

		if(fgets(buffer,1023,fp))
		{
			sscanf(buffer,"%d %d %d",&n_node,&n_face,&n_edge);//读取off中的点、面、边信息

			for(i=0;i<n_node;i++)//读入多少个点信息
			{
				fgets(buffer,1023,fp);
				sscanf(buffer,"%f%f%f",&ver[i].x,&ver[i].y,&ver[i].z);//点
				ver[i].x = ver[i].x * 0.1;
				ver[i].y = ver[i].y * 0.1;
				ver[i].z = ver[i].z * 0.1;
			}
			for(i=0;i<n_face;i++)//读入面，面有几个顶点，顶点1的索引、顶点2的索引、顶点3的索引。
			{
				fgets(buffer,1023,fp);
				sscanf(buffer,"%d%d%d%d",&fap[i].num,&fap[i].order[0],&fap[i].order[1],&fap[i].order[2]);

			}//面信息

			for(i=0;i<n_face;i++){//得到完整的边信息
				int first=edgetmp;
				edgel[edgetmp].facea=i;
				edgel[edgetmp].svert=fap[i].order[0];
				edgel[edgetmp].dvert=fap[i].order[1];
				ver[fap[i].order[0]].edge=edgetmp;//设置点的边
				fap[i].edge=edgetmp;
				int eee=edgetmp;
				edgetmp++;
				edgel[eee].next=edgetmp;

				edgel[edgetmp].facea=i;
				edgel[edgetmp].svert=fap[i].order[1];
				edgel[edgetmp].dvert=fap[i].order[2];
				ver[fap[i].order[1]].edge=edgetmp;
				fap[i].edge=edgetmp;
				eee=edgetmp;
				edgetmp++;
				edgel[eee].next=edgetmp;

				edgel[edgetmp].facea=i;
				edgel[edgetmp].svert=fap[i].order[2];
				edgel[edgetmp].dvert=fap[i].order[0];
				ver[fap[i].order[2]].edge=edgetmp;
				fap[i].edge=edgetmp;
				edgel[edgetmp].next=first;//三角形
				edgetmp++;
			}
			for( i=0;i<edgetmp;i++){
				bool loop=false;

				for(int j=i+1;j<edgetmp;j++){
					if(!loop){
						if(edgel[j].pair==-1){
							if(edgel[i].svert==edgel[j].dvert&&edgel[i].dvert==edgel[j].svert){
								edgel[i].pair=j;
								edgel[j].pair=i;
								loop=true;
							}
						}
					}
				}
			}
		}

    }
}

void change(){
	int flength=0;int elength=0;
	for(i=0;i<n_face;i++){//对于每一个面，增加新的面
		int index=fap[i].edge;//得到一个面的代表边，注意这个代表边在后面会迭代变化。
//		int tmpnode=n_node;//n_node增加，点增加。
		int vnew[3];//代表新增的三个点
		int ivnew=0;
		do {
		  if(edgel[edgel[index].pair].mid==-1){//即对边没有加新点，可以加新点
			  int v0,v1,v2,v3;
			  v0=edgel[index].svert;
			  v1=edgel[index].dvert;
			  v2=edgel[edgel[index].next].dvert;
			  v3=edgel[edgel[index].pair].dvert;

			  ver[n_node].x=(float)(0.125)*(ver[v2].x+ver[v3].x)+(0.375)*(ver[v0].x+ver[v1].x);
			  ver[n_node].y=(float)(0.125)*(ver[v2].y+ver[v3].y)+(0.375)*(ver[v0].y+ver[v1].y);
			  ver[n_node].z=(float)(0.125)*(ver[v2].z+ver[v3].z)+(0.375)*(ver[v0].z+ver[v1].z);

			  edgel[index].mid=n_node;//点正常新增
			  vnew[ivnew]=n_node;//新增点坐标
			  n_node++;
//			  新增了一个点
		  }else{
			  edgel[index].mid=edgel[edgel[index].pair].mid;
			  vnew[ivnew]=edgel[edgel[index].pair].mid;//对边加过点，就用对边加过的点
		  }
		  	  	  ivnew++;
//	生成新的点
		        // do something with edge
		        index =edgel[index].next;

		 } while (index != fap[i].edge);
		//得到新的边和新的面
		fapnew[flength].num=3;//肯定都是一样的
		fapnew[flength].order[0]=edgel[fap[i].edge].svert;
		fapnew[flength].order[1]=vnew[0];
		fapnew[flength].order[2]=vnew[2];
		fapnew[flength].edge=elength;
		int efirst=elength;//插新边，记住第一个边
		edgenew[elength].svert=edgel[fap[i].edge].svert;
		ver[edgel[fap[i].edge].svert].edge=elength;
		edgenew[elength].dvert=vnew[0];
		edgenew[elength].facea=flength;
		int tmppp=elength;
		edgenew[tmppp].next=++elength;//下一个
		edgenew[elength].svert=vnew[0];
		ver[vnew[0]].edge=elength;
		edgenew[elength].dvert=vnew[2];
		edgenew[elength].facea=flength;
		tmppp=elength;
		edgenew[tmppp].next=++elength;//下一个//下一个
		edgenew[elength].svert=vnew[2];
		ver[vnew[0]].edge=elength;
		edgenew[elength].dvert=edgel[fap[i].edge].svert;
		edgenew[elength].facea=flength;
		edgenew[elength].next=efirst;//第一个
		elength++;//下一个填取
		flength++;		//下一个面填取
		int va=edgel[fap[i].edge].dvert;
		fapnew[flength].num=3;
		fapnew[flength].order[0]=vnew[0];
		fapnew[flength].order[1]=va;
		fapnew[flength].order[2]=vnew[1];
		fapnew[flength].edge=elength;
		efirst=elength;//记录当前坐标
		edgenew[elength].svert=vnew[0];
		ver[vnew[0]].edge=elength;
		edgenew[elength].dvert=va;
		edgenew[elength].facea=flength;
		 tmppp=elength;
		edgenew[tmppp].next=++elength;//下一个//下一个
		edgenew[elength].svert=va;
		ver[va].edge=elength;
		edgenew[elength].dvert=vnew[1];
		edgenew[elength].facea=flength;
		tmppp=elength;
		edgenew[tmppp].next=++elength;//下一个//下一个
		edgenew[elength].svert=vnew[1];
		ver[vnew[1]].edge=elength;
		edgenew[elength].dvert=vnew[0];
		edgenew[elength].facea=flength;
		edgenew[elength].next=efirst;//第一个
		elength++;//下一个填取
		flength++;		//下一个面填取
		int vatemp=edgel[fap[i].edge].next;
		int vb=edgel[vatemp].dvert;
		fapnew[flength].num=3;
		fapnew[flength].order[0]=vnew[2];
		fapnew[flength].order[1]=vnew[1];
		fapnew[flength].order[2]=vb;
		fapnew[flength].edge=elength;
		efirst=elength;//记录当前坐标
		edgenew[elength].svert=vnew[2];
		ver[vnew[2]].edge=elength;
		edgenew[elength].dvert=vnew[1];
		edgenew[elength].facea=flength;
		tmppp=elength;
		edgenew[tmppp].next=++elength;//下一个//下一个
		edgenew[elength].svert=vnew[1];
		ver[vnew[1]].edge=elength;
		edgenew[elength].dvert=vb;
		edgenew[elength].facea=flength;
		tmppp=elength;
		edgenew[tmppp].next=++elength;//下一个//下一个
		edgenew[elength].svert=vb;
		ver[vb].edge=elength;
		edgenew[elength].dvert=vnew[2];
		edgenew[elength].facea=flength;
		edgenew[elength].next=efirst;//第一个
		elength++;//下一个填取
		flength++;		//下一个面填取
		fapnew[flength].num=3;
		fapnew[flength].order[0]=vnew[2];
		fapnew[flength].order[1]=vnew[0];
		fapnew[flength].order[2]=vnew[1];
		fapnew[flength].edge=elength;
		efirst=elength;//记录
		edgenew[elength].svert=vnew[2];
		ver[vnew[2]].edge=elength;
		edgenew[elength].dvert=vnew[0];
		edgenew[elength].facea=flength;
		edgenew[elength].pair=elength-8;
		edgenew[elength-8].pair=elength;
		tmppp=elength;
		edgenew[tmppp].next=++elength;//下一个//下一个
		edgenew[elength].svert=vnew[0];
		ver[vnew[0]].edge=elength;
		edgenew[elength].dvert=vnew[1];
		edgenew[elength].pair=elength-5;
		edgenew[elength-5].pair=elength;
		edgenew[elength].facea=flength;
		tmppp=elength;
		edgenew[tmppp].next=++elength;//下一个//下一个
		edgenew[elength].svert=vnew[1];
		ver[vnew[1]].edge=elength;
		edgenew[elength].dvert=vnew[2];
		edgenew[elength].pair=elength-5;
		edgenew[elength-5].pair=elength;
		edgenew[elength].facea=flength;
		edgenew[elength].next=efirst;//第一个
		elength++;//下一个填取
		flength++;		//下一个面填取
		cout<<elength<<" "<<flength<<" "<<"sad"<<n_node<<endl;

	}

	//加对边
	for( i=0;i<elength;i++){
		bool loop=false;
			for(int j=i+1;j<elength;j++){
				if(!loop){
					if(edgenew[j].pair==-1){
						if(edgenew[i].svert==edgenew[j].dvert&&edgenew[i].dvert==edgenew[j].svert){
							edgenew[i].pair=j;
							edgenew[j].pair=i;
							loop=true;
						}
					}
			  }
		}
	}

//	for(int k=0;k<flength;k++){
////		cout<<fapnew[k].edge;
//
//		int mmedge = fapnew[k].edge;
//		cout<<k<<":"<<mmedge<<endl;
//		do {
//
//	        // do something with edge
////			cout<<edgenew[mmedge].svert<<" "<<edgenew[mmedge].dvert<<" "<<edgenew[mmedge].next<<" "<<edgenew[mmedge].pair<<endl;
//			mmedge  = edgenew[mmedge].next;
//
//		} while (mmedge!= fapnew[k].edge);
//
//		cout<<endl;
//	}

	for(i=0;i<n_node;i++){
		 int counten=0;
//		i,当前点
		int vtmp=ver[i].edge;//点对应的边
//		int vfirstt=edgenew[ver[i].edge].dvert;//当前点第一个临近点
		int vedge = ver[i].edge; //当前点的边,遍历用
		int vertmp[5000];
//		vertmp[count]=edgenew[ver[i].edge].dvert;
		do {
//			cout<<counten<<"eee"<<endl;
		          // do something with edge, edge->pair or edge->face
		   vertmp[counten]=edgenew[vedge].dvert;//vertmp记录周围点的序号
		   vedge= edgenew[edgenew[vedge].pair].next;  //重点指向下一个周边点的边de坐标
		   counten++;//多少个点

		} while (vedge != vtmp);
//		cout<<counten<<"sdaaaaafq"<<endl;
		float xcount= (float)1/counten;
		float ycount= (float)(2*pi)/counten;
		float beta=xcount*(0.625-pow(0.375+0.25*cos(ycount),2.0));
//double beta=0;
		Vertex suml;
		for(int jj=0;jj<counten;jj++){//把每个点的坐标加在一起。
			suml.x+=ver[vertmp[jj]].x;
			suml.y+=ver[vertmp[jj]].y;
			suml.z+=ver[vertmp[jj]].z;
		}
		cout<<beta<<"seq"<<endl;
		suml.x=suml.x*beta;
		suml.y=suml.y*beta;
		suml.z=suml.z*beta;
//		cout<<ver[i].x<<" "<<ver[i].y<<" "<<ver[i].z<<endl;
		ver[i].x=ver[i].x*(1-counten*beta)+suml.x;
		ver[i].y=ver[i].y*(1-counten*beta)+suml.y;
		ver[i].z=ver[i].z*(1-counten*beta)+suml.z;
		cout<<ver[i].x<<" "<<ver[i].y<<" "<<ver[i].z<<endl;
	}

//坐标代换。
	for(i=0;i<elength;i++){
		edgel[i].dvert=edgenew[i].dvert;
		edgel[i].svert=edgenew[i].svert;
		edgel[i].facea=edgenew[i].facea;
		edgel[i].next=edgenew[i].next;
		edgel[i].pair=edgenew[i].pair;
		edgel[i].mid=-1;//边是新的，就没有加点了
		//以下所有的信息初始化
		edgenew[i].dvert=-1;
		edgenew[i].svert=-1;
		edgenew[i].facea=-1;
		edgenew[i].next=-1;
		edgenew[i].pair=-1;
		edgenew[i].mid=-1;
	}
	n_edge=elength;
	for(i=0;i<flength;i++){
		fap[i].num=3;
		fap[i].edge=fapnew[i].edge;
		fap[i].order[0]=fapnew[i].order[0];
		fap[i].order[1]=fapnew[i].order[1];
		fap[i].order[2]=fapnew[i].order[2];
		fapnew[i].order[0]=0;
		fapnew[i].order[1]=0;
		fapnew[i].order[2]=0;
		fapnew[i].edge=0;
	}
	n_face=flength;
	elength=0;
	flength=0;

}
bool changing=true;
void redraw(void)
{
	int a=clock();
	glClearColor(0.0, 0.0, 0.0, 0.0);
//由于模型和视图的变换都通过矩阵运算来实现，在进行变换前，应先设置当前操作的矩阵为“模型
//	视图矩阵”。设置的方法是以 GL_MODELVIEW 为参数调用 glMatrixMode 函数，
	glMatrixMode(GL_MODELVIEW);//通常，我们需要在进行变换前把当前矩阵设置为单位矩阵。这也只需要一行代码
	glLoadIdentity();

	glScalef(scale,scale,scale);//缩放函数,把当前矩阵和一个表示缩放物体的矩阵相乘。x,y,z 分别表示在该方向上的缩放比例。
//	cout<<"旋转点为：x-"<<xx<<";y-"<<yy<<";z-"<<zz<<endl;
	glRotatef(spin,xx,yy,zz);//旋转,物体将绕着(0,0,0)到(x,y,z)的直线以逆时针旋转，参数 angle 表示旋转的角度
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(5.0,5.0,5.0);
	glTranslatef(wx, wy, wz);//把当前矩阵和一个表示移动物体的矩阵相乘。三个参数分别表示了在三个坐标上的位移值。

	glBegin(GL_TRIANGLES);//基本图元
//	if(changing){
//	change();
//	change();
//	change();
//	change();
//	changing=false;
//	}
	for(i=0;i<n_face;i++)
	{

		int count=fap[i].order[0];//找第一个点索引
//		cout<<count<<" "<<ver[count].x<<" "<<ver[count].y<<" "<<ver[count].z<<"||";
		glVertex3f(ver[count].x,ver[count].y,ver[count].z);//对应点
		count=fap[i].order[1];//第二个点索引
//		cout<<count<<" "<<ver[count].x<<" "<<ver[count].y<<" "<<ver[count].z<<"||";
		glVertex3f(ver[count].x,ver[count].y,ver[count].z);
		count=fap[i].order[2];//第三个点索引
//		cout<<count<<" "<<ver[count].x<<" "<<ver[count].y<<" "<<ver[count].z<<"||";
		glVertex3f(ver[count].x,ver[count].y,ver[count].z);
//		cout<<endl;
	}
		cout<<"------------------------"<<endl;

	glEnd();
	glFlush();
	glutSwapBuffers();

}
void keyboard(unsigned char key, int x, int y)
{
	switch(key)
	{
	   case 'p':glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);glutPostRedisplay();break;//point
       case 'l':glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);glutPostRedisplay();break;//line
       case 'f':glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);glutPostRedisplay();break;//fill
       case 'e':scale*=1.1;glutPostRedisplay();break;
       case 'r':scale*=0.9;glutPostRedisplay();break;
       case 't':change();glutPostRedisplay();break;
       case 'z':
    	   if(!zsame)zz+=1.0;zsame=true;xsame=false;ysame=false;
    	   spin=spin+1;
    	   glutPostRedisplay();
    	   glFlush();
    	   break;
//       case 'd':if(!zz==1.0){zz=1.0;}spin=spin-1;glutPostRedisplay();glFlush();break;
//       case 'z':if(!xx==1.0){xx=1.0;yy=0.0;zz=0.0;}spin=spin+1;glutPostRedisplay();glFlush();break;
//       case 'x':if(!xx==1.0&yy==0.0&zz==0.0)){xx=1.0;yy=0.0;zz=0.0;}spin=spin-1;glutPostRedisplay();glFlush();break;
//       case 'c':if(!(xx==0.0&yy==1.0&zz==0.0)){xx=0.0;yy=1.0;zz=0.0;}spin=spin+1;glutPostRedisplay();glFlush();break;
//       case 'v':if(!(xx==0.0&yy==1.0&zz==0.0)){xx=0.0;yy=1.0;zz=0.0;}spin=spin-1;glutPostRedisplay();glFlush();break;
       case 'x':if(!zsame)zz+=1.0;zsame=true;xsame=false;ysame=false;spin=spin-1;glutPostRedisplay();glFlush();break;
       case 'c':if(!xsame)xx+=1.0;xsame=true;zsame=false;ysame=false;;spin=spin+1;glutPostRedisplay();glFlush();break;
       case 'v':if(!xsame)xx+=1.0;xsame=true;zsame=false;ysame=false;spin=spin-1;glutPostRedisplay();glFlush();break;
       case 'b':if(!ysame)yy+=1.0;ysame=true;zsame=false;xsame=false;spin=spin+1;glutPostRedisplay();glFlush();break;
       case 'n':if(!ysame)yy+=1.0;ysame=true;zsame=false;xsame=false;spin=spin-1;glutPostRedisplay();glFlush();break;
       case 'a':wx-=0.1;spin=spin+1;glutPostRedisplay();glFlush();break;
       case 'd':wx+=0.1;;spin=spin-1;glutPostRedisplay();glFlush();break;
       case 'w':wy+=0.1;spin=spin+1;glutPostRedisplay();glFlush();break;
       case 's':wy-=0.1;;spin=spin-1;glutPostRedisplay();glFlush();break;
	}
}
void init(int *argc, char **argv)
{
  // Open window
  glutInit(argc, argv);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(window_width, window_height);

  glutCreateWindow("OFF文件读取");

  // Initialize GLUT callback functions
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  //glutReshapeFunc(GLUTResize);
  glutDisplayFunc(redraw);
  glutKeyboardFunc(keyboard);
//  glutMouseFunc(mouse);
  glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
  //glutMotionFunc(motion);
  glutIdleFunc(0);


}

int main(int argc, char** argv)
{
	init(&argc,argv);

	readoff(filename);
	glutMainLoop();
	return 0;
}
