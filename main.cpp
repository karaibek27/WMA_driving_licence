#include <iostream>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/text.hpp>

using namespace std;
using namespace cv;

int threshValue = 50;
Ptr<cv::text::OCRTesseract> tess = cv::text::OCRTesseract::create();

bool verifySizes(RotatedRect candidate) {


    double error = 0.4;
    // PL driving licence size: 86mm x 54mm , ratio(aspect) = 86/54=1.59259
    const double aspect = 1.59259;
    //Set min and max area.
    int min = 320 * 320;
    int max = 820 * 820;
    //caltulate values with error
    double ratioMin = aspect - aspect*error;
    double ratioMax = aspect + aspect*error;
    //calculate area
    int area = candidate.size.height*candidate.size.width;
    //calculate candidate's ratio
    double ratio = (double)candidate.size.width / (double)candidate.size.height;

    if (ratio < 1) ratio = 1 / ratio;

    if ((area <min || area>max) || (ratio<ratioMin || ratio>ratioMax)) {
        return false;
    }
    else {
        return true;
    }


    return true;
}

bool checkRectangleAreaInBorders(Rect _rectangle, int _DownBorderSize, int _UpBorderSize) {
    int rectArea = _rectangle.size().height*_rectangle.size().width;

    if (rectArea<_DownBorderSize*_DownBorderSize || rectArea>_UpBorderSize*_UpBorderSize) {
        return false;
    }else{
        return true;
    }
}

int main() {

    Mat front, back, front_original;
    string input[2];
    input[0] = "../data/20180405_190055_HDR.jpg"; //20180405_190055_HDR.jpg - front ,     20180407_165633_HDR.jpg - rotated  , 20180408_114146_HDR.jpg - rotated(2)
    input[1] = "../data/20180405_190102_HDR.jpg"; //20180405_190102_HDR.jpg
    front = imread(input[0]);
    back = imread(input[1]);
        if (front.empty() == true) {
            cout << "Front image not loaded!!!" << endl;
        }
        else if (back.empty() == true) {
            cout << "Back image not loaded!!!" << endl;
        }
        else {
            cout << "Front image loaded" << endl << "Back image loaded" << endl;
            for (int i = 0; i < 2; i++) {
                pyrDown(front, front);
            }
            front_original = front.clone();
            //convert to grayscale
            //Mat front_gray;
            //cvtColor(front, front_gray, CV_BGR2GRAY);
            ////find vertical lines.
            //Mat front_sobel;
            //Sobel(front_gray, front_sobel, CV_8U, 1, 0, 3);
            ////threshold sobeled image
            //Mat front_threshold;
            //threshold(front_sobel, front_threshold, 0, 255, CV_THRESH_OTSU + CV_THRESH_BINARY);

            //imshow("front_processed", front_threshold);
            //moveWindow("front_processed", 0, 320);
            //imshow("front", front);
            //moveWindow("front", 1000, 320);
            //cout << "Press 'q' to exit." << endl;



            //====================FIRST=================

            Mat front_binary;
            /*cvtColor(front, front_binary, COLOR_BGR2GRAY);
            cvtColor(front, front, COLOR_BGR2GRAY);
            inRange(front_binary, Scalar(0, 0, 0), Scalar(75), front_binary);*/
            cvtColor(front, front_binary, COLOR_BGR2GRAY);

            morphologyEx(front_binary, front_binary, MORPH_GRADIENT, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 1);
            threshold(front_binary, front_binary, 0, 255, THRESH_BINARY | THRESH_OTSU);

            //connecting horizontaly oriented regions
            morphologyEx(front_binary, front_binary, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(9, 1)));

            //find contours
            Mat mask = Mat::zeros(front_binary.size(), CV_8UC1);
            vector<vector<Point>> contours;
            vector<Vec4i> hierarchy;
            findContours(front_binary, contours, hierarchy, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));

            vector<Rect> mrz;
            double r = 0;
            //filter contours

            for (int idx = 0; idx >= 0; idx = hierarchy[idx][0]) {
                Rect rect = boundingRect(contours[idx]);
                r = rect.height ? (double)(rect.width / rect.height) : 0;
                if ((rect.width > front_binary.cols* .7) && /*filter from rect width*/ (r > 25) && /*filter from width::height ratio*/ (r < 36)/*filter from width::highr ratio*/)
                {
                    mrz.push_back(rect);
                    //draw rectangles
                    //rectangle(front, rect, Scalar(0, 0, 255), 1);
                }
                else {
                    waitKey(1000);
                    //draw rectangles
                    //rectangle(front, rect, Scalar(0, 0, 255), 1);

                }
            }
            if (mrz.size() == 2) {
                //CvRect max = cvMaxRect(&(CvRect)mrz[0], &(CvRect)mrz[1]);
                //rectangle(front, max, Scalar(25, 0, 0), 2); //draw MRZ

                vector<Point2f> mrzSrc;
                vector<Point2f>mrzDst;

                //MRZ region in front image
               // mrzDst.push_back(Point2f((float)max.x, (float)max.y));								//left up corner
               // mrzDst.push_back(Point2f((float)(max.x + max.width), (float)max.y));				//right up corner
                //mrzDst.push_back(Point2f((float)(max.x + max.width), (float)(max.y + max.height))); //right down corner
                //mrzDst.push_back(Point2f((float)max.x, (float)(max.y + max.height)));				//left down corner

                //MRZ in template
                mrzSrc.push_back(Point2f(0.23f, 9.3f));
                mrzSrc.push_back(Point2f(18.0f, 9.3f));
                mrzSrc.push_back(Point2f(18.0f, 10.9f));
                mrzSrc.push_back(Point2f(0.23f, 10.9f));

                //find the transformation
                Mat t = getPerspectiveTransform(mrzSrc, mrzDst);

                //photo region in our template
                vector<Point2f> photoSrc;
                photoSrc.push_back(Point2f(0.0f, 0.0f));
                photoSrc.push_back(Point2f(1.0f, 1.0f));
                photoSrc.push_back(Point2f(1.0f, 1.0f));
                photoSrc.push_back(Point2f(1.0f, 0.0f));

                Rect rect2 = boundingRect(photoSrc);


                // surname region in our template
                vector<Point2f> surnameSrc;
                surnameSrc.push_back(Point2f(6.4f, 0.7f));
                surnameSrc.push_back(Point2f(8.96f, 0.7f));
                surnameSrc.push_back(Point2f(8.96f, 1.2f));
                surnameSrc.push_back(Point2f(6.4f, 1.2f));

                vector<Point2f> photoDst(4);
                vector<Point2f> surnameDst(4);

                // map the regions from our template to image
                perspectiveTransform(photoSrc, photoDst, t);
                perspectiveTransform(surnameSrc, surnameDst, t);
                cout << "enetered" << endl;
                //draw the mapped regions
               /*for (int i = 0; i < 4; i++)
               {
                   line(front, photoDst[i], photoDst[(i + 1) % 4], Scalar(0, 128, 255), 2);
               }
               for (int i = 0; i < 4; i++)
               {
                   line(front, surnameDst[i], surnameDst[(i + 1) % 4], Scalar(0, 128, 255), 2);
               }
               */
            }
            vector<Point2f> photoSrc;
            photoSrc.push_back(Point2f(0.0f, 0.0f));
            photoSrc.push_back(Point2f(10.0f, 10.0f));
            photoSrc.push_back(Point2f(10.0f, 10.0f));
            photoSrc.push_back(Point2f(10.0f, 0.0f));


            //find max contour
            RotatedRect rect1, rectMax = minAreaRect(contours[0]);
            double area, max = (double)rect1.size.height*rect1.size.width;
            int maxIdnx = 0;
            for (int i = 0; i < contours.size(); i++) {
                rect1 = minAreaRect(contours[i]);
                area = (double)rect1.size.height*rect1.size.width;
                if (area > max) {
                    maxIdnx = i;
                    max = area;
                    rectMax = minAreaRect(contours[i]);
                }
            }
            cv::Rect roi = boundingRect(contours[maxIdnx]);
            cout << "roi at: " << roi.x << "  " << roi.y << "  " << roi.height << "  " << roi.width << endl;
            Mat ROI = front_original(roi);
            cvtColor(ROI, ROI, CV_BGR2GRAY);
            threshold(ROI, ROI, 75, 255, THRESH_BINARY);
            //morphologyEx(front_binary, front_binary, MORPH_CLOSE, getStructuringElement(MORPH_RECT, Size(1, 9)));
            //threshold(front_binary, front_binary, 55, 255, THRESH_BINARY);

            //========= Canny  ROI ============second========
            Mat front_canny;
            cvtColor(front_original, front_canny, CV_8UC1);
            cvtColor(front_canny, front_canny, CV_BGR2GRAY);
            //Find edges using canny algorithm
            Canny(front_canny, front_canny, 20, 65);
            dilate(front_canny, front_canny, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)));
            morphologyEx(front_canny, front_canny, MORPH_CLOSE, getStructuringElement(MORPH_ELLIPSE, Size(3, 3)), Point(-1, -1), 4);
            //find contours
            vector<vector<Point>> contours_canny;
            findContours(front_canny, contours_canny, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));
            //find biggest contour and make vector of minimalAreaRectangles
            vector<RotatedRect> rectangles;
            vector<vector<Point>>::iterator itc = contours_canny.begin();

            while(itc!=contours_canny.end()) {
                RotatedRect minimumRectangle = minAreaRect(Mat(*itc));
                if (verifySizes(minimumRectangle) == false) {
                    itc = contours_canny.erase(itc);
                }
                else {
                    ++itc;
                    rectangles.push_back(minimumRectangle);
                }
            }
            //Get rotation matrix
            double rr = (double)rectangles[0].size.width / (double)rectangles[0].size.height;
            double angle = rectangles[0].angle;
            if (rr < 1) { //check if rectangle is not rotated (may be)
                angle = angle + 90;
            }
            Mat rotationMatrix = getRotationMatrix2D(rectangles[0].center, angle, 1);
            //create destination for rotated affine image with driving licence
            Mat img_rotated;
            warpAffine(front_original, img_rotated, rotationMatrix, front_original.size(), CV_INTER_CUBIC);
            //crop image
            Size rect_size = rectangles[0].size;
            if (rr < 1) swap(rect_size.width, rect_size.height);
            Mat img_crop;
            getRectSubPix(img_rotated, rect_size, rectangles[0].center, img_crop);
            //resize result
            Mat resultResized;
            resultResized.create(540, 860, CV_8UC3);
            resize(img_crop, resultResized, resultResized.size(), 0, 0, INTER_CUBIC);
            //==============PHOTO===============
            //cropp photo zone
            Size photoZoneSize = Size(220, 280);
            Mat photoZone;
            Point2f photoZoneCenter = Point2f(140, 290);
            getRectSubPix(resultResized, photoZoneSize, photoZoneCenter, photoZone);
            //===========SURNAME==========
            //cropp surname zone
            Size surnameZoneSize = Size(360, 30);
            Mat surnameZone;
            Point2f surnameZoneCenter = Point2f(500, 132);
            getRectSubPix(resultResized, surnameZoneSize, surnameZoneCenter, surnameZone);
            cvtColor(surnameZone, surnameZone, CV_BGR2GRAY);
            vector<vector<Point>> surnameContours;
            //threshold(surnameZone, surnameZone, 75, 255, THRESH_BINARY_INV);
            inRange(surnameZone,0,80,surnameZone);
            threshold(surnameZone,surnameZone,10,255,THRESH_BINARY_INV);
            imwrite("surnameZone3.jpg",surnameZone);
            //OCRTesseract try
            string surnameText;
            tess->run(surnameZone, surnameText);


            findContours(surnameZone, surnameContours, RETR_CCOMP, CHAIN_APPROX_SIMPLE, Point(0, 0));
            //character ratio approx: aspect = height/width = 2/1.5 = 1.3333
            vector<vector<Point>> surnameCharacters;
            int liczba_liter = 0;
            cvtColor(surnameZone, surnameZone, CV_GRAY2BGR);
            for (int i = 0; i < surnameContours.size(); i++) {
                Rect surnameRectangle = boundingRect(surnameContours[i]);
                double surnameRectangleRatio=(double)surnameRectangle.size().height/(double)surnameRectangle.size().width, surnameRectangleRatioError = 0.4,surnameCharacterAspect=1.3333;
                if ((surnameRectangleRatio>(surnameCharacterAspect - surnameRectangleRatioError*surnameCharacterAspect) || surnameRectangleRatio<(surnameCharacterAspect + surnameRectangleRatioError*surnameCharacterAspect))&&(checkRectangleAreaInBorders(surnameRectangle,10,25))) {
                    surnameCharacters.push_back(surnameContours[i]);
                    rectangle(surnameZone, surnameRectangle, Scalar(0, 255, 0));
                    cout << ++liczba_liter << endl;
                }

            }

            vector<vector<Point>> SurnameCharacters;
            //equalize cropped and resized image
            Mat grayResult;
            cvtColor(resultResized, grayResult, CV_BGR2GRAY);
            equalizeHist(grayResult, grayResult);


            //display images
            cout << "liczba kontorow: " << contours_canny.size() << endl;
            //imshow("front_binary", front_binary);
            //moveWindow("front_binary", 0, 320);
            //imshow("front", front);
            //moveWindow("front", 1000, 320);
            cout << "Press 'q' to exit." << endl;
            //imshow("ROI", ROI);
            //imshow("front_canny", front_canny);
            cout << "number of rectangles: " << rectangles.size() << endl;
            imshow("rotated image affine", img_rotated);
            imshow("img_crop", img_crop);
            imshow("resized", resultResized);
            //imshow("grayResult", grayResult);
            imshow("photoZone", photoZone);
            imshow("surnameZone", surnameZone);
            cout << "Surname: " << surnameText << endl;

        }

    while (waitKey(10) != 'q') {

    }

    return 0;
}
