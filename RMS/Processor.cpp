#include "Processor.h"
#include <opencv2\features2d\features2d.hpp>


Processor::Processor()
{
	processorHnd = CreateThread(NULL, 0, ProcessFrame, NULL, 0, &processorID);
}
//get the Similarity of the successive frame
DWORD WINAPI Processor::ProcessFrame(LPVOID lpParam)
{
	MSG msg;
	IplImage * frame_last=NULL;
	IplImage * frame_now=NULL;


	cvNamedWindow("last");
	while (1)
	{
		while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			if (msg.message == UM_WORK)
			{
				double t = getTickCount();
				IplImage* frame = (IplImage*)msg.wParam;
				if (frame_now != NULL)
				{
					cvReleaseImage(&frame_last);
					frame_last = frame_now;
					//copy the image
					IplImage* tempImg = cvCreateImage(cvGetSize(frame),
						frame->depth,
						frame->nChannels);
					cvCopy(frame, tempImg, NULL);
					frame_now = tempImg;
					//CompareImage(frame_last, frame_now);
					//cvShowImage("last", frame_last);
					//cvShowImage("camera", frame_now);
					//getORB(frame_last, frame_now);
					CompareImage(frame_last, frame_now);
					SavePicture(frame_now);
				}
				else
				{
					//copy the image
					IplImage* tempImg = cvCreateImage(cvGetSize(frame),
							frame->depth,
							frame->nChannels);
					cvCopy(frame, tempImg, NULL);
					frame_now = tempImg;
				}
				t = getTickCount() - t;
				t = t * 1000 / getTickFrequency();
				printf("%f ms\n", t);
			}
			Sleep(500);
		}
	}

	return 0;
}
void Processor::CompareImage(IplImage *image1, IplImage *image2)
{
	Mat i1(image1);
	Mat i2(image2);
	const double C1 = 6.5025, C2 = 58.5225;
	int d = CV_32F;
	Mat I1, I2;
	i1.convertTo(I1, d);
	i2.convertTo(I2, d);
	Mat I2_2 = I2.mul(I2);
	Mat I1_2 = I1.mul(I1);
	Mat I1_I2 = I1.mul(I2);
	Mat mu1, mu2;
	GaussianBlur(I1, mu1, Size(11, 11), 1.5);
	GaussianBlur(I2, mu2, Size(11, 11), 1.5);
	Mat mu1_2 = mu1.mul(mu1);
	Mat mu2_2 = mu2.mul(mu2);
	Mat mu1_mu2 = mu1.mul(mu2);
	Mat sigma1_2, sigma2_2, sigma12;
	GaussianBlur(I1_2, sigma1_2, Size(11, 11), 1.5);
	sigma1_2 -= mu1_2;
	GaussianBlur(I2_2, sigma2_2, Size(11, 11), 1.5);
	sigma2_2 -= mu2_2;
	GaussianBlur(I1_I2, sigma12, Size(11, 11), 1.5);
	sigma12 -= mu1_mu2;
	Mat t1, t2, t3;
	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma12 + C2;
	t3 = t1.mul(t2);
	t1 = mu1_2 + mu2_2 + C1;
	t2 = sigma1_2 + sigma2_2 + C2;
	t1 = t1.mul(t2);
	Mat ssim_map;
	divide(t3, t1, ssim_map);
	Scalar mssim = mean(ssim_map);
	printf("%f\n", mssim.val[0] * 100);
	printf("%f\n", mssim.val[1] * 100);
	printf("%f\n", mssim.val[2] * 100);
}

void Processor::SavePicture(IplImage *image)
{
	char * filepath = "D:\\RMS\\RMS_";
	static int pictureID = 0;
	char* pictureID_str = new char[100];
	stringstream stream;
	stream <<filepath<<pictureID++<<".jpg";
	stream >>pictureID_str;
	char * path = (LPSTR)pictureID_str;
	cvSaveImage(path, image);
}

DWORD Processor::GetProcessorId()
{
	return processorID;
}


Processor::~Processor()
{
	CloseHandle(processorHnd);
}
