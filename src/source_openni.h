/*
 * Copyright (C) 2012 Emmanuel Durand
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
 * @source_openni.h
 * The Source_OpenNI class.
 */

#ifndef SOURCE_OPENNI_H
#define SOURCE_OPENNI_H

#include <thread>
#include <atomic>

#include "source.h"
#include "XnCppWrapper.h"

class Source_OpenNI : public Source
{
    public:
        Source_OpenNI();
        Source_OpenNI(int pParam);
        ~Source_OpenNI();

        static void init();
        static void release();

        static std::string getClassName() {return mClassName;}
        static std::string getDocumentation() {return mDocumentation;}

        atom::Message getSubsources(); 

        bool connect();
        bool disconnect();
        bool grabFrame();
        cv::Mat retrieveFrame();

        void setParameter(atom::Message pParam);
        atom::Message getParameter(atom::Message pParam);

    private:
        static std::string mClassName;
        static std::string mDocumentation;

        static xn::Context mNiContext;
        static bool mContextOk;
        static xn::NodeInfoList mNodeInfoList;

        static std::thread mNiThread;

        void make(int pParam);
        static void capture();
};

#endif // SOURCE_OPENNI_H
