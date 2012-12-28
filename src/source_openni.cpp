#include "source_openni.h"

#define OPENNI_DEPTH    0
#define OPENNI_IMAGE    10

std::string Source_OpenNI::mClassName = "Source_OpenNI";
std::string Source_OpenNI::mDocumentation = "N/A";

xn::Context Source_OpenNI::mNiContext;
bool Source_OpenNI::mContextOk;
xn::NodeInfoList Source_OpenNI::mNodeInfoList;
std::thread Source_OpenNI::mNiThread;

/*************/
Source_OpenNI::Source_OpenNI()
{
    make(0);
}

/*************/
Source_OpenNI::Source_OpenNI(int pParam)
{
    make(pParam);
}

/*************/
void Source_OpenNI::make(int pParam)
{
    mName = mClassName;
    mSubsourceNbr = pParam;
    mBuffer = cv::Mat::zeros(0, 0, CV_8U);
}

/*************/
Source_OpenNI::~Source_OpenNI()
{
}

/*************/
void Source_OpenNI::init()
{
    // Launching the main thread
    mNiThread = std::thread(Source_OpenNI::capture);
}

/*************/
void Source_OpenNI::release()
{
    if (mContextOk)
    {
        mNiContext.Release();
    }
}

/*************/
bool Source_OpenNI::connect()
{
    return true;
}

/*************/
bool Source_OpenNI::disconnect()
{
}

/*************/
bool Source_OpenNI::grabFrame()
{
}

/*************/
cv::Mat Source_OpenNI::retrieveFrame()
{
    cv::Mat mat;
    return mat;
}

/*************/
void Source_OpenNI::setParameter(atom::Message pParam)
{
}

/*************/
atom::Message Source_OpenNI::getParameter(atom::Message pParam)
{
    atom::Message message;
    return message;
}

/*************/
atom::Message Source_OpenNI::getSubsources()
{
    atom::Message message;

    if (mContextOk)
    {
        int index = 0;
        for (xn::NodeInfoList::Iterator it = mNodeInfoList.Begin(); it != mNodeInfoList.End(); ++it)
        {
            switch ((*it).GetDescription().Type)
            {
            case XN_NODE_TYPE_DEVICE:
                message.push_back(atom::IntValue::create(index));
                message.push_back(atom::StringValue::create("Device"));
                break;
            case XN_NODE_TYPE_DEPTH:
                message.push_back(atom::IntValue::create(index));
                message.push_back(atom::StringValue::create("Depth"));
                break;
            case XN_NODE_TYPE_IMAGE:
                message.push_back(atom::IntValue::create(index));
                message.push_back(atom::StringValue::create("Image"));
                break;
            case XN_NODE_TYPE_IR:
                message.push_back(atom::IntValue::create(index));
                message.push_back(atom::StringValue::create("IR"));
                break;
            }

            index++;
        }
    }

    return message;
}

/*************/
void Source_OpenNI::capture()
{
    xn::ScriptNode script;
    xn::EnumerationErrors errors;

    // Initialize context from a configuration file
    XnStatus result = mNiContext.InitFromXmlFile("SamplesConfig.xml", script, &errors);
    if (result != XN_STATUS_OK)
    {
        mContextOk = false;
        std::cout << __FILE__ << " - " << __FUNCTION__ << " - Error while creating OpenNI context" << std::endl;
        return;
    }
    else
    {
        mContextOk = true;
        std::cout << __FILE__ << " - " << __FUNCTION__ << " - OpenNI context created" << std::endl;
    }

    // Get the list of all available nodes
    result = mNiContext.EnumerateExistingNodes(mNodeInfoList);
    if (result != XN_STATUS_OK)
    {
        std::cout << __FILE__ << " - " << __FUNCTION__ << " - Error while enumerating nodes" << std::endl;
    }

    // We loop through all available nodes
    xn::DepthGenerator depthGen;
    xn::ImageGenerator imageGen;
    xn::IRGenerator irGen;

    bool isDepth, isImage, isIR;

    for (xn::NodeInfoList::Iterator it = mNodeInfoList.Begin(); it != mNodeInfoList.End(); ++it)
    {
        switch ((*it).GetDescription().Type)
        {
        case XN_NODE_TYPE_DEPTH:
            isDepth = true;
            (*it).GetInstance(depthGen);
            break;
        case XN_NODE_TYPE_IMAGE:
            isImage = true;
            (*it).GetInstance(imageGen);
            break;
        case XN_NODE_TYPE_IR:
            isIR = true;
            (*it).GetInstance(irGen);
            break;
        }
    }

    while (true)
    {
        

        usleep(33);
    }
}
