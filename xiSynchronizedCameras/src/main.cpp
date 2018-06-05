
/**
 * Test code for the stereo camera built on the xiC model produced by XIMEA.
 *
 * Author
 * ======
 *
 * Yaoyu Hu <yyhu_live@outlook.com>
 *
 * Date
 * ====
 *
 * Created: 2018-06-04
 *
 */

#include <iostream>
#include <string>
#include <sstream>

#include <stdio.h>
#include "xiApiPlusOcv.hpp"

using namespace cv;

// ============ Static global variables. ==========

const int TRIGGER_SOFTWARE       = 1;
const int EXPOSURE_MILLISEC_BASE = 1000;
const int N_IMAGES               = 3;

const std::string OUT_DIR = "../Output";

int EXPOSURE_MILLISEC(int val)
{
	return val * EXPOSURE_MILLISEC_BASE;
}

char* XI_CAMERA_SN_0 = "CUCAU1814018";
char* XI_CAMERA_SN_1 = "CUCAU1814020";

const int CAM_IDX_0 = 0;
const int CAM_IDX_1 = 1;

// =================== Macros. ==========================

#define LOOP_CAMERAS_BEGIN \
	for( int loopIdx = 0; loopIdx < 2; loopIdx++ )\
	{

#define LOOP_CAMERAS_END \
	}

// ================ Local functions. =====================

int setup_camera_common(xiAPIplusCameraOcv& cam)
{
	int sta = 0;

	// Set exposure time.
//	cam.SetExposureTime( EXPOSURE_MILLISEC(100) );
	cam.SetAutoExposureAutoGainExposurePriority(1.0);
	cam.SetAutoExposureTopLimit( EXPOSURE_MILLISEC(100) );
	cam.EnableAutoExposureAutoGain();

	// Enable auto-whitebalance.
	cam.EnableWhiteBalanceAuto();

	return sta;
}

// =============== main(). =========================

int main(int argc, char* argv[])
{
	int ret = 0;

	xiAPIplusCameraOcv cams[2];

	try
	{
		// Open the first camera for tuning.
		cams[CAM_IDX_0].OpenBySN(XI_CAMERA_SN_0);
		cams[CAM_IDX_0].SetExposureTime( EXPOSURE_MILLISEC(100) );

		// Open the second camera for tuning.
		cams[CAM_IDX_1].OpenBySN(XI_CAMERA_SN_1);
		cams[CAM_IDX_1].SetExposureTime( EXPOSURE_MILLISEC(100) );

		// Configure common parameters.
		LOOP_CAMERAS_BEGIN
			setup_camera_common(cams[loopIdx]);
		LOOP_CAMERAS_END

		// Configure synchronization.
		// set trigger mode on the first camera - as master.
		cams[CAM_IDX_0].SetTriggerSource(XI_TRG_SOFTWARE);
		cams[CAM_IDX_0].SetGPOSelector(XI_GPO_PORT1);
		cams[CAM_IDX_0].SetGPOMode(XI_GPO_EXPOSURE_ACTIVE);

		// set trigger mode on the second camera - as slave.
		cams[CAM_IDX_1].SetGPISelector(XI_GPI_PORT1);
		cams[CAM_IDX_1].SetGPIMode(XI_GPI_TRIGGER);
		cams[CAM_IDX_1].SetTriggerSource(XI_TRG_EDGE_RISING);

		// Start acquisition.
		std::cout << "Start acquisition." << std::endl;

		LOOP_CAMERAS_BEGIN
			cams[loopIdx].StartAcquisition();
		LOOP_CAMERAS_END

		// Wait for a short period of time.
		cvWaitKey(500);

		// Obtain the images.
		XI_IMG_FORMAT format;
		Mat cv_mat_image;
		std::stringstream ss;

		for ( int i = 0; i < N_IMAGES; i++ )
		{
			std::cout << "i = " << i << std::endl;

			// Trigger.
			cams[CAM_IDX_0].SetTriggerSoftware(TRIGGER_SOFTWARE);

			LOOP_CAMERAS_BEGIN
				ss.flush();
				ss.str("");
				ss.clear();

				ss << OUT_DIR << "/" << i << "_" << loopIdx << ".jpg";

				format = cams[loopIdx].GetImageDataFormat();
				cv_mat_image = cams[loopIdx].GetNextImageOcvMat();
				if (format == XI_RAW16 || format == XI_MONO16)
				{
					normalize(cv_mat_image, cv_mat_image, 0, 65536, NORM_MINMAX, -1, Mat()); // 0 - 65536, 16 bit unsigned integer range
				}

				// Save the captured image to file system.
				imwrite(ss.str(), cv_mat_image);

				std::cout << "Camera " << loopIdx << " captured image." << std::endl;

				cvWaitKey(300);
			LOOP_CAMERAS_END
		}

		// Stop acquisition.
		LOOP_CAMERAS_BEGIN
			cams[loopIdx].StopAcquisition();
		LOOP_CAMERAS_END

		std::cout << "Begin waiting..." << std::endl;
		cvWaitKey(500);

		cams[CAM_IDX_1].Close();
		cams[CAM_IDX_0].Close();
		std::cout << "Done." << std::endl;
	}
	catch ( xiAPIplus_Exception& exp )
	{
		std::cout << "Error:" << std::endl;
		exp.PrintError();
		ret = -1;
	}

	return ret;
}

