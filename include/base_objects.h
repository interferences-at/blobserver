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
 * @base_objects.h
 * A few classes and structs used in multiple files
 */

#ifndef BASE_OBJECTS_H
#define BASE_OBJECTS_H

#include <opencv2/opencv.hpp>
#include <lo/lo.h>

#include "config.h"
#if HAVE_SHMDATA
#include <shmdata/any-data-writer.h>
#endif
#include "source.h"
#include "detector.h"

using namespace std;

/*************/
// lo_address in an object
class OscClient
{
    public:
        OscClient(lo_address pAddress) {mAddress = pAddress;}
        ~OscClient() {lo_address_free(mAddress);}

        lo_address get() {return mAddress;}
        void replace(lo_address newAddress)
        {
            lo_address_free(mAddress);
            mAddress = newAddress;
        }

    private:
        lo_address mAddress;
};

#if HAVE_SHMDATA
/*************/
// Simple class to send image through shm
class ShmImage
{
    public:
        ShmImage(const char* filename);
        ~ShmImage();
        void setImage(cv::Mat& image, const unsigned long long timestamp = 0);

    private:
        shmdata_any_writer_t* _writer;
        std::string _filename;
        int _type;
        unsigned int _width, _height, _bpp;

        bool init(const unsigned int width, const unsigned int height, int type);
};
#endif // HAVE_SHMDATA

/*************/
// Struct to contain a complete flow, from capture to client
struct Flow
{
    vector<shared_ptr<Source>> sources;
    shared_ptr<Detector> detector;
#if HAVE_SHMDATA
    shared_ptr<ShmImage> shm;
#endif
    shared_ptr<OscClient> client;
    unsigned int id;
    bool run;
};

#endif // BASE_OBJECTS_H