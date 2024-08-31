#include <iostream>
#include "stdafx.h"
#include "common.h"
#include <opencv2/core/utils/logger.hpp>

wchar_t* projectPath;
using namespace std;

float computeSAD(Mat left, Mat right, int x, int y, int d, int level) {
    float sad = 0.0;

        for (int i = y; i < y + level; i++) {
            for (int j = x; j < x + level; j++) {
                Vec3b left_pixel = left.at<Vec3b>(y, x);
                Vec3b right_pixel = right.at<Vec3b>(y, x - d);


                sad = (float)abs(left_pixel[0] - right_pixel[0]) +
                    (float)abs(left_pixel[1] - right_pixel[1]) +
                    (float)abs(left_pixel[2] - right_pixel[2]);
            }
        }
    
    return sad;
}


void computeInitialHypotheses(const Mat left, const Mat right, vector<vector<vector<int>>>& hypotheses, int max_disparity) {

    int rows = left.rows;
    int cols = left.cols;
    hypotheses.resize(rows, vector<vector<int>>(cols, vector<int>(1, 0)));

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {

            vector<int> best_disparities(4, 0);
            vector<int> best_costs(4, INT_MAX);

            for (int d = 0; d < max_disparity; d++) {
                int xr = x - d;
                if (xr >= 0) {

                    int cost = (int)computeSAD(left, right, x, y, d,1);

                    for (int h = 0; h < 4; h++) {
                        if (cost < best_costs[h] && cost < 20) {
                            best_costs[h] = cost;
                            best_disparities[h] = d;
                            break;
                        }
                    }
                }
            }

            
         hypotheses[y][x][0] = best_disparities[0];
            
            
        }
    }
}



void computeHypotheses(Mat left, Mat right, vector<vector<vector<int>>>& hypotheses) {

    int rows = left.rows;
    int cols = left.cols;
  
    int level = 2;

    while (level <= 16) {
      
        for (int y = 0; y < rows; y += level) {
            for (int x = 0; x < cols; x += level) {

                int best_cost = INT_MAX;
                int best_disparity = 0;

                for (int yy = y ; yy < y + level/2 + 1 ; yy+=level / 2) {
                    for (int xx = x + level/2 - 1  ; xx < x + level; xx+=level/2) {

                        int cost;
                        if (xx - level / 2 + 1 - hypotheses[yy][xx][0] >= 0)
                        {

                        
                        cost = (int)computeSAD(left, right, xx - level / 2 + 1 , yy, hypotheses[yy][xx][0], level);
                       
                        if (cost < best_cost) {
                            best_cost = cost;
                            best_disparity = hypotheses[yy][xx][0];
                        }
                        }
                    }
                }
             
                
                
                for (int yy = y; yy < y + level; yy++) {
                    for (int xx = x; xx < x + level; xx++) {
                        
                        hypotheses[yy][xx][0] = best_disparity;
                    }
                }

            }
        }
        
        level *= 2;
    }
}

void refineDisparity(Mat left, Mat right, vector<vector<vector<float>>>& refined_disp, vector<vector<vector<int>>>& hypotheses) {
    int rows = left.rows;
    int cols = left.cols;
    for (int y = 0; y < rows; y += 16) {
        for (int x = 0; x < cols; x += 16) {

            if (x - hypotheses[y][x][0] - 1 >= 0) {
            int cost = computeSAD(left, right, x, y, hypotheses[y][x][0], 16);
            int cost_plus = computeSAD(left, right, x, y, hypotheses[y][x][0] + 1, 16);
            int cost_minus = computeSAD(left, right, x, y, hypotheses[y][x][0] - 1, 16);

            float numerator = cost_plus - cost_minus;
            float denominator = 2.0 * (cost_plus + cost_minus - 2.0 * cost);

            if (denominator == 0) {
                refined_disp[y][x][0] = (float)hypotheses[y][x][0];
            }
            else {
                float delta = numerator / denominator;

            for (int yy = y; yy < y + 16;yy++) {
                for (int xx = x; xx < x + 16; xx++) {
                    refined_disp[yy][xx][0] = abs((float)hypotheses[yy][xx][0] - delta);
                    cout << hypotheses[yy][xx][0]<< " "<< refined_disp[yy][xx][0] << endl;
                }
            }

            }

           
           }
        }
    }
}



void refineDisparityWithSlant(Mat left, Mat right, vector<vector<vector<float>>>& refined_disp, vector<vector<vector<int>>>& hypotheses) {
    int rows = left.rows;
    int cols = left.cols;
    int tile_size = 16;

    for (int y = 0; y < rows; y += tile_size) {
        for (int x = 0; x < cols; x += tile_size) {

            int center_x = x + tile_size / 2;
            int center_y = y + tile_size / 2;
            int d_center = hypotheses[center_y][center_x][0];

            // ...
        }
    }
}


void displayDisparityMap( vector<vector<vector<int>>>& hypotheses, Mat disparityMap) {
    int rows = hypotheses.size();
    int cols = hypotheses[0].size();

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            
            disparityMap.at<uchar>(y, x) = static_cast<uchar>(hypotheses[y][x][0]);
        }
    }

    normalize(disparityMap, disparityMap, 0, 255, NORM_MINMAX);
    resize(disparityMap, disparityMap, Size(), 0.6, 0.5);

    imshow("Disparity Map", disparityMap);


}


void createDepthMap(Mat disparityMap, Mat depthMap, double focal_length, double baseline) {

    depthMap = Mat(disparityMap.size(), CV_64FC1);

    for (int y = 0; y < disparityMap.rows; y++) {
        for (int x = 0; x < disparityMap.cols; x++) {
            uchar disparity = disparityMap.at<uchar>(y, x);

            if (disparity > 0) {
                double depth = (focal_length * baseline) / static_cast<double>(disparity);
                depthMap.at<double>(y, x) = depth;
            }
            else {
                depthMap.at<double>(y, x) = 0;
            }
        }
    }

    Mat normalizedDepthMap;
    normalize(depthMap, normalizedDepthMap, 0, 255, NORM_MINMAX);
    normalizedDepthMap.convertTo(normalizedDepthMap, CV_8UC1);

    Mat colorMap;
    applyColorMap(normalizedDepthMap, colorMap, COLORMAP_JET);
    resize(colorMap, colorMap, Size(), 0.6, 0.5);

    imshow("Depth Map", colorMap);

}

float computeScore(Mat disparityMap) {

    char fname[MAX_PATH];
    openFileDlg(fname);
    Mat groundTruth = imread(fname, IMREAD_GRAYSCALE);

    int score = 0;
    for (int y = 0; y < disparityMap.rows; y++) {
        for (int x = 0; x < disparityMap.cols; x++) {
            if (disparityMap.at<uchar>(y, x) == groundTruth.at<uchar>(y, x)) {
                score++;
            }
        }
    }

   
    return (float)score / (float(disparityMap.rows * disparityMap.cols));
}



void initialization(Mat left, Mat right, int maxDisparity, Mat disparityMap, Mat depthMap, double focal_length, double baseline) {

    vector<vector<vector<int>>> hypotheses;

    
    computeInitialHypotheses(left, right, hypotheses, maxDisparity);
    //computeHypotheses(left, right, hypotheses);

    vector<vector<vector<float>>> refined(left.rows, vector<vector<float>>(left.cols, vector<float>(1, 0.0f)));
    //refineDisparity(left, right, refined, hypotheses);

    //refineDisparityWithSlant(left, right, refined, hypotheses);


    displayDisparityMap(hypotheses, disparityMap);
    //createDepthMap(disparityMap, depthMap, focal_length, baseline);

    waitKey(0);
}



int main() {
    cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_FATAL);
    projectPath = _wgetcwd(0, 0);

    while (1) {
        int n;
        cout << "Choose a Dataset 1/2/3 (0 to exit):" << endl;
        cin >> n;

        if (n == 1) {
            char fname[MAX_PATH];
            char fname1[MAX_PATH];
            openFileDlg(fname);
            openFileDlg(fname1);
            Mat ir, il;
            ir = imread(fname, IMREAD_COLOR);
            il = imread(fname1, IMREAD_COLOR);

            double baseline = 177.288;
            double focal_length = 2270.780; //pixeli
         

            int maxDisparity = 64;

            Mat disparityMap = Mat::zeros(il.rows, il.cols, CV_8UC1);
            Mat depthMap = Mat::zeros(il.rows, il.cols, CV_8UC1);

            initialization(il, ir, maxDisparity, disparityMap, depthMap, focal_length, baseline);



        }
        else if (n == 2) {
            char fname[MAX_PATH];
            char fname1[MAX_PATH];
            openFileDlg(fname);
            openFileDlg(fname1);
            Mat ir, il;
            ir = imread(fname, IMREAD_COLOR);
            il = imread(fname1, IMREAD_COLOR);

            double baseline = 174.019;
            double focal_length = 2511.220; //pixeli

            int maxDisparity = 256;

            Mat disparityMap = Mat::zeros(il.rows, il.cols, CV_8UC1);
            Mat depthMap = Mat::zeros(il.rows, il.cols, CV_8UC1);

            initialization(il, ir, maxDisparity, disparityMap, depthMap, focal_length, baseline);

        }
        else if (n == 0) {
            break;
        }
    }
    return 0;
}
