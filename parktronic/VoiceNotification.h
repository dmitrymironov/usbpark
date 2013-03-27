#ifndef	_VOICE_NOTIFICATION_H
#define	_VOICE_NOTIFICATION_H

#include	<QObject>
#include	<map>
#include	<QThread>
#include	<QMutex>
#include	"novopark.h"
#include	<QTime>

namespace Novorado {
	namespace Parking {
		class Settings;
	};

	namespace Media {

		class SpeechSynth : public QObject {
			Q_OBJECT

			struct Flags {
				unsigned char initialized : 1;
				unsigned char voice_set: 1;
				Flags():initialized(0),voice_set(0){}
				};

			static Flags flags;

			public:
				SpeechSynth(Novorado::Parking::Settings*);
				~SpeechSynth();

			enum SST_LANG {
				SST_RUSSIAN,
				SST_ENGLISH
				};

			public:
				// That method will try really hard to find where
				// festival library directory is located
				std::string getFestivalDir(const std::string&);

			public slots:
				void say(QString);
				void setLanguage(SpeechSynth::SST_LANG);

			signals:
				void doneTalking();
			};

		class VoiceNotification : public QThread {
			Q_OBJECT

			QMutex mutex;
			Novorado::Parking::Settings* m_settings;
			QString randomFreeMessage();

			public:
				VoiceNotification(Novorado::Parking::Settings*);
				~VoiceNotification();

				enum SECTOR {
					VNS_REAR_LL,
					VNS_REAR_LC,
					VNS_REAR_RC,
					VNS_REAR_RR,

					VNS_FRONT_LL,
					VNS_FRONT_LC,
					VNS_FRONT_RC,
					VNS_FRONT_RR
					};

			protected:

				int m_min_dist;
				SECTOR m_min_sector;

			public slots:

				void sayDistanceChange(const std::map<SECTOR,Novorado::DistanceCM>&);
				void startVoice();
				void finishVoice();
				void run();
				void talkAgain();
				void onDeviceDisconnected();
				void onDeviceConnected();

				void suspendSignals();

			signals:
				void setLanguage(SpeechSynth::SST_LANG);
				void sayThat(QString);

			protected:
				QTime m_lastConnectionReport;
				bool m_ros_busy;
				SpeechSynth* rightOfSpeech;
				std::map<SECTOR,Novorado::DistanceCM> m_prev_distances;
			};
		}; // namespace Media
};// namespace Novorado

#endif//_VOICE_NOTIFICATION_H
