//
//  main.cpp
//  cornerdetection
//
//  Created by Zheng Shuangyue on 29/3/16.
//  Copyright (c) 2016 Zheng Shuangyue. All rights reserved.
//  

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <math.h>
#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;



double sigma=1.4; // smoothing the image, reducing the noise

//Sobel Matrices Horizontal
/*******      | -1 0 1|   *******/
/*******  Gx =| -2 0 2|   *******/
/*******      | -1 0 1|   *******/

int GX[3][3] = {{-1,0,1},{-2,0,2},{-1,0,1}};

//Sobel Matrices Vertical
/*******      |-1 -2 -1|   *******/
/*******  Gy =| 0  0  0|   *******/
/*******      | 1  2  1|   *******/

int GY[3][3] = {{-1,-2,-1},{0,0,0},{1,2,1}};


double g1, g2;


int main(int argc, const char *argv[]){
  
    Mat OriImg;
    FILE *fp = NULL;
    unsigned char *imagedata = (unsigned char *) malloc(5);
    unsigned char *buffer = NULL;
    //int framesize = IMAGE_WIDTH * IMAGE_HEIGHT;
    
    //Open raw Bayer image.
    fp = fopen("/Users/zhengshuangyue/Documents/cornerdectection/lamp.raw", "rb");
    
    fseek (fp , 0 , SEEK_END);
    long length = ftell (fp);
    rewind (fp);
    cout<<length<<endl;
    
    //Read first 5 bytes and store in imagedata.
    fread(imagedata, 1, 5, fp);
    
    const unsigned int col=(int)(imagedata[0]|(imagedata[1]<<8));
    cout<<col<<endl;
    const unsigned int row=(int)(imagedata[2]|(imagedata[3]<<8));
    cout<<row<<endl;
    
    int framesize = col*row;
    
    //Read rest of bytes and store in buffer.
    buffer = (unsigned char*) malloc (sizeof(char) * framesize);

    fread(buffer, sizeof(char), framesize, fp);
    
    //Create Opencv mat structure for image dimension. For 8 bit bayer, type should be CV_8UC1.
    OriImg.create(row, col, CV_8UC1);
    
    memcpy(OriImg.data, buffer, framesize);
    //cout<< OriImg<<endl;
    //imwrite("/Users/zhengshuangyue/Documents/cornerdectection/fruit.jpg", OriImg);

    free(imagedata);
    free(buffer);
    fclose(fp);


    
    Mat img = OriImg.clone();
    

    int mask_width = (int) (sigma * 5 + 0.5);
    
    cout<< mask_width <<endl;
    
    if(mask_width%2 == 0){
        mask_width = mask_width + 1;
    }
    
    cout<< mask_width <<endl;
    
    Mat gaussianMask(mask_width, mask_width, CV_8UC1, Scalar(0));
    
    
    unsigned char *ptr_mask =(unsigned char*)(gaussianMask.data);
    
    int value_of_mask = 0;
    
    double smallest = exp(-(pow((int)(mask_width/2),2)+pow((int)(mask_width/2),2))/(2*pow(sigma,2)));
    
    for(int u = 0; u < mask_width; u++){
        for(int v = 0; v < mask_width; v++){
            int m = u - (int)(mask_width/2);
            int n = v - (int)(mask_width/2);
            double temp1 = exp(-(pow(m,2)+pow(n,2))/(2*pow(sigma,2)))/smallest;
            int temp2 = (int)(temp1 + 0.5);
            ptr_mask[gaussianMask.step * u + v] =(unsigned char) temp2;
            value_of_mask = value_of_mask + ptr_mask[gaussianMask.step * u + v];
        }
    }
    
    cout << gaussianMask <<endl;
    cout << value_of_mask <<endl;
    
    const unsigned int width = img.cols;
    const unsigned int height = img.rows;
    
    Mat blurImg(height, width, CV_8UC1);
    
    blurImg.setTo(Scalar(0));
    
    
    unsigned char *ptr_blurImg = (unsigned char*)(blurImg.data);
    
    unsigned char *ptr_orgImg =(unsigned char*)(img.data);
    
    
    
    
    for(int h = (int) (mask_width/2); h < (height - (int) (mask_width/2)); h ++){
        for(int w = (int) (mask_width/2); w < (width-(int) (mask_width/2)); w ++){
            
            int pixels = 0;
            
            for(int u = - (int) (mask_width/2); u <= (int) (mask_width/2); u++){
                for(int v = - (int) (mask_width/2); v <= (int) (mask_width/2); v++){
            
                    pixels = pixels + (int)ptr_mask[gaussianMask.step * (u + (int)(mask_width/2)) + (v + (int)(mask_width/2))] * (int)ptr_orgImg[img.step * (h + u) + (w + v)];
                    
                    
                    //cout << (int)ptr_mask[gaussianMask.step * (u + (int)(mask_width/2)) + (v + (int)(mask_width/2))];
                    
                   // cout << " -> "<< (int)ptr_orgImg[img.step * (h + u) + (w + v)] <<" -> ";
    
                }
            }
            //cout << (int)ptr_orgImg[img.step * h  + w ] <<" -> ";
            int blur_pixel = pixels /(int) value_of_mask;
            
            //cout << blur_pixel <<endl;
            
            ptr_blurImg[blurImg.step * h + w] =(unsigned char)blur_pixel;
            //cout << "col" << w << "row" << h << " -> "<< (int)ptr_blurImg[blurImg.step * h + w]<< endl;
        }
    }
    

    
    

   // imwrite("/Users/zhengshuangyue/Documents/cornerdectection/gaussianBlur.jpg", blurImg);


    //   cout << blurImg << "Blur img" <<endl;
 
    //Edge detection using Sobel Algorithm
    
    Mat sobel_img(height, width, CV_8UC1);
    sobel_img.setTo(Scalar(0));
    
    Mat x_img(height, width, CV_8SC1);
    x_img.setTo(Scalar(0));
    
    Mat y_img(height, width, CV_8SC1);
    y_img.setTo(Scalar(0));
    
    //Mat theta_angle(height, width, CV_32FC1);
    //theta_angle.setTo(Scalar(0.0));
    
    int SUM;
    
    float theta_XY = 0;
    
    unsigned char *ptr = (unsigned char*) (blurImg.data);
    signed char *ptr_x = (signed char*) (x_img.data);
    signed char *ptr_y = (signed char*) (y_img.data);
 
    
    unsigned char *sobel_ptr = (unsigned char*) (sobel_img.data);
    //float *theta_ptr = (float*) (theta_angle.data);
    
    for(int y = 1; y < (width -1); y++){
        for(int x = 1; x < (height-1); x++){
            int sumX = 0;
            int sumY = 0;
            //cout<<ptr[blurImg.step * (x ) + (y)]<<endl;
            
            //Convolution for X
            for(int i = -1; i < 2; i++){
                for(int j = -1; j < 2; j++){
                    sumX = sumX + GX[j+1][i+1] * ptr[blurImg.step * (x + i) + (y + j)];
                }
            }
            
            //Convolution for Y
            for(int i = -1; i < 2; i++){
                for(int j = -1; j < 2; j++){
                    sumY = sumY + GY[j+1][i+1] * ptr[blurImg.step * (x + i) + (y + j)];
                }
            }
            
            //sumX = sumX/8;
            //sumY = sumY/8;

            ptr_x[x_img.step*x+y] =sumX;
            ptr_y[y_img.step*x+y] =sumY;
            //cout << sumX <<"x"<<endl;
            //cout << sumY<< "y" <<endl;
            
            //Edge strength
            SUM = (int)sqrt(pow((double)sumX,2) + pow((double)sumY,2)+0.5);


            
            if(SUM > 255) SUM = 255;
            
            sobel_ptr[sobel_img.step * x + y] = SUM; // save to sobel_img
            
        }
    }
    
    for(int h =0; h < (int) (mask_width); h++){
        for(int w = 0; w < width; w ++){
            sobel_ptr[sobel_img.step * h + w] = 0;
        }
    }
    for(int h = (height-(int) (mask_width)); h < height; h++){
        for(int w = 0; w < width; w ++){
            sobel_ptr[sobel_img.step * h + w] = 0;
        }
    }
    for(int w = (width-(int) (mask_width)); w < width; w++){
        for(int h = 0; h < height; h ++){
            sobel_ptr[sobel_img.step * h + w] = 0;
        }
    }
    for(int w = 0; w < (int) (mask_width); w++){
        for(int h = 0; h < height; h ++){
            sobel_ptr[sobel_img.step * h + w] = 0;
        }
    }
    
    
    //cout << sobel_img << "sobel_img" <<endl;
    
    //cout << theta_angle;
    imwrite("/Users/zhengshuangyue/Documents/cornerdectection/fat_img.jpg", sobel_img);
    imwrite("/Users/zhengshuangyue/Documents/cornerdectection/x.jpg", x_img);
    imwrite("/Users/zhengshuangyue/Documents/cornerdectection/y.jpg", y_img);
      
    //non-maximum suppression
      
    Mat nonmax_sup_img(height, width, CV_8UC1);
    nonmax_sup_img.setTo(Scalar(0));
      
    unsigned char *sup_ptr = (unsigned char*)(nonmax_sup_img.data);
    //cout << sobel_img<<endl;

    for(int y =1; y < width; y++){
        for(int x=1; x < height; x++){
            if((ptr_y[y_img.step*x + y] == 0) && (ptr_x[x_img.step*x+y] == 0) ){
                sobel_ptr[sobel_img.step*x+y] = 0;
                
            }else{
            
                theta_XY = atan2(ptr_y[y_img.step*x + y],ptr_x[x_img.step*x+y]) * 180/M_PI;
                //cout << theta_XY <<"x"<<(int)ptr_x[x_img.step*x+y]<<"y"<<(int)ptr_y[y_img.step*x + y]<< "atan2"<< endl;
                
                // Vertical gradient
                //////////        ////////////
                //////////g1 g  g2///////////
                //////////        ////////////
                if(((theta_XY >= 67.5 && theta_XY < 112.5 ))|| ((theta_XY < -67.5 && theta_XY >= -112.5))){

                    g1 = sobel_ptr[sobel_img.step*x + y-1];
                    g2 = sobel_ptr[sobel_img.step*x + y+1];
                 
                }
            
                //45 degree gradient
                //////////g1      ////////////
                //////////   g    ///////////
                //////////      g2////////////
                else if(((theta_XY >= 22.5 )&&(theta_XY< 67.5))||((theta_XY>=-157.5)&&(theta_XY< -112.5))){
             
                    g1 = sobel_ptr[sobel_img.step*(x-1) + y-1];
                    g2 = sobel_ptr[sobel_img.step*(x+1) + y+1];
                  
                }
                //135 degree gradient
                //////////      g1////////////
                //////////   g    ///////////
                //////////g2      ////////////
                else if(((theta_XY >= -67.5)&&(theta_XY < -22.5)) || ((theta_XY>=112.5)&&(theta_XY<157.5))){
             
                    g1 = sobel_ptr[sobel_img.step*(x-1) + y+1];
                    g2 = sobel_ptr[sobel_img.step*(x+1) + y-1];
                    
                }
            
                //horizontal gradient
                //////////   g1   ////////////
                //////////   g    ////////////
                //////////   g2   ////////////
                else{
           
                    g1 = sobel_ptr[sobel_img.step* (x-1) + y];
                    g2 = sobel_ptr[sobel_img.step* (x+1) + y];
                    
                }
              
                
                if((sobel_ptr[sobel_img.step*x+y] >= g1)&&(sobel_ptr[sobel_img.step*x+y] >= g2)){
                    
                    sup_ptr[nonmax_sup_img.step*x + y] = 128;
                
                }
            }
        }
    }
    
    int nHist[256];
    for(int i = 0; i < 256; i++){
        nHist[i] = 0;
    }
    
    int edgeNum=0;
    for(int y =1; y < width; y++){
        for(int x=1; x < height; x++){
            if(sobel_ptr[sobel_img.step*x +y] !=0){
                int pixel = sobel_ptr[sobel_img.step*x +y];
                nHist[pixel]++;
                edgeNum++;
            }
        }
    }
    
    int thresNum = (int)(0.8*edgeNum);
    
    edgeNum =0;
    int j = 1;
    while(edgeNum<thresNum){
        edgeNum += nHist[j];
        j++;
    }
    cout << j<<endl;
    
    int thresNo = j;
    for(int y =1; y < width; y++){
        for(int x=1; x < height; x++){
            if((sobel_ptr[sobel_img.step*x+y] > thresNo)&&(sup_ptr[nonmax_sup_img.step*x + y] == 128)){
                sup_ptr[nonmax_sup_img.step*x + y] = 255;
            }else{
                sup_ptr[nonmax_sup_img.step*x + y] = 0;
            }
        }
    }
    
    //imwrite("/Users/zhengshuangyue/Documents/cornerdectection/after_suppression_sobel.jpg", sobel_img);
    imwrite("/Users/zhengshuangyue/Documents/cornerdectection/thin_img.jpg", nonmax_sup_img);

    imshow("image", nonmax_sup_img);
    waitKey(0);
    
    
    return 0;
    
}
