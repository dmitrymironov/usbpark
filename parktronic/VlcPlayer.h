#ifndef	VLC_PLAYER_H
#define	VLC_PLAYER_H

#include 	<vlc/vlc.h>
#include	<QWidget>
#include	<QThread>
#include	<QMutex>
#include	<QTimer>
#include	<QTime>
#include	<map>
#include	<nbeep.h>

#include <SDL/SDL.h>
#include <SDL/SDL_mutex.h>
// Fix qMain link bug
#undef main

#include	"../sup/src/ptrholder.h"

namespace Novorado {
	namespace Parking {
		class Settings;
		class ParkAssist;
	};
};

namespace Novorado {

	namespace Media {

		class SoundPlayer : public QObject {

			Q_OBJECT

			bool done;

			signals:

				void finishedPlaying();

			public:

				SoundPlayer();
				~SoundPlayer();

			public slots:

				void play(const std::string&);
			};

		class VideoFrame;
		class WebCamThread;

		class VlcPlayer : public QObject {

			Q_OBJECT

			bool done;
			//bool f_noVideo; m_frame will be used instead

			// Libvlc vars
			libvlc_instance_t *libvlc;
			libvlc_media_t *m;
			libvlc_media_player_t *mp;

			Novorado::Parking::Settings* m_settings;

			QString mrl;

			VideoFrame* m_frame;

			WebCamThread* m_thread;

			public:

				bool hasVideo() const;// { return !f_noVideo; }

				VlcPlayer(Novorado::Parking::Settings*, WebCamThread*);
				~VlcPlayer();

				VideoFrame *videoFrame() { return m_frame; }

			public slots:

				int init();
				void stop();
			};

		class SoundThread : public QThread {

			Q_OBJECT

			SoundPlayer* dude;
			Parking::ParkAssist* parkAss;

			public:

				SoundThread(Parking::ParkAssist*);
				~SoundThread();

				void run();

			signals:

				void makeSound(const std::string&);

			public slots:

				void play(const std::string&);
			};

		class WebCamThread: public QThread {

			Q_OBJECT

//			QTimer pollTimer;

			int m_argc;
			char** m_argv;

			Parking::ParkAssist* parkAss;

			bool f_videoStarted;

			public:

				WebCamThread(Parking::ParkAssist*,int argc, char* argv[]);
				~WebCamThread();

				void run();

				Parking::ParkAssist* owner() { return parkAss; }

			signals:

				// Signals will be emitted to VlcPlayer
				void requestedStop();
				void requestedStart();

			public slots:

				// Signal retranslators
				void onVideoStop();
				void onVideoStart();

				void resize(const QRect&);
			};


		class VideoFrame : public QObject
		{
			Q_OBJECT


			VideoFrame(const VideoFrame&):QObject(NULL){}

			public:

				SDL_Surface *surf;
//				QMutex mutex;
				bool imageUpdated;

				QMutex camImageLock;
				QImage camImage;

				QTime lastCallTime;
				void resetTime() { lastCallTime = QTime::currentTime(); }
				unsigned long callCount;

				int idleTime();

				VideoFrame();

				void signalUp() { emit haveImage(this); }

			signals:
				void haveImage(Novorado::Media::VideoFrame*);
			};
	}; // namespace Media
}; // namespace Novorado
#endif

