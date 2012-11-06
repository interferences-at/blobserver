#include "detector_objOnAPlane.h"

using namespace atom;

std::string Detector_ObjOnAPlane::mClassName = "Detector_ObjOnAPlane";
std::string Detector_ObjOnAPlane::mDocumentation = "N/A";

/*****************/
Detector_ObjOnAPlane::Detector_ObjOnAPlane()
{
    make();
}

/*****************/
Detector_ObjOnAPlane::Detector_ObjOnAPlane(int pParam)
{
    make();
}

/*****************/
void Detector_ObjOnAPlane::make()
{
    mName = mClassName;
    mOscPath = "/blobserver/objOnAPlane";

    mMaxTrackedBlobs = 8;
    mDetectionLevel = 10.0;

    mProcessNoiseCov = 1e-5;
    mMeasurementNoiseCov = 1e-5;

    // Set mBlobDetector to indeed detect light
    cv::SimpleBlobDetector::Params lParams;
    lParams.filterByColor = true;
    lParams.blobColor = 255;
    lParams.minCircularity = 0.1f;
    lParams.maxCircularity = 1.f;
    lParams.minInertiaRatio = 0.f;
    lParams.maxInertiaRatio = 1.f;
    lParams.minArea = 10.f;
    lParams.maxArea = 65535.f;
    mBlobDetector.reset(new cv::SimpleBlobDetector(lParams));
}

/*****************/
atom::Message Detector_ObjOnAPlane::detect(std::vector<cv::Mat> pCaptures)
{
    // If the spaces definition changed
    if (mMapsUpdated == false)
        updateMaps(pCaptures);

    // Conversion of captures from their own space to a common space
    std::vector<cv::Mat> correctedCaptures;

    int index = 0;
    std::vector<cv::Mat>::iterator iterCapture;
    std::vector<cv::Mat>::const_iterator iterMap;
    for (iterCapture = pCaptures.begin(), iterMap = mMaps.begin()+1;
        iterCapture != pCaptures.end();
        ++iterCapture, ++iterMap)
    {
        cv::Mat capture = *iterCapture;
        cv::Mat map = *iterMap;

        cv::Mat correctedCapture;
        cv::remap(capture, correctedCapture, map, cv::Mat(), cv::INTER_AREA);
        char str[16];
        sprintf(str, "capture %i", index);
        index++;
        cv::imshow(str, correctedCapture);
        correctedCaptures.push_back(correctedCapture);
    }

    // Resize all captures to match the first one,
    // and convert all of them to HSV
    cv::cvtColor(correctedCaptures[0], correctedCaptures[0], CV_BGR2HSV);
    for (iterCapture = correctedCaptures.begin()+1; iterCapture != correctedCaptures.end(); ++iterCapture)
    {
        cv::Mat capture = *iterCapture;
        cv::Mat resized;

        cv::resize(capture, resized, correctedCaptures[0].size(), 0, 0, cv::INTER_LINEAR);
        cv::cvtColor(resized, capture, CV_BGR2HSV);
    }

    // Compare the first capture to all the others
    cv::Mat master = correctedCaptures[0];
    cv::Mat detected = cv::Mat::zeros(master.rows, master.cols, CV_8U);

    for (iterCapture = correctedCaptures.begin(); iterCapture != correctedCaptures.end(); ++iterCapture)
    {
        cv::Mat capture = *iterCapture;

        for (int x = 0; x < master.cols; ++x)
        {
            for (int y = 0; y < master.rows; ++y)
            {
                float dist = cv::norm(master.at<cv::Vec3b>(y, x), capture.at<cv::Vec3b>(y, x), cv::NORM_L2);
                if (dist > mDetectionLevel)
                    detected.at<uchar>(y, x) = 255;
            }
        }
    }
    
    // Convert the detection to real space
    cv::Mat realDetected;
    cv::remap(detected, realDetected, mMaps[0], cv::Mat(), cv::INTER_AREA);

    // Apply the mask
    cv::Mat lMask = getMask(realDetected, CV_INTER_NN);
    for (int x = 0; x < realDetected.cols; ++x)
        for (int y = 0; y < realDetected.rows; ++y)
        {
            if (lMask.at<uchar>(y, x) == 0)
                realDetected.at<uchar>(y, x) = 0;
        }

    // Detect blobs
    std::vector<cv::KeyPoint> lKeyPoints;
    mBlobDetector->detect(realDetected, lKeyPoints);

    // We use Blob::properties, not cv::KeyPoints
    std::vector<Blob::properties> lProperties;
    for(int i = 0; i < std::min((int)(lKeyPoints.size()), mMaxTrackedBlobs); ++i)
    {
        Blob::properties properties;
        properties.position.x = lKeyPoints[i].pt.x;
        properties.position.y = lKeyPoints[i].pt.y;
        properties.size = lKeyPoints[i].size;
        properties.speed.x = 0.f;
        properties.speed.y = 0.f;

        lProperties.push_back(properties);
    }

    // We want to track them
    trackBlobs<Blob2D>(lProperties, mBlobs);

    // Make sure their covariances are correctly set
    std::vector<Blob2D>::iterator lIt = mBlobs.begin();
    for(; lIt != mBlobs.end(); ++lIt)
    {
        lIt->setParameter("processNoiseCov", mProcessNoiseCov);
        lIt->setParameter("measurementNoiseCov", mMeasurementNoiseCov);
    }

    // And we send and print them
    Message message;
    // Include the number and size of each blob in the message
    message.push_back(IntValue::create((int)mBlobs.size()));
    message.push_back(IntValue::create(6));

    for(int i = 0; i < mBlobs.size(); ++i)
    {
        int lX, lY, lSize, ldX, ldY, lId;
        Blob::properties properties = mBlobs[i].getBlob();
        lX = (int)(properties.position.x);
        lY = (int)(properties.position.y);
        lSize = (int)(properties.size);
        ldX = (int)(properties.speed.x);
        ldY = (int)(properties.speed.y);
        lId = (int)mBlobs[i].getId();

        // Print the blob number on the blob
        char lNbrStr[8];
        sprintf(lNbrStr, "%i", lId);
        cv::putText(realDetected, lNbrStr, cv::Point(lX, lY), cv::FONT_HERSHEY_COMPLEX, 0.66, cv::Scalar(128.0, 128.0, 128.0, 128.0));

        // Add this blob to the message
        message.push_back(IntValue::create(lX));
        message.push_back(IntValue::create(lY));
        message.push_back(IntValue::create(lSize));
        message.push_back(IntValue::create(ldX));
        message.push_back(IntValue::create(ldY));
        message.push_back(IntValue::create(lId));
    }

    // Save the result in a buffer
    mOutputBuffer = realDetected;

    return message;
}

/*****************/
void Detector_ObjOnAPlane::setParameter(Message pMessage)
{
    Message::const_iterator iter = pMessage.begin();

    std::string cmd;
    try
    {
        cmd = toString(pMessage[0]);
    }
    catch (BadTypeTagError error)
    {
        return;
    }

    // Add a new space to the vector
    if (cmd == "addSpace")
    {
        int size = 0;
        if (mSpaces.size() != 0)
            size = mSpaces[0].size();

        if (pMessage.size() == size + 1 || size == 0)
        {
            std::vector<cv::Vec2f> space;
            for (int i = 1; i < pMessage.size(); i+=2)
            {
                cv::Vec2f point;
                try
                {
                    point[0] = toFloat(pMessage[i]);
                    point[1] = toFloat(pMessage[i+1]);
                }
                catch (BadTypeTagError error)
                {
                    return;
                }
                space.push_back(point);
            }
            mSpaces.push_back(space);
            mMapsUpdated = false;
        }
    }
    // Replace all the current spaces at once
    else if (cmd == "setSpaces")
    {
        if (pMessage.size() <= 2)
            return;

        int number;
        try
        {
            number = toInt(pMessage[1]);
        }
        catch (BadTypeTagError error)
        {
            return;
        }

        int size = (pMessage.size()-2) / number;
        // If all spaces specified have not the same size
        // or size is an odd number (we need 2D points)
        // or not enough points are given for each space (3 minimum)
        if (size*number < pMessage.size()-2 || size % 2 != 0  || size < 6)
            return;

        std::vector<std::vector<cv::Vec2f>> tempSpaces;
        for (int i = 0; i < number; ++i)
        {
            std::vector<cv::Vec2f> space;
            for (int j = 0; j < size; j+=2)
            {
                cv::Vec2f point;
                try
                {
                    point[0] = toFloat(pMessage[i*size+2+j]);
                    point[1] = toFloat(pMessage[i*size+2+j+1]);
                }
                catch (BadTypeTagError error)
                {
                    return;
                }

                space.push_back(point);
            }
            tempSpaces.push_back(space);
        }
        
        // Everything went fine, we can replace the old spaces
        mSpaces.clear();
        for (int i = 0; i < tempSpaces.size(); ++i)
            mSpaces.push_back(tempSpaces[i]);

        mMapsUpdated = false;
    }
    else if (cmd == "clearSpaces")
    {
        mSpaces.clear();
        mMapsUpdated = false;
    }
}

/*****************/
void Detector_ObjOnAPlane::updateMaps(std::vector<cv::Mat> pCaptures)
{
    mMaps.clear();

    // Creating map to convert from uniform space to the real one
    {
        std::vector<cv::Vec2f> space = mSpaces[0];

        cv::Vec2f baseX, baseY, origin;
        int size = space.size();

        baseX[0] = space[1][0] - space[0][0];
        baseX[1] = space[1][1] - space[0][1];
        baseX /= cv::norm(baseX);
        
        // The second vector has to be ortho
        baseY[0] = -baseX[1];
        baseY[1] = baseX[0];

        // Temporary origin
        origin = space[0];
        
        // We have an ortho base, we can use the dot product
        std::vector<cv::Vec2f> newCoords;
        newCoords.push_back(origin);

        cv::Vec2f max = 0.f;
        std::vector<cv::Vec2f>::iterator iterPoint;
        for (iterPoint = space.begin()+1; iterPoint != space.end(); ++iterPoint)
        {
            cv::Vec2f point = *iterPoint;
            cv::Vec2f vector = point - origin;

            cv::Vec2f coords;
            coords[0] = vector.dot(baseX);
            coords[1] = vector.dot(baseY);

            if (coords[0] < 0.f)
            {
                origin[0] += coords[0];
                coords[0] = 0.f;

            }
            if (coords[1] < 0.f)
            {
                origin[1] += coords[1];
                coords[1] = 0.f;
            }

            max[0] = std::max(max[0], coords[0]);
            max[1] = std::max(max[1], coords[1]);
            
            // New coords for the point
            newCoords.push_back(coords);
        }

        // Create the map
        float ratio = (float)(max[1])/(float)(max[0]);
        cv::Mat map = cv::Mat::zeros((int)(1024*ratio), 1024, CV_32FC2); 

        // We now need to projection from the uniform space to real space
        // Calculates the tranformation matrix
        cv::Point2f inPoints[4];
        cv::Point2f outPoints[4];
        for (int i = 0; i < 4; ++i)
            inPoints[i] = newCoords[i];

        outPoints[0] = cv::Point2f(0.f, 0.f);
        outPoints[1] = cv::Point2f((float)(map.cols), 0.f);
        outPoints[2] = cv::Point2f((float)(map.cols), (float)(map.rows));
        outPoints[3] = cv::Point2f(0.f, (float)(map.rows));

        cv::Mat transformMat = cv::getPerspectiveTransform(inPoints, outPoints);

        // Create the map
        cv::Mat tmpMap = map.clone(); 

        for (int x = 0; x < tmpMap.cols; ++x)
        {
            for (int y = 0; y < tmpMap.rows; ++y)
            {
                tmpMap.at<cv::Vec2f>(y, x) = cv::Vec2f(x, y);
            }
        }

        cv::warpPerspective(tmpMap, map, transformMat, tmpMap.size(), cv::INTER_LINEAR);

        mMaps.push_back(map);
    }

    // Creating maps for the captures
    std::vector<std::vector<cv::Vec2f>>::const_iterator iterSpace;
    std::vector<cv::Mat>::const_iterator iterCapture;
    for (iterSpace = mSpaces.begin()+1, iterCapture = pCaptures.begin();
        iterSpace != mSpaces.end() && iterCapture != pCaptures.end();
        ++iterSpace, ++iterCapture)
    {
        std::vector<cv::Vec2f> space = *iterSpace;
        cv::Mat capture = *iterCapture;

        // Calculates the tranformation matrix
        cv::Point2f inPoints[4];
        cv::Point2f outPoints[4];
        for (int i = 0; i < 4; ++i)
            inPoints[i] = space[i];

        outPoints[0] = cv::Point2f(0.f, 0.f);
        outPoints[1] = cv::Point2f((float)(capture.cols), 0.f);
        outPoints[2] = cv::Point2f((float)(capture.cols), (float)(capture.rows));
        outPoints[3] = cv::Point2f(0.f, (float)(capture.rows));

        cv::Mat transformMat = cv::getPerspectiveTransform(inPoints, outPoints);

        // Create the map
        cv::Mat tmpMap = cv::Mat::zeros(capture.rows, capture.cols, CV_32FC2); 
        cv::Mat map = cv::Mat::zeros(capture.rows, capture.cols, CV_32FC2); 

        for (int x = 0; x < tmpMap.cols; ++x)
        {
            for (int y = 0; y < tmpMap.rows; ++y)
            {
                tmpMap.at<cv::Vec2f>(y, x) = cv::Vec2f(x, y);
            }
        }

        cv::warpPerspective(tmpMap, map, transformMat, tmpMap.size(), cv::INTER_LINEAR);

        mMaps.push_back(map);
    }

    mMapsUpdated = true;
}