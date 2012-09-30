
#ifndef CAMERACAPTURE_VIDEOINPUT_H
#define CAMERACAPTURE_VIDEOINPUT_H



#include <opencv2/opencv.hpp>
#include "Pipeline.h"

// use special library for video input configurations
#include "videoInput.h"

// keyboard special handling:
#include "KeyboardHandle.h"

using namespace cv;
using namespace std;


class CameraCapture_VideoInput
{
	videoInput VI;
	int device;
	int width, height;

	int deviceR;
	int widthR, heightR;
	int numDevices;

	bool stereo;
	Pipeline * pipeline;
	Pipeline * pipelineR; // for stereo

	KeyboardHandle * keyHandler;


public:
	CameraCapture_VideoInput(Pipeline * p=0, bool stereo=false){
		pipeline = p;
		pipelineR = 0;
		keyHandler = 0;
		// list of opened devices
		device = 0;
		deviceR = 1;
		numDevices = videoInput::listDevices();	
		VI.setUseCallback(false);
		this->stereo=stereo;
	}
	~CameraCapture_VideoInput(){
		//Shut down devices properly
		if (stereo){
			VI.stopDevice(device);
			VI.stopDevice(deviceR);
		}
		else
			VI.stopDevice(device);
	}

    inline void setPipeline(Pipeline * p, Pipeline * pr = 0) {pipeline = p; pipelineR = pr;}
	inline void setKeyboardHandler(KeyboardHandle * kbh) {keyHandler = kbh;}
    inline void run(); // Method to start acquisition with pipeline processing
    inline void runWithLearning(); // Method to start acquisition, keyboard handling to take pictures and apply pipeline filters



protected:
    inline void cameraInit();
};


void CameraCapture_VideoInput::cameraInit()
{
	
	if (!VI.setupDevice(device, VI_COMPOSITE))
			cerr << "Camera load error" <<endl; 

	if (stereo){
		if (!VI.setupDevice(deviceR, VI_COMPOSITE))
			cerr << "Camera right load error" <<endl; 
	}



	width = VI.getWidth(device);
	height = VI.getHeight(device);
	
	if (stereo){
		widthR = VI.getWidth(deviceR);
		heightR = VI.getHeight(deviceR);
	}
	//namedWindow("Acquisition");

	cout << "Camera acquisition is started. Press 'q' to quit. " << endl;
	cout << "Press 's' for camera settings configuration" << endl;


}

void CameraCapture_VideoInput::run()
{
	cameraInit();

	while(1){
	
		Mat img(height, width, CV_8UC3), imgR;
		if(VI.isFrameNew(device)){
			VI.getPixels(device, img.data, false, true);	//fills pixels as a BGR (for openCV) unsigned char array - no flipping
		}
	
		if (stereo){
			imgR = Mat(heightR, widthR, CV_8UC3);
			if(VI.isFrameNew(deviceR)){
				VI.getPixels(deviceR, imgR.data, false, true);	//fills pixels as a BGR (for openCV) unsigned char array - no flipping
			}	
		}


		// Display:
		// mirror transformation:
		// flip(img,img,1);
		// cvShowImage("Acquisition", &img.operator CvMat());
		

		// Pipeline image processing:
		pipeline->setInputImage(img);
		pipeline->processPipeline();

		for (int i=0; i<pipeline->getNumberOfOutputs();i++){
			Mat pipelineOutImage(pipeline->getOutputImage(i));		
			stringstream ss;
			ss << i;
			string wname = "Pipeline_" + ss.str();
			cvShowImage(wname.c_str(), &pipelineOutImage.operator CvMat());
		}


		if (stereo){
			pipelineR->setInputImage(imgR);		
			pipelineR->processPipeline();

		
			for (int i=0; i<pipelineR->getNumberOfOutputs();i++){
				Mat pipelineOutImageR(pipelineR->getOutputImage(i));		
				stringstream ss;
				ss << i;
				string wname = "PipelineR_" + ss.str();
				cvShowImage(wname.c_str(), &pipelineOutImageR.operator CvMat());
			}

		}
		
		// keyboard handling :
		int c = cvWaitKey(5);
		if( (char) c == 'q' || (char) c == 27 ) { break; } 
		
		if( (char) c == 's' ) { 
			VI.showSettingsWindow(device);
			if (stereo){
				VI.showSettingsWindow(deviceR);
			}			
		} 
		
		if (keyHandler!=NULL){
			vector<KeyboardHandle::KeyDescription>::iterator it = keyHandler->getKeys().begin();
			for (;it!=keyHandler->getKeys().end();it++){
				if ((*it).key == (char ) c){
					
					if ((*it).action == KeyboardHandle::SAVE_IMAGE_ON_DISK){
						int index = (*it).pipelineChannel; 
						Mat img;
						if (index != -1){
							img = pipeline->getOutputImage(index);
						}else{
							img = pipeline->getBufferImage((*it).bufferElementName);
						}
						imwrite((*it).name, img);
						cout << "Image is saved on the disk" << endl;

					}else if ((*it).action == KeyboardHandle::SAVE_MATRIXDATA_ON_DISK){
						int index = (*it).pipelineChannel;
						Mat img;
						if (index != -1){
							img = pipeline->getOutputImage(index);
						}else{
							img = pipeline->getBufferImage((*it).bufferElementName);
						}
						FileStorage fs((*it).name, FileStorage::WRITE);
						int n = (*it).name.size(); 
						int pos = (*it).name.find_last_of("//");
						string item = (*it).name.substr(pos+1, n-4 - (pos+1));
						fs << item << img;
						fs.release();

						cerr << "Matrix has been saved on the disk" << endl;
					
					}else if ((*it).action == KeyboardHandle::SAVE_IMAGE_IN_BUFFER || (*it).action == KeyboardHandle::SAVE_MATRIXDATA_IN_BUFFER){
						int index = (*it).pipelineChannel;
						if (index != -1){
							Mat img = pipeline->getOutputImage(index);
							pipeline->shareBuffer()->setInternalImage((*it).name, img);
							cerr << "Matrix or Image has been saved in the buffer" << endl;
						}else{
							cerr << "pipeline channel error: pipelineChannel = -1" << endl; 
						}
						
					
					}else if ((*it).action == KeyboardHandle::LOAD_MATRIXDATA_FROM_DISK_INTO_BUFFER){
						FileStorage fs((*it).name, FileStorage::READ);
						if (fs.isOpened()){
							Mat img;
							int pos = (*it).name.find_last_of("//");
							int n = (*it).name.size();
							string item = (*it).name.substr(pos+1, n-4 - (pos+1));
							//cout << "item name : " << item << endl;
							fs[item] >> img;
							pipeline->shareBuffer()->setInternalImage((*it).bufferElementName, img);
							cout << "File has been succesfully read and matrix has been loaded into the buffer" << endl;
						}else{
							cerr << "Can not read yml data file" << endl;
						}
						fs.release();

					}else if ((*it).action == KeyboardHandle::EXEC_FUNCTION){
						

					
					}

				}
			}
		}
		
	}


}





void CameraCapture_VideoInput::runWithLearning()
{
	cameraInit();


	cerr << "Acquisition with LearningKit: " << endl;
	cerr << "	Press 'x' key to capture the current image and process with the pipeline" << endl;

    bool imageIsTaken = false;
	
	while(1){
	
		Mat img(height, width, CV_8UC3), imgR;
		if(VI.isFrameNew(device)){
			VI.getPixels(device, img.data, false, true);	//fills pixels as a BGR (for openCV) unsigned char array - no flipping
		}
	
		if (stereo){
			imgR = Mat(heightR, widthR, CV_8UC3);
			if(VI.isFrameNew(deviceR)){
				VI.getPixels(deviceR, imgR.data, false, true);	//fills pixels as a BGR (for openCV) unsigned char array - no flipping
			}	
		}


		// Display:
		// mirror transformation:
		// flip(img,img,1);
		// cvShowImage("Acquisition", &img.operator CvMat());
		

		// Display original images:
		string wname = "Original image";
		cvShowImage(wname.c_str(), &img.operator CvMat());
		if (stereo){
			string wname = "Original image R";
			cvShowImage(wname.c_str(), &imgR.operator CvMat());
		}




		// keyboard handling :
		int c = cvWaitKey(5);
		if( (char) c == 'q' || (char) c == 27 ) { break; } 
		
		if( (char) c == 's' ) { 
			VI.showSettingsWindow(device);
			if (stereo){
				VI.showSettingsWindow(deviceR);
			}			
		} 


		if ((char) c == 'x'){
			// take a photo and process it
            imageIsTaken = true;


            // Pipeline image processing:
            pipeline->setInputImage(img);
            pipeline->processPipeline();

            for (int i=0; i<pipeline->getNumberOfOutputs();i++){
                Mat pipelineOutImage(pipeline->getOutputImage(i));
                stringstream ss;
                ss << i;
                string wname = "Pipeline_" + ss.str();
                cvShowImage(wname.c_str(), &pipelineOutImage.operator CvMat());
            }


            if (stereo){
                pipelineR->setInputImage(imgR);
                pipelineR->processPipeline();


                for (int i=0; i<pipelineR->getNumberOfOutputs();i++){
                    Mat pipelineOutImageR(pipelineR->getOutputImage(i));
                    stringstream ss;
                    ss << i;
                    string wname = "PipelineR_" + ss.str();
                    cvShowImage(wname.c_str(), &pipelineOutImageR.operator CvMat());
                }

            }

		
		}




		
	}


    // close all CV windows:

    cvDestroyWindow("Original image");
    if (stereo)
        cvDestroyWindow("Original image R");

    if (imageIsTaken){
        for (int i=0; i<pipeline->getNumberOfOutputs();i++){
            stringstream ss;
            ss << i;
            string wname = "Pipeline_" + ss.str();
            cvDestroyWindow(wname.c_str());
        }

        if (stereo){
            for (int i=0; i<pipelineR->getNumberOfOutputs();i++){
                stringstream ss;
                ss << i;
                string wname = "PipelineR_" + ss.str();
                cvDestroyWindow(wname.c_str());
            }
        }
    }


}


#endif // CAMERACAPTURE_VIDEOINPUT_H
