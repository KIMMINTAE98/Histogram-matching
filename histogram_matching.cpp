#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cmath>
#include <string>
using namespace std;

#define MAX_LEVEL 256

bool Histogram_Matching(string input_file,           // input file name
						string target_file,          // target file name
						string output_file,          // output file name
					    int input_H, int input_W,    // input file height & width
						int target_H, int target_W)  // target file height & width
{
	unsigned char** input_data = new unsigned char* [input_H];    // 2D array to save input raw data
	for (int i = 0; i < input_H; i++)							  //
		input_data[i] = new unsigned char[input_W];				  //
	unsigned char** target_data = new unsigned char* [target_H];  // 2D array to save target raw data
	for (int i = 0; i < target_H; i++)							  //
		target_data[i] = new unsigned char[target_W];		      //
	unsigned char** output_data = new unsigned char* [input_H];   // 2D array to save output raw data
	for (int i = 0; i < input_H; i++)							  //
		output_data[i] = new unsigned char[input_W];			  //

	int input_histogram[MAX_LEVEL]  = { 0, };
	int target_histogram[MAX_LEVEL] = { 0, };
	double input_cdf[MAX_LEVEL]     = { 0, };
	double target_cdf[MAX_LEVEL]    = { 0, };
	int input_eq[MAX_LEVEL]         = { 0, };  // input file equalization transform function  (T(r))
	int target_eq[MAX_LEVEL]        = { 0, };  // target file equalization transform function (G(z))
	int mactching_func[MAX_LEVEL]   = { 0, };  // matching function input to output (G-1(T(r))

	FILE* fp;


	fp = fopen(input_file.c_str(), "rb");           // Open input raw file
	if (fp == NULL) {                               // if open fail,
		cout << "Input file open error!" << endl;   // print error message
		return false;                               // and return false
	}                                               //
	for (int i = 0; i < input_H; i++)               // read input raw file data
	{                                               //
		for (int j = 0; j < input_W; j++)           //
			input_data[i][j] = fgetc(fp);           //
	}                                               //
	fclose(fp);                                     // close input file stream

	fp = fopen(target_file.c_str(), "rb");          // Open target raw file
	if (fp == NULL) {                               // if open fail,
		cout << "Target file open error!" << endl;  // print error message
		return false;                               // and return false
	}                                               //                                                           
	for (int i = 0; i < target_H; i++)              // read input raw file data
	{                                               //
		for (int j = 0; j < target_W; j++)          //
			target_data[i][j] = fgetc(fp);          //
	}                                               //
	fclose(fp);                                     // close target file stream

	for (int i = 0; i < input_H; i++)               // Get 0 ~ 255 gray levels histogram about input file
	{                                               //
		for (int j = 0; j < input_W; j++)           //
			input_histogram[input_data[i][j]]++;    //
	}
	for (int i = 0; i < target_H; i++)              // Get 0 ~ 255 gray levels histogram about target file
	{												//
		for (int j = 0; j < target_W; j++)			//
			target_histogram[target_data[i][j]]++;  //
	}

	input_cdf[0] = (double)input_histogram[0] / (input_H * input_W);      // get 0 level pixel's CDF about input file
	target_cdf[0] = (double)target_histogram[0] / (target_H * target_W);  // get 0 level pixel's CDF about target file
	input_eq[0] = round((MAX_LEVEL - 1) * input_cdf[0]);                  // get 0 level pixel's equlization level about input file
	target_eq[0] = round((MAX_LEVEL - 1) * target_cdf[0]);                // get 0 level pixel's equlization level about target file
	for (int i = 1; i < MAX_LEVEL; i++)                                         // Get CDF & Equlization level (T(r) & G(z))
	{                                                                           // 
		input_cdf[i] = input_cdf[i - 1] +										// get each level pixel's CDF about input file
			           ((double)input_histogram[i] / (input_H * input_W));      // 
		target_cdf[i] = target_cdf[i - 1] +										// get each level pixel's CDF about target file
			            ((double)target_histogram[i] / (target_H * target_W));  // 
		input_eq[i] = round((MAX_LEVEL - 1) * input_cdf[i]);                    // get each level pixel's equlization level about input file  (T(r))
		target_eq[i] = round((MAX_LEVEL - 1) * target_cdf[i]);                  // get each level pixel's equlization level about target file (G(z))
	}

	for (int r = 0; r < MAX_LEVEL; r++)                             // Get Ouput matching function (G-1(T(r))
	{																//
		for (int z = 0; z < MAX_LEVEL; z++)                         // loop to find z
		{														    //
			if (input_eq[r] == target_eq[z]) {                      // if T(r) == G(z),
				mactching_func[r] = z;							    // G-1(T(r)) = z
				break;											    // and go next r
			}													    //
			else if (input_eq[r] < target_eq[z]) {                  // else if T(r) < G(z),
				if (z == 0) 									    // if z == 0,
					mactching_func[r] = 0;                          // G-1(T(r)) = 0
				else if (z - input_eq[r] <= input_eq[r] - (z - 1))  // else if G(z)-T(r) <= T(r)-G(z-1) (= T(r) is closer to G(z) than G(z-1), 
					mactching_func[r] = z;         					// G-1(T(r)) = z
				else                                                // else 
					mactching_func[r] = z - 1;                      // G-1(T(r)) = z - 1
				break;				           					    // and go next r
			}
		}
	}

	for (int i = 0; i < input_H; i++)                                  // Get Ouput data
	{																   //
		for (int j = 0; j < input_W; j++)							   //
		{															   //
			output_data[i][j] = mactching_func[input_data[i][j]];      // output = G-1(T(r)
		} 
	}

	fp = fopen(output_file.c_str(), "w+b");        // open(create) output file stream
	if (fp == NULL) {                              // if output file open fail,
		cout << "Output file open error" << endl;  // print error message
		return false;							   // and return false
	}											   //
	for (int i = 0; i < input_H; i++)              // write output data
	{                                              //
		for (int j = 0; j < input_W; j++)          //
			fputc(output_data[i][j], fp);          //
	}                                              //
	fclose(fp);                                    // close output file stream

	for (int i = 0; i < input_H; i++)   // delete input data memory
		delete[] input_data[i];			//
	delete[] input_data;				//
	for (int i = 0; i < target_H; i++)  // delete target data memory
		delete[] target_data[i];		//
	delete[] target_data;				//
	for (int i = 0; i < input_H; i++)   // delete output data memory
		delete[] output_data[i];	    //
	delete[] output_data;				//

	return true;
}

int main()
{
	if (!Histogram_Matching("barbara(512x512).raw", "Couple(512x512).raw", "barbara(512x512)_output1.raw", 512, 512, 512, 512))
		cout << "1 Histogram_Matching fail!" << endl;
	else
		cout << "1 Histogram_Matching sucess!" << endl;

	if (!Histogram_Matching("barbara(512x512).raw", "gBaboon256_256.raw", "barbara(512x512)_output2.raw", 512, 512, 256, 256))
		cout << "2 Histogram_Matching fail!" << endl;
	else
		cout << "2 Histogram_Matching sucess!" << endl;

	if (!Histogram_Matching("barbara(512x512).raw", "gAirplane350_555.raw", "barbara(512x512)_output3.raw", 512, 512, 350, 555))
		cout << "3 Histogram_Matching fail!" << endl;
	else
		cout << "3 Histogram_Matching sucess!" << endl;
	
	return 0;
}