/*
 * Copyright (C) 2013 Emmanuel Durand
 *
 * This file is part of blobserver.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * blobserver is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with blobserver.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @actuator_hog.h
 * The Actuator_Hog class.
 */

#ifndef ACTUATOR_HOG_H
#define ACTUATOR_HOG_H

#include <vector>

#include "config.h"
#include "actuator.h"
#include "descriptor_hog.h"
#include "blob_2D.h"

 /*************/
// Class Actuator_Hog
class Actuator_Hog : public Actuator
{
    public:
        Actuator_Hog();
        Actuator_Hog(int pParam);

        static std::string getClassName() {return mClassName;}
        static std::string getDocumentation() {return mDocumentation;}

        atom::Message detect(const std::vector< Capture_Ptr > pCaptures);
        void setParameter(atom::Message pMessage);

        std::shared_ptr<Shm> getShmObject(const char* filename) const {return std::shared_ptr<Shm>(new ShmImage(filename));}

    private:
        static std::string mClassName;
        static std::string mDocumentation;
        static unsigned int mSourceNbr;

        std::vector<Blob2D> mBlobs; // Vector of detected and tracked blobs

        // Some filtering parameters
        int mFilterSize;
        int mFilterDilateCoeff;

        // Tracking and movement filtering parameters
        int mBlobLifetime;
        int mKeepOldBlobs, mKeepMaxTime; // Parameters to set when we need blobs to be kept even when not detected anymore
        float mProcessNoiseCov, mMeasurementNoiseCov;

        // Descriptor to identify objects...
        Descriptor_Hog mDescriptor;
        // ... and its parameters
        cv::Size_<int> mRoiSize;
        cv::Size_<int> mBlockSize;
        cv::Size_<int> mCellSize;
        unsigned int mBins;
        float mSigma;

        // SVM...
        CvSVM mSvm;
        float mSvmMargin;
        bool mIsModelLoaded;
        std::vector<cv::Point> mSvmValidPositions;
        unsigned long long mMaxTimePerFrame; // Maximum time allowed per frame, in usec
        int mMaxThreads; // Maximum number of concurrent threads
        // PCA ...
        cv::PCA mPca;
        bool mIsPcaLoaded;

        // Background subtractor, used to select window of interest
        // to feed to the SVM
        cv::BackgroundSubtractorMOG2 mBgSubtractor;

        // Various variables
        cv::Mat mBgSubtractorBuffer;
        cv::RNG mRng;
        float mBlobMergeDistance; // Distance to considerer two blobs as one
        bool mSaveSamples; // If true, save samples older than mSaveSamplesAge
        unsigned long mSaveSamplesAge;

        // Methods
        void make();
        void updateDescriptorParams();
};

#endif // ACTUATOR_HOG_H
