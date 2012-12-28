bin_PROGRAMS = blobserver

blobserver_SOURCES = \
    main.cpp \
    blob.cpp \
    blob_2D.cpp \
    detector.cpp \
    detector_meanOutliers.cpp \
    detector_lightSpots.cpp \
    detector_objOnAPlane.cpp \
    source.cpp \
    source_opencv.cpp

noinst_HEADERS = \
    blob.h \
    blob_2D.h \
    detector.h \
    detector_meanOutliers.h \
    detector_lightSpots.h \
    detector_objOnAPlane.h \
    source.h \
    source_opencv.h

blobserver_CXXFLAGS = \
    $(GLIB_CFLAGS) \
    $(OPENCV_CFLAGS) \
    $(LIBLO_CFLAGS) \
    $(LIBATOM_CFLAGS) \
    $(LCMS_CFLAGS)

blobserver_LDADD = \
    $(GLIB_LIBS) \
    $(OPENCV_LIBS) \
    $(LIBLO_LIBS) \
    $(LIBATOM_LIBS) \
    $(LCMS_LIBS)

if OPENNI
blobserver_SOURCES += \
    source_openni.cpp

noinst_HEADERS += \
    source_openni.h

blobserver_CXXFLAGS += \
    $(OPENNI_CFLAGS)

blobserver_LDADD += \
    $(OPENNI_LIBS)
endif
