
#ifndef PIPELINEBUFFER_H
#define PIPELINEBUFFER_H


#include <opencv2/opencv.hpp>
using namespace cv;

#include <vector>
#include <map>
#include <string>
using namespace std;


enum VariableType {CHAR_TYPE, INT_TYPE, FLOAT_TYPE, DOUBLE_TYPE};


class InternalVariable
{
	
	VariableType type;
	char _charv;
	int _intv;
	float _floatv;
	double _doublev;
	

public:
	InternalVariable() : type(INT_TYPE) {_intv=0; _floatv=0.; _doublev=0.0; _charv = ' ';};
	InternalVariable(char c) : type(CHAR_TYPE),  _charv(c) {_intv=0; _floatv=0.; _doublev=0.0;};
	InternalVariable(int i) : type(INT_TYPE),  _intv(i) {_charv=' '; _floatv=0.; _doublev=0.0;};
	InternalVariable(float f) : type(FLOAT_TYPE),  _floatv(f) {_intv=0; _charv=' '; _doublev=0.0;};
	InternalVariable(double d) : type(DOUBLE_TYPE),  _doublev(d) {_intv=0; _floatv=0.; _charv=' ';};

	inline VariableType GetType() const {return type;}
	
	inline char GetCharValue() const {return _charv;}
	inline int GetIntValue() const {return _intv;}
	inline float GetFloatValue() const {return _floatv;}
	inline double GetDoubleValue() const {return _doublev;}


};

class PipelineBuffer
{
	vector<Mat> outputImages;  // [0] is the original image, [1],[2] other outputs
	map<string,Mat> internalImages;
	map<string,vector<KeyPoint> > internalKeyPoints;
	map<string,InternalVariable> internalVariables; 

public:
	
	PipelineBuffer(){
		internalImages.insert(pair<string, Mat>("NULL", Mat())); // NULL matrix, is send at getInternalImage(string) method when 'find' method fails to find requested matrix
		vector<KeyPoint> nullVector; 
		internalKeyPoints.insert(pair<string, vector<KeyPoint> >("NULL", nullVector)); // NULL vector, is send at getInternalKeyPoints(string) method when 'find' method fails to find requested KeyPoints
		InternalVariable nullVar(0);
		internalVariables.insert(pair<string, InternalVariable>("NULL", nullVar)); // NULL variable, is send at getInternalVariable(string) method when 'find' method fails to find requested variable
	}

	inline Mat & getOutputImage(int i) {return outputImages[i];}
	inline Mat & getOutputImage(string s) {if (s == "orig") return outputImages[0];}


	inline void setInternalImage(string name, Mat & value) { 
		map<string,Mat>::iterator it = internalImages.find(name);	
		if (it == internalImages.end())
			internalImages.insert(pair<string, Mat>(name,value)); 
		else
			it->second = value;
	} // insert checks if the same key exists, if so does not insert or replace the value. However for Mat that shares the same datacontainer, value modification happens automatically.

	inline void setInternalVariable(string name, InternalVariable & value) { 
		map<string, InternalVariable>::iterator it = internalVariables.find(name);	
		if (it == internalVariables.end())
			internalVariables.insert(pair<string, InternalVariable>(name,value)); 
		else
			it->second = value;
	}	

	inline void setInternalKeyPoints(string name, vector<KeyPoint> & kpoints) { 
		map<string, vector<KeyPoint> >::iterator it = internalKeyPoints.find(name);	
		if (it == internalKeyPoints.end())
			internalKeyPoints.insert(pair<string, vector<KeyPoint> >(name,kpoints)); 
		else
			it->second = kpoints;
	}	


    inline Mat & getInternalImage(string name) {
		map<string, Mat>::iterator it = internalImages.find(name);
		return (it != internalImages.end()) ? it->second : internalImages["NULL"]; 
	}

    inline InternalVariable & getInternalVariable(string name) {
		map<string, InternalVariable>::iterator it = internalVariables.find(name);
		return (it != internalVariables.end()) ? it->second : internalVariables["NULL"]; 
	}

	// !!! crash if no vector<KeyPoint> !!!
    inline vector<KeyPoint> & getInternalKeyPoints(string name) {
		map<string, vector<KeyPoint> >::iterator it = internalKeyPoints.find(name);	
		return (it != internalKeyPoints.end()) ? it->second : internalKeyPoints["NULL"]; 
	}
	
	inline int setOutputImages(Mat & value, int i=-1) {
		if (i==-1 || i >= outputImages.size()){ // no index indicated
			outputImages.push_back(value);
			return outputImages.size()-1;
		}else{
			outputImages[i] = value;
			return i;
		}	
		
	}


};

#endif
