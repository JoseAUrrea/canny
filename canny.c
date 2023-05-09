/*
TO RUN:
gcc canny.c
./a.out garb34.pgm p1.pgm p2.pgm p3_4.pgm
*/

#include <stdio.h>                 
#include <math.h>
#include <stdlib.h>
#define  PICSIZE 256
#define  MAXMASK 100

         int    pic[PICSIZE][PICSIZE];
         int    peaks[PICSIZE][PICSIZE];
         double xmask[MAXMASK][MAXMASK], ymask[MAXMASK][MAXMASK];
         double xconv[PICSIZE][PICSIZE], yconv[PICSIZE][PICSIZE];
         double mag[PICSIZE][PICSIZE];
         int    hg[PICSIZE];
         int    complete[PICSIZE][PICSIZE];

int main(int argc,char **argv)
{
        int     i,j,p,q,s,t,mr,centx,centy;
        double  xsum, ysum, sig, slope, maxival, minival, maxval, hi, lo, cut, area, percent;
        double  xmaskval, ymaskval;
        FILE    *out1, *out2, *out3, *input, *fopen();
        char    *foobar;

        argc--; argv++;
        foobar = *argv;
        input=fopen(foobar,"rb"); //garb34.pgm

        argc--; argv++;
        foobar = *argv;
        out1=fopen(foobar,"wb"); //pt1
        
        argc--; argv++;
        foobar = *argv;
        out2=fopen(foobar,"wb"); //pt2
        
        argc--; argv++;
        foobar = *argv;
        out3=fopen(foobar,"wb"); //pt3&4
        
        argc--; argv++;
        foobar = *argv;
        sig = atof(foobar); //use 1
        
        argc--; argv++;
        foobar = *argv;
        percent = atof(foobar); //use .05
        

        mr = (int)(sig * 3);
        centx = (MAXMASK / 2);
        centy = (MAXMASK / 2);

        for (i=0;i<PICSIZE;i++)
        { for (j=0;j<PICSIZE;j++)
                {
                  pic[i][j]  =  getc (input);
                }
        }
        
        /*PART ONE*/

        //DERIVATIVES
        for (p=-mr;p<=mr;p++)
        {  for (q=-mr;q<=mr;q++)
           {
              xmaskval = (q * exp(-1 * ((p * p) + (q * q)) / (2 * (sig * sig))));
              ymaskval = (p * exp(-1 * ((p * p) + (q * q)) / (2 * (sig * sig))));
           
              xmask[p+centy][q+centx] = xmaskval;
              ymask[p+centy][q+centx] = ymaskval;
           }
        }

        //CONVOLUTIONS
        for (i=mr;i<=255-mr;i++)
        { for (j=mr;j<=255-mr;j++)
          {
             xsum = 0;
             ysum = 0;
             for (p=-mr;p<=mr;p++)
             {
                for (q=-mr;q<=mr;q++)
                {
                   xsum += pic[i+p][j+q] * xmask[p+centy][q+centx]; 
                   ysum += pic[i+p][j+q] * ymask[p+centy][q+centx];
                }
             }
             //xoutpic1[i][j] = xsum;
             //youtpic1[i][j] = ysum;
             
             xconv[i][j] = xsum;
             yconv[i][j] = ysum;
          }
        }

        //MAGNITUDE
        maxival = 0;
        for(i=mr; i<PICSIZE-mr;i++)
        {
            for(j=mr;j<PICSIZE-mr; j++)
            {
                mag[i][j] = sqrt((double)((xconv[i][j] * xconv[i][j]) + (yconv[i][j] * yconv[i][j])));

                if(mag[i][j] > maxival)
                {
                    maxival = mag[i][j];
                }
            }
        }

        //OUTPUT PT 1
        fprintf(out1, "P5\n%d %d\n255\n", PICSIZE, PICSIZE);
        for(i=0; i<PICSIZE; i++)
        {
            for(j=0; j<PICSIZE; j++)
            {
                mag[i][j] = (mag[i][j]/maxival)*255;
                fprintf(out1, "%c", (char)(int)mag[i][j]);
            }
        }


         /*PART 2*/

         //PEAKS
         for(i=mr; i<PICSIZE-mr; i++) //double for loop to scan
         {
            for(j=mr; j<PICSIZE-mr; j++)
            {
                if((xconv[i][j]) == 0.0) //avoids dividing by 0
                    xconv[i][j] = .00001;
                
                slope = yconv[i][j]/xconv[i][j]; //tandir = convY/convX

                if((slope <= .4142) && (slope > -.4142)) //horizonal cone
                {
                    if((mag[i][j] > mag[i][j-1]) && (mag[i][j] > mag[i][j+1]))
                    {
                        peaks[i][j] = 255;
                    }
                }
                else if((slope <= 2.4142) && (slope > .4142)) //cone running south left to north east
                {
                    if((mag[i][j] > mag[i-1][j-1]) && (mag[i][j] > mag[i+1][j+1]))
                    {
                        peaks[i][j] = 255;
                    }
                }
                else if((slope <= -.4142) && (slope > -2.4142)) //cone running south east to north west
                {
                    if(mag[i][j] > mag[i+1][j-1] && (mag[i][j] > mag[i-1][j+1]))
                    {
                        peaks[i][j] = 255;
                    }
                }
                else //vertical cone
                {
                    if((mag[i][j] > mag[i-1][j]) && (mag[i][j] > mag[i+1][j]))
                    {
                        peaks[i][j] = 255;
                    }
                }

            }
         }

         //OUTPUT PT 2
        fprintf(out2, "P5\n%d %d\n255\n", PICSIZE, PICSIZE);
        for(i=0; i<PICSIZE; i++)
        {
            for(j=0; j<PICSIZE; j++)
            {
                fprintf(out2, "%c", (char)((int)peaks[i][j]));
            }
        }


        /*PART 3*/

        //USING THRESHOLDS
        for(i=0; i<PICSIZE; i++)
        {
            for(j=0; j<PICSIZE; j++)
            {
                hg[(int)mag[i][j]]++;
            }
        }

        cut = percent * PICSIZE * PICSIZE;
        area = 0;
        for(i=PICSIZE-1; i>=0; i--)
        {
            area += hg[i];
            if(area > cut)
                break;
        }

        hi = i;
        lo = .35 * hi;

        //APPLYING THE THRESHOLDS
        for(i=0; i<PICSIZE; i++)
        {
            for(j=0; j<PICSIZE; j++)
            {
                complete[i][j]=0;
                if(peaks[i][j] == 255)
                {
                    if(mag[i][j] > hi)
                    {
                        peaks[i][j]=0;
                        complete[i][j]=255;
                    }
                    else if(mag[i][j] < lo)
                    {
                        peaks[i][j]=0;
                        complete[i][j]=0;
                    }
                }
                
            }
        }

        //MORETODO LOOP
        int flag = 1;
        while(flag==1)
        {
            flag=0;
            for(i=0; i<PICSIZE; i++)
            {
                for(j=0; j<PICSIZE; j++)
                {
                    if(peaks[i][j] == 255)
                    {
                        for(p=-1; p <= 1; p++)
                        {
                            for(q=-1; q <= 1; q++)
                            {
                                if(complete[i+p][j+q] == 255)
                                {
                                    complete[i][j] = 255;
                                    peaks[i][j] = 0;
                                    flag = 1;
                                }
                            }
                        }
                    }
                }
            }
        }

        //OUTPUT PT 3/4
        fprintf(out3, "P5\n%d %d\n255\n", PICSIZE, PICSIZE);

        for(i=0; i<PICSIZE; i++)
        {
            for(j=0; j<PICSIZE; j++)
            {
                fprintf(out3, "%c", (char)((int)(complete[i][j])));
            }
        }
}

