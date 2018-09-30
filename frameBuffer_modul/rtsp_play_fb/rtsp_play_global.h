#ifndef RTSP_PLAY_GLOBAL_H
#define RTSP_PLAY_GLOBAL_H

//#include <QtCore/qglobal.h>

#if defined(RTSP_PLAY_LIBRARY)
#  define RTSP_PLAYSHARED_EXPORT Q_DECL_EXPORT
#else
#  define RTSP_PLAYSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // RTSP_PLAY_GLOBAL_H
