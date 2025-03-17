/* INCLUDES FOR THIS PROJECT */
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <cmath>
#include <limits>
#include <map>
#include <opencv2/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d.hpp>
#include <opencv2/xfeatures2d.hpp>
#include <opencv2/xfeatures2d/nonfree.hpp>

#include "dataStructures.h"
#include "matching2D.hpp"
#include "ringBuffer.cpp"

using namespace std;

/* MAIN PROGRAM */
int main(int argc, const char *argv[])
{
    // #####################################
    // ### VARIABLES AND DATA STRUCTURES ###
    // #####################################

    // Detector Choice:
    string detectorType = "SHITOMASI";      // -> HARRIS, FAST, BRISK, ORB, AKAZE, SIFT
    
    // Descriptor Choice:
    string descriptorType = "BRIEF";    // -> BRIEF, ORB, FREAK, AKAZE, SIFT

    // FLAGS
    bool flag_all_combinations = true; // to process all above Detector/Descriptor combinations

    // Matching Choice:
    string matcherType = "MAT_FLANN";        // -> MAT_BF, MAT_FLANN
    string matchSelectorType = "SEL_KNN";       // -> SEL_NN, SEL_KNN
    

    // data location
    string dataPath = "../";

    // camera
    string imgBasePath = dataPath + "images/";
    string imgPrefix = "KITTI/2011_09_26/image_00/data/000000"; // left camera, color
    string imgFileType = ".png";
    int imgStartIndex = 0; // first file index to load (assumes Lidar and camera names have identical naming convention)
    int imgEndIndex = 9;   // last file index to load
    int imgFillWidth = 4;  // no. of digits which make up the file index (e.g. img-0001.png)

    // Visualize matching results
    bool bVis = false;            // visualize results

    // reduce search to proceeding vehicle box
    bool bFocusOnVehicle = true; // focus only on the proceeding vehicle
    cv::Rect vehicleRect(535, 180, 180, 150); // fix pixle locations

    // ############
    // ### CODE ###
    // ############

    vector<string> selectedDetectorType;
    vector<string> selectedDescriptorType;

    if (flag_all_combinations)
    {
        selectedDetectorType = {"SHITOMASI", "HARRIS", "FAST", "BRISK", "ORB", "AKAZE", "SIFT"}; //"SHITOMASI", "HARRIS", "FAST", "BRISK", "ORB", "AKAZE", "SIFT"
        selectedDescriptorType = {"BRIEF", "BRISK", "FREAK", "ORB", "AKAZE", "SIFT"}; // "BRIEF", "BRISK", "FREAK", "ORB", "AKAZE", "SIFT"
    } else {
        selectedDetectorType = {detectorType};
        selectedDescriptorType = {descriptorType};
    }

    /* MAIN LOOP OVER ALL Detectors */

    for (const std::string& dectType : selectedDetectorType) 
        {
            /* MAIN LOOP OVER ALL Descriptors */
        for (const std::string& descType : selectedDescriptorType)
            {
                // Catch invalid combinations 
                if ((descType.compare("AKAZE") == 0 && dectType.compare("AKAZE") != 0)  ||
                    (descType.compare("ORB") == 0 && dectType.compare("SIFT") == 0 ))
                    {
                        // AKAZE descriptor extractor works only with key-points detected with KAZE/AKAZE detectors
                        // ORB descriptor extractor does not work with the SIFT detetor
                        continue;
                    }
                
                // data buffer
                int dataBufferSize = 2;       // no. of images which are held in memory (ring buffer) at the same time
                vector<DataFrame> dataBuffer; // list of data frames which are held in memory at the same time
    
                /* MAIN LOOP OVER ALL IMAGES */

                for (size_t imgIndex = 0; imgIndex <= imgEndIndex - imgStartIndex; imgIndex++)
                    {
                        /* LOAD IMAGE INTO BUFFER */
                        // cout << "Start processing all requested combinations for image " << imgIndex << endl;

                        // assemble filenames for current index
                        ostringstream imgNumber;
                        imgNumber << setfill('0') << setw(imgFillWidth) << imgStartIndex + imgIndex;
                        string imgFullFilename = imgBasePath + imgPrefix + imgNumber.str() + imgFileType;

                        // load image from file and convert to grayscale
                        cv::Mat img, imgGray;
                        img = cv::imread(imgFullFilename);
                        cv::cvtColor(img, imgGray, cv::COLOR_BGR2GRAY);

                        //// STUDENT ASSIGNMENT
                        //// TASK MP.1 -> replace the following code with ring buffer called dataBuffer of size dataBufferSize
                        //ImageRingBuffer dataBuffer(dataBufferSize);

                        // push image into data frame buffer
                        DataFrame frame;
                        frame.cameraImg = imgGray;
                        dataBuffer.push_back(frame);

                        // limit data frame buffer size by removing oldest frame
                        if (dataBuffer.size() > dataBufferSize) {
                            dataBuffer.erase(dataBuffer.begin());
                        }

                        //// EOF STUDENT ASSIGNMENT
                        // cout << "#1 : LOAD IMAGE INTO BUFFER done" << endl;
                        

                        
                            /* DETECT IMAGE KEYPOINTS */

                            // cout << "Start processing dectetors: " << dectType << "." << endl;

                            // Measure time for detectors
                            double detectorTime = (double)cv::getTickCount();

                            // extract 2D keypoints from current image
                            vector<cv::KeyPoint> keypoints; // create empty feature list for current image
                            
                            //// STUDENT ASSIGNMENT
                            //// TASK MP.2 -> add the following keypoint detectors in file matching2D.cpp and enable string-based selection based on detectorType
                            if (dectType.compare("SHITOMASI") == 0)
                            {
                                detKeypointsShiTomasi(keypoints, imgGray, false);
                            }
                            else if (dectType.compare("HARRIS") == 0) {
                                detKeypointsHarris(keypoints, imgGray, false);
                            }
                            else 
                            {
                                detKeypointsModern(keypoints, imgGray, dectType, false);
                            }
                            
                            detectorTime = ((double)cv::getTickCount() - detectorTime) / cv::getTickFrequency();

                            // cout << "Detector processing time: " << detectorTime << " []" << endl;

                            //// EOF STUDENT ASSIGNMENT

                            //// STUDENT ASSIGNMENT
                            //// TASK MP.3 -> only keep keypoints on the preceding vehicle
                            // only keep keypoints on the preceding vehicle
                            
                            if (bFocusOnVehicle)
                            {
                                // temp vector to write out the keypoints of interest
                                vector<cv::KeyPoint> framedKeypoints;

                                for (auto kp : keypoints) {
                                    if (vehicleRect.contains(kp.pt)) framedKeypoints.push_back(kp);
                                }
                                // reframed keypoints
                                keypoints = framedKeypoints;
                            }
                            //// EOF STUDENT ASSIGNMENT

                            // optional : limit number of keypoints (helpful for debugging and learning)
                            bool bLimitKpts = false;
                            if (bLimitKpts)
                            {
                                int maxKeypoints = 50;

                                if (dectType.compare("SHITOMASI") == 0)
                                { // there is no response info, so keep the first 50 as they are sorted in descending quality order
                                    keypoints.erase(keypoints.begin() + maxKeypoints, keypoints.end());
                                }
                                cv::KeyPointsFilter::retainBest(keypoints, maxKeypoints);
                                cout << " NOTE: Keypoints have been limited!" << endl;
                            }

                            // push keypoints and descriptor for current frame to end of data buffer
                            (dataBuffer.end() - 1)->keypoints = keypoints;

                            //cout << "#2 : DETECT KEYPOINTS done." << endl;

                            /* EXTRACT KEYPOINT DESCRIPTORS */
                            
                                //// STUDENT ASSIGNMENT
                                // cout << "Start processing combination: " << dectType << "/" << descType << "." << endl;
                            
                                //// TASK MP.4 -> add the following descriptors in file matching2D.cpp and enable string-based selection based on descriptorType
                                cv::Mat descriptors;
                                descKeypoints((dataBuffer.end() - 1)->keypoints, (dataBuffer.end() - 1)->cameraImg, descriptors, descType);
                                //// EOF STUDENT ASSIGNMENT

                                // push descriptors for current frame to end of data buffer
                                (dataBuffer.end() - 1)->descriptors = descriptors;

                                // cout << "#3 : EXTRACT DESCRIPTORS done." << endl;

                                if (dataBuffer.size() > 1) // wait until at least two images have been processed
                                {
                                    /* MATCH KEYPOINT DESCRIPTORS */
                                    // cout << "Starting matching process with " << matchDescriptorType << "and" << matchSelectorType << " ." << endl;
                                    
                                    double descTime = (double)cv::getTickCount();

                                    // Catch SIFT needs gradient based and all others are going with binary
                                    string matchDescriptorType;
                                    if (descType.compare("SIFT")==0)
                                    {
                                        matchDescriptorType = "DES_HOG"; // -> DES_BINARY, DES_HOG
                                    } else {
                                        matchDescriptorType = "DES_BINARY";
                                    }
                                    
                                    vector<cv::DMatch> matches;

                                    //// STUDENT ASSIGNMENT
                                    //// TASK MP.5 -> add FLANN matching in file matching2D.cpp
                                    //// TASK MP.6 -> add KNN match selection and perform descriptor distance ratio filtering with t=0.8 in file matching2D.cpp

                                    matchDescriptors((dataBuffer.end() - 2)->keypoints, (dataBuffer.end() - 1)->keypoints,
                                                    (dataBuffer.end() - 2)->descriptors, (dataBuffer.end() - 1)->descriptors,
                                                    matches, matchDescriptorType, matcherType, matchSelectorType);
                                    
                                    descTime = ((double)cv::getTickCount() - descTime) / cv::getTickFrequency();

                                    cout << "IMAGE_" << imgIndex << ",";
                                    cout << dectType << ",";
                                    cout << descType << ",";
                                    cout << matches.size() << ",";
                                    cout << 1000 * detectorTime / 1.0 << ",";
                                    cout << 1000 * descTime / 1.0 << ",";
                                    cout << (1000 * detectorTime / 1.0) + (1000 * descTime / 1.0) << endl;

                                    //// EOF STUDENT ASSIGNMENT

                                    // store matches in current data frame
                                    (dataBuffer.end() - 1)->kptMatches = matches;

                                    // cout << "#4 : MATCH KEYPOINT DESCRIPTORS done." << endl;

                                    // cout << "Finalized processing following combinatio: " << dectType << "/ " << selectedDescriptorType[i] << "/ " << matchDescriptorType << "/ " << matchSelectorType << endl;
                                    // visualize matches between current and previous image
                                    if (bVis)
                                    {
                                        // here there need to be setting the "unset GTK_PATH" before it works under my local setup!
                                        cv::Mat matchImg = ((dataBuffer.end() - 1)->cameraImg).clone();
                                        cv::drawMatches((dataBuffer.end() - 2)->cameraImg, (dataBuffer.end() - 2)->keypoints,            
                                                        (dataBuffer.end() - 1)->cameraImg, (dataBuffer.end() - 1)->keypoints,
                                                        matches, matchImg,
                                                        cv::Scalar::all(-1), cv::Scalar::all(-1),
                                                        vector<char>(), cv::DrawMatchesFlags::DRAW_RICH_KEYPOINTS);

                                        string windowName = "Matching keypoints between two camera images";
                                        cv::namedWindow(windowName, cv::WINDOW_AUTOSIZE);
                                        cv::imshow(windowName, matchImg);
                                        cout << "Press key to continue to next image" << endl;
                                        cv::waitKey(0); // wait for key to be pressed
                                    }
                                    bVis = false;
                                    
                                }
                            }
                        }
    } // eof loop over all images

    return 0;
}