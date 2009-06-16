#include <stdio.h>
#include <string.h>


#include "oobj3d/obj3d.h"
#include "tine.h"

/* Prototipes */

typedef struct t_obj
{
  int CenterX,CenterY,CenterZ;
  void * objp;
  char ID;
  char User;
  char XRem, YRem, ZRem;
  char orientation[18];
}t_obj;


extern t_obj * pointer;

/* Polygon test */

extern	void AddLineASM();
extern	void FillTablesASM();
extern	void ClearAndSwapFlag();
extern	void ComputeDivMod();


extern int miDiv(int a,int b);
extern int mimul16(int a, int b);



extern char CurrentPattern;

extern char CurrentPixelX;
extern char CurrentPixelY;
extern char OtherPixelX;
extern char OtherPixelY;
extern void DrawLine();

extern void pixel_address();
extern void put_pixel(int,int);
extern int addr;
extern char scan;


/* Variables to mantain for each space object */
char rotx[MAXSHIPS];
char roty[MAXSHIPS];
char rotz[MAXSHIPS];
char accel[MAXSHIPS];
char speed[MAXSHIPS];
char target[MAXSHIPS];
char flags[MAXSHIPS];
char ttl[MAXSHIPS];
char energy[MAXSHIPS];
char missiles[MAXSHIPS];
char ai_state[MAXSHIPS];
char vertexXLO[MAXSHIPS];
char vertexXHI[MAXSHIPS];
char vertexYLO[MAXSHIPS];
char vertexYHI[MAXSHIPS];



/* Needed in GetVec and GetPos */

extern signed char VX,VY,VZ;
extern int PosX,PosY,PosZ;

/* Used to specify final destination in fly_to_vector */
extern int VectX,VectY,VectZ;


/* For fired lasers */
extern char numlasers;
extern char laser_source[4];
extern char laser_target[4];



extern int SqRoot(int);
extern int abs(int);
extern void GetFrontVector();
extern void GetDownVector();
extern void GetSideVector();
extern void GetShipPos();
extern void SetCurrentObject(char);
extern int dot_product();
extern void fly_to_vector();
extern void norm_big();



main()
{
   
    char * p;
    /*unsigned int delay;
    int i,j,k;
    int res1,res2;*/
    
       
    init_tine();




    p=(char *)0x24e;  
    *p=5;
    p=(char *)0x24f;  
    *p=1; 

      
/*    for (i=0;i<=30000;i+=10000)
        for(j=0;j<=20000;j+=10000)
            for(k=0;k<=20000;k+=10000)
                {
                    VectX=i;VectY=j;VectZ=k;
                    printf("%d, %d, %d ->",VectX,VectY,VectZ); 
                    norm_big();
                    printf("%d, %d, %d\n",VectX,VectY,VectZ); 
                    getchar();
                    
                }
        
*/

    hires();

#ifndef FILLEDPOLYS
    curset(5,5);
    draw(0,123,1);
    draw(230,0,1);
    draw(0,-123,1);
    draw(-230,0,1);
#endif
    printf("A/Z Pitch, Q/W Roll, S/D Yaw\nO/L accel/deccel B missile 1 laser");


    InitTestCode();
    
    FirstFrame();

    RunDemo();

    //FirstFrame();
    //TestLoop();


}







#define DESTX 10000
#define DESTY 10000
#define DESTZ 3500

int GoX=DESTX;
int GoY=DESTY;
int GoZ=DESTZ;


void fly_to_pos()
{

    GetShipPos();

    //printf("Pos ");
    //printf("%d,%d,%d\n",PosX,PosY,PosZ);
   
    VectX=(GoX-PosX);
    VectY=(GoY-PosY);
    VectZ=(GoZ-PosZ);

    //printf("Vect %d\n",abs(VectX)|abs(VectY)|abs(VectZ));

    /*if ( (abs(VectX)|abs(VectY)|abs(VectZ)) < 500)
        fly_to_vector_old();
    else fly_to_vector();*/
 
    if ( (abs(VectX)|abs(VectY)|abs(VectZ)) > 300)
        fly_to_vector();
    else
    {
    printf("*");
        GoX=-GoX;GoY=-GoY;//GoZ=-GoZ;
    }
    //fly_to_vector();

   // printf("rotx=%d, roty=%d, rotz=%d\n",(int)rotx[curr_ship],(int)roty[curr_ship], (int)rotz[curr_ship]);
   /*printf("Speed 1 = %d\n",speed[1]);
   printf("Speed 2 = %d\n",speed[2]);
   printf("Speed 3 = %d\n",speed[3]);
   printf("Speed 4 = %d\n",speed[4]);*/

}


#ifdef 0
void MoveOthers()
{
   

    SetCurrentObject(2);
    //fly_to_pos();

    Tactics();
    
    MoveShips();

    /*
    SetCurrentObject(2);
    GetShipPos();
    printf("Pos ");
    printf("%d,%d,%d\n",PosX,PosY,PosZ);
    if ((abs(PosX)<70 && abs(PosY)<70 && abs(PosZ-1300)<70))
    {

        target[3]=0; speed[3]=0; accel[3]=0;
    }*/


    //printf("rx %d, ry %d, rz %d, s %d, a %d\n", rotx[6],roty[6],rotz[6],speed[6],accel[6]);
 

}


void Lasers()
{
    int i;

   if (! numlasers) cls();

    for (i=0; i<numlasers;i++)
        printf("%d fires at %d!\n",laser_source[i],laser_target[i]);

    numlasers=0;

}
#endif


/*
void print()
{
  //printf("POS: %d,%d,%d\n",*(int *)pointer,*((int *)(pointer)+2),*((int *)(pointer)+4));
    printf("POS: %d,%d,%d\n",pointer->CenterX,pointer->CenterY,pointer->CenterZ);

}
*/

extern char ID;
/*int * pop1=(int *)0x66;
int * pop2=(int *)0x68;
*/
void printHit()
{
    
    //props();
    //printf("  HIT %d \n",ID);
    explode();
    

}

/*
void props()
{
    printf("op1=%d, op2=%d",*pop1,*pop2);

}
*/

/*
extern char num_collisions;
extern char collision_list[4];

void prcolls()
{
    int i;

    if (num_collisions)
     printf("Collision with ");
    for (i=0;i<num_collisions;i++)
    {
        printf("%d ",collision_list[i]);
    }
    if (num_collisions)
     printf("\n");
      


}*/

void DrawLaser()
{
    curset(10,128,1);
    draw(110,-64,1);    

    curset(230,128,1);
    draw(-110,-64,1);    

}

#ifdef 0
void prdbug()
{

    int i;

    //for (i=4;i<=5;i++)

    //for (i=0; i<numlasers;i++)
    if (1/*numlasers*/)
    {
        i=0;
        printf("%d,%d to %d,%d!\n",vertexXLO[laser_source[i]]+vertexXHI[laser_source[i]]*256,
                                   vertexYLO[laser_source[i]]+vertexYHI[laser_source[i]]*256,
                                   vertexXLO[laser_target[i]]+vertexXHI[laser_target[i]]*256,
                                   vertexYLO[laser_target[i]]+vertexYHI[laser_target[i]]*256);
    }
    
     i=5;
     printf("%d: s %d, a %d, ai %d\n",i,speed[i],accel[i],ai_state[i]);


}
#endif
