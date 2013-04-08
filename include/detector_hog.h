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
 * switcher is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with switcher.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * @detector_hog.h
 * The Detector_Hog class.
 */

#ifndef DETECTOR_HOG_H
#define DETECTOR_HOG_H

#include <vector>

#include "config.h"
#include "detector.h"
#include "descriptor_hog.h"
#include "blob_2D.h"


 /*************/
// Class Detector_Hog
class Detector_Hog : public Detector
{
    public:
        Detector_Hog();
        Detector_Hog(int pParam);

        static std::string getClassName() {return mClassName;}
        static std::string getDocumentation() {return mDocumentation;}

        atom::Message detect(std::vector<cv::Mat> pCaptures);
        void setParameter(atom::Message pMessage);

    private:
        static std::string mClassName;
        static std::string mDocumentation;
        static unsigned int mSourceNbr;

        std::vector<Blob2D> mBlobs; // Vector of detected and tracked blobs

        // Some filtering parameters
        int mFilterSize;

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

        // Background subtractor, used to select window of interest
        // to feed to the SVM
        BackgroundSubtractorMOG2 mBgSubtractor;

        // Various buffers
        cv::Mat mBgSubtractorBuffer;

        void make();
        void updateDescriptorParams();
};

#endif // DETECTOR_HOG_H