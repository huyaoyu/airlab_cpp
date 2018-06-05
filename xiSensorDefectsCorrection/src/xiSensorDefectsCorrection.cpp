
#include <iostream>

#include <stdio.h>

#include "xiApiPlusOcv.hpp"

using namespace cv;

XI_RETURN listingUserDefects(xiAPIplusCameraOcv *cam);

XI_RETURN addDefectPixel(int x, int y, xiAPIplusCameraOcv *cam);

XI_RETURN removeLastAddedPixel(xiAPIplusCameraOcv *cam);

const int SENSOR_DEFECTS_LIST_BUFFER_SIZE = 4096;

int main(int argc, char* argv[])
{
	int ret = 0;

	// Buffer for the sensor defects list.
	char sensorDefectsListBuffer[SENSOR_DEFECTS_LIST_BUFFER_SIZE];

	for ( int i=0; i < SENSOR_DEFECTS_LIST_BUFFER_SIZE; i++ )
	{
		sensorDefectsListBuffer[i] = '\0';
	}

	try
	{
		// Create a new XIMEA object.
		xiAPIplusCameraOcv cam;

		// Open the first camera for tuning.
		cam.OpenFirst();
		cam.SetExposureTime(10000);

		// Check the status of the sensor defects correction.
		bool isSDC = cam.IsSensorDefectsCorrection();

		if ( true == isSDC )
		{
			std::cout << "isSDC is true." << std::endl;
		}
		else
		{
			std::cout << "isSDC is false." << std::endl;

			// Enable sensor defects correction.
			std::cout << "Enable sensor defects correction." << std::endl;
			cam.SetSensorDefectsCorrectionListSelector(XI_SENS_DEFFECTS_CORR_LIST_SEL_FACTORY);
			cam.EnableSensorDefectsCorrection();
		}

		isSDC = cam.IsSensorDefectsCorrection();

		if ( true == isSDC )
		{
			std::cout << "isSDC is true." << std::endl;
		}
		else
		{
			std::cout << "isSDC is false." << std::endl;
		}

		// Checkout the sensor defects list selection.
		XI_SENS_DEFFECTS_CORR_LIST_SELECTOR sensDefectsCorrListSelector = cam.GetSensorDefectsCorrectionListSelector();

		switch ( sensDefectsCorrListSelector )
		{
		case XI_SENS_DEFFECTS_CORR_LIST_SEL_FACTORY:
		{
			std::cout << "The factory defects list is selected." << std::endl;
			break;
		}
		case XI_SENS_DEFFECTS_CORR_LIST_SEL_USER0:
		{
			std::cout << "The user defined defects list is selected." << std::endl;
			break;
		}
		default:
			std::cout << "Error: Unrecognized sensor defects list selection." << std::endl;
		}

		// Get the sensor defects list to the buffer.
		cam.GetSensDefectsCorrListContent(sensorDefectsListBuffer, SENSOR_DEFECTS_LIST_BUFFER_SIZE);

		std::cout << "sensorDefectsListBuffer = " << std::endl;
		std::cout << sensorDefectsListBuffer << std::endl << std::endl;

		// =============================== User defects. ========================================

		// List all image defects of type pixel from USER0 list.
		std::cout << "Pixel defects from USER0 list." << std::endl;

		listingUserDefects(&cam);

		// ========================== End of user defects. ======================================

		std::cout << "Begin waiting..." << std::endl;
		cvWaitKey(500);

		cam.Close();
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

XI_RETURN listingUserDefects(xiAPIplusCameraOcv *cam)
{
	cam->SetSensorDefectsCorrectionListSelector(XI_SENS_DEFFECTS_CORR_LIST_SEL_USER0);

	cam->SetImageCorrectionSelector(XI_DEFECT_TYPE);
	cam->SetImageCorrectionValue(XI_IMAGE_DEFECT_TYPE_PIXEL);

	cam->SetImageCorrectionSelector(XI_DEFECTS_COUNT_BY_TYPE);
	int defect_count = 0;
	defect_count = cam->GetImageCorrectionValue();
	for (int defect_id = 0; defect_id < defect_count; defect_id++)
	{
		cam->SetImageCorrectionSelector(XI_DEFECT_ID);
		cam->SetImageCorrectionValue(defect_id);
		cam->SetImageCorrectionSelector(XI_DEFECT_POS_X);
		int x = cam->GetImageCorrectionValue();
		cam->SetImageCorrectionSelector(XI_DEFECT_POS_Y);
		int y = cam->GetImageCorrectionValue();
		std::cout << "Defect Pixel ID = " << defect_id << " at x = " << x << " y = " << y << std::endl;
	}
	return XI_OK;
}

XI_RETURN addDefectPixel(int x, int y, xiAPIplusCameraOcv *cam)
{
	cam->SetImageCorrectionSelector(XI_DEFECT_POS_X);
	cam->SetImageCorrectionValue(x);

	cam->SetImageCorrectionSelector(XI_DEFECT_POS_Y);
	cam->SetImageCorrectionValue(y);

	cam->SetImageCorrectionSelector(XI_DEFECT_CMD_ADD);
	cam->SetImageCorrectionValue(1);

	cam->SetImageCorrectionSelector(XI_DEFECT_CMD_APPLY_CHANGES);
	cam->SetImageCorrectionValue(1);

	cam->SetImageCorrectionSelector(XI_DEFECT_CMD_SAVE);
	cam->SetImageCorrectionValue(1);
	return XI_OK;
}

XI_RETURN removeLastAddedPixel(xiAPIplusCameraOcv *cam)
{
	cam->SetImageCorrectionSelector(XI_DEFECT_TYPE);
	cam->SetImageCorrectionValue(XI_IMAGE_DEFECT_TYPE_PIXEL);

	cam->SetImageCorrectionSelector(XI_DEFECTS_COUNT_BY_TYPE);
	int defectCount = cam->GetImageCorrectionValue();

	cam->SetImageCorrectionSelector(XI_DEFECT_ID);
	cam->SetImageCorrectionValue(defectCount - 1);

	cam->SetImageCorrectionSelector(XI_DEFECT_CMD_DELETE);
	cam->SetImageCorrectionValue(1);

	cam->SetImageCorrectionSelector(XI_DEFECT_CMD_APPLY_CHANGES);
	cam->SetImageCorrectionValue(1);

	cam->SetImageCorrectionSelector(XI_DEFECT_CMD_SAVE);
	cam->SetImageCorrectionValue(1);
	return XI_OK;
}
