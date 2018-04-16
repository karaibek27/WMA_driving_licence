TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

INCLUDEPATH += /usr/local/include/opencv
LIBS += -L/usr/local/lib -lopencv_superres -lopencv_videostab -lopencv_stitching -lopencv_xfeatures2d -lopencv_shape -lopencv_dpm -lopencv_aruco -lopencv_freetype -lopencv_dnn_objdetect -lopencv_hfs -lopencv_ccalib -lopencv_bioinspired -lopencv_cvv -lopencv_rgbd -lopencv_img_hash -lopencv_saliency -lopencv_face -lopencv_photo -lopencv_structured_light -lopencv_tracking -lopencv_datasets -lopencv_plot -lopencv_line_descriptor -lopencv_bgsegm -lopencv_surface_matching -lopencv_optflow -lopencv_video -lopencv_xobjdetect -lopencv_objdetect -lopencv_ximgproc -lopencv_text -lopencv_dnn -lopencv_ml -lopencv_fuzzy -lopencv_phase_unwrapping -lopencv_reg -lopencv_stereo -lopencv_calib3d -lopencv_features2d -lopencv_highgui -lopencv_videoio -lopencv_imgcodecs -lopencv_flann -lopencv_xphoto -lopencv_imgproc -lopencv_core
