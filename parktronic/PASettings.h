#if !defined(PASETTINGS_H)
#define	PASETTINGS_H

#include	<QSettings>
#include	<QTranslator>
#include	<string>
#include	<QDir>
#include	<map>
#include	<novopark.h>
#include	<vector>
#include	<parkAssist.h>

namespace Novorado {

	namespace Parking {

		class Settings : public QObject {

			Q_OBJECT

			// Stored value due to high-frequency multi-thread request
			ParkAssist::SOUND_NOTIFICATION_MODE m_pasnm;

			QTranslator
				m_translator;

			QString m_videoSourceMRL;

			std::string
				ssInstDir,
				m_soundPath,
				m_language;

			int
				m_showDelay,
				m_hideDelay;

			bool
				m_videoEnabled;

			void load_heads_assignment();
			void load_language();
			void load_delays();
			void load_sounds();
			void load_webcam();

			QDir instDir() const;
			QDir videolanDir() const;

			QSettings m_settings;

			void clearHeadsAssignementFromConfigurationFile();

			bool f_LogicNumbersLoaded;
			std::map<LineId, std::map<Novorado::HeadId,Novorado::HeadLogicNumber> > m_ha;

			class ThreadLock{
				public:
					ThreadLock();
					~ThreadLock();
				};

			std::map<LineId, std::set<HeadId> > m_unassigned_heads;

			public:

				// Implementing thread safety
				bool loadedHeadAssignments() const { return f_LogicNumbersLoaded; }

				std::map<LineId, std::set<HeadId> > getUnassignedHeads() const;
				bool hasLine(LineId);
				std::map<Novorado::HeadId,Novorado::HeadLogicNumber> getHeads(LineId);
				bool hasHead(LineId,HeadId);
				HeadLogicNumber getHeadNumber(LineId,HeadId);
				std::list<HeadLogicNumber> getHeadNumbers(LineId lid);
				void clearHeadAssisgnments();
				std::map<LineId, std::map<Novorado::HeadId,Novorado::HeadLogicNumber> >	heads() const;

				void addDevice(LineId);
				void assignLogicHead(LineId,HeadId,HeadLogicNumber);

				Settings();
				virtual ~Settings();
				void load();

				QString videoSourceMRL();
				void setVideoSourceMRL(const QString&);

				bool videoEnabled();
				void setVideoEnabled(bool);

				QString bgImageFilePath();
				void setBgImageFilePath(const QString&);

				void setNotification(const QString&);

				std::string soundPath() const;
				std::string speechPath() const;
				std::string installPath() const;
				std::string videolanPath() const;

				std::string lang() const { return m_language; }
				void setLang(const std::string&);

				int showDelay() const { return m_showDelay; }
				int hideDelay() const { return m_hideDelay; }

				// Protocols seatting on line id
				std::map<LineId, ParkingProtocol::PROTOCOLS> getLineAssignment();
				void setLineAssignment(std::map<LineId, ParkingProtocol::PROTOCOLS>&);

				void setHideDelay(int);
				void setShowDelay(int);

				// Both functions will write into m_allHeads
				void removeDevices();
				void saveDevices(std::map<LineId, std::set<HeadId> >&);

				ParkAssist::SOUND_NOTIFICATION_MODE soundNotification();
				void setSoundNotification(ParkAssist::SOUND_NOTIFICATION_MODE);

				std::vector<ParkAssist::SensorVisual> m_sensors;

				QTranslator& translator() { return m_translator; }

				// Diagnostics
				bool logUSB();
				void setLogUSB(bool f=true);

				bool applicationDebugMessages();
				void setApplicationDebugMessages(bool f=true);

				bool logToConsoleOnly();
				void setLogToConsoleOnly(bool f=true);

				bool logToFile();
				void setLogToFile(bool f=true);

				QString logFileName();
				void setLogFileName(const QString&);

				// Flag to clean the log file on next run
				void setCleanLog(bool f=true);
				bool cleanLog();

				int getConfigWidth();
				int getConfigHeight();

				void clear();
			};
		};
	};

#endif

