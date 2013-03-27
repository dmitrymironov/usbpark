/*
----------------------------------------------------------------------
CONTENTS of this file is private property of Novorado, LLC
======================================================================
Source Code is distributed for reference purposes only for Novorado 
customers. For other uses, please contact Novorado 
http://www.novorado.com or on our facebook/twitter page

You may use source code in your non-commercial and educational 
and customization projects free of charge.  

Novorado specializes in custom software and hardware design
and engineering services.
======================================================================
*/
#include	<QTextCodec>
#include	"PASettings.h"
#include	"VoiceNotification.h"
#include	<sstream>
#include	<iostream>
#include    	<festival.h>
#include	<stdexcept>

using namespace Novorado::Media;

static std::string l_speechPath;

std::string SpeechSynth::getFestivalDir(const std::string& maybeHere)
{
	QString tmp=maybeHere.c_str();

	// Check if provided path are valid
	QFileInfo fi1(tmp+"/init.scm");
	std::cout << "[1] Trying '" << fi1.absoluteFilePath().toAscii().constData() << "'" << std::endl;
	if(fi1.exists()) {
		std::cout << "[1] Found '" << fi1.absoluteFilePath().toAscii().constData() << "'" << std::endl;
		return tmp.toAscii().constData();
		}

	// No, lets look if envrionmental variables has been
	// set up

	// $SCM_INIT_PATH
	tmp=getenv("SCM_INIT_PATH");
	if(tmp.length()) {
		QFileInfo fi(tmp+"/init.scm");
		std::cout << "[2] Trying '" << fi.absoluteFilePath().toAscii().constData() << "'" << std::endl;
		if(fi.exists()) {
			std::cout << "[2] Found '" << fi1.absoluteFilePath().toAscii().constData() << "'" << std::endl;
			return tmp.toAscii().constData();
			}
		}

	// $NOVORADO_DISTRIBS/festival/lib
	tmp=getenv("NOVORADO_DISTRIBS");
	if(tmp.length()) {
		tmp += "/festival/lib";
		QFileInfo fi(tmp+"/init.scm");
		std::cout << "[3] Trying '" << fi.absoluteFilePath().toAscii().constData() << "'" << std::endl;
		if(fi.exists()) {
			std::cout << "[3] Found '" << fi.absoluteFilePath().toAscii().constData() << "'" << std::endl;
			return tmp.toAscii().constData();
			}
		}

	// Standard install pathes
#ifndef	MINGW
	// Linux: /usr/share/novorado/nvspeech
	tmp = "/usr/share/novorado/nvspeech";
	QFileInfo fi2(tmp+"/init.scm");
	std::cout << "[4] Trying '" << fi2.absoluteFilePath().toAscii().constData() << "'" << std::endl;
	if(fi2.exists()) {
		std::cout << "[4] Found '" << fi2.absoluteFilePath().toAscii().constData() << "'" << std::endl;
		return tmp.toAscii().constData();
		}
#else
	// Win32: c:\Program Files\Novorado\Parking Assistant\nvspeech
	tmp = "c:\\Program Files\\Novorado\\Parking\ Assistant\\nvspeech";
	QFileInfo fi3(tmp+"\\init.scm");
	std::cout << "[4] Trying '" << fi3.absoluteFilePath().toAscii().constData() << "'" << std::endl;
	if(fi3.exists()) {
		std::cout << "[4] Found '" << fi3.absoluteFilePath().toAscii().constData() << "'" << std::endl;
		return tmp.toAscii().constData();
		}
#endif

	// Wow, that is though. Let's look around a little bit
	// {./,../}/{./,festival/lib}
	QStringList l1, l2;
	l1 << ".";
	l1 << "..";
	l1 << "../..";
	l2 << "";
	l2 << "/festival/lib";

	for(int i=0;i<l1.size();i++) {
		for(int j=0;j<l2.size(); j++) {
			tmp = l1[i] + l2[j];
			QFileInfo fi(tmp+"/init.scm");
			std::cout << "[*] Trying '" << fi.absoluteFilePath().toAscii().constData() << "'" << std::endl;
			if(fi.exists()) {
				std::cout << "[*] Found '" << fi.absoluteFilePath().toAscii().constData() << "'" << std::endl;
				return tmp.toAscii().constData();
				}
			}
		}

	// Give up
	std::cout << "Guessing speach dir '" << maybeHere << "'" << std::endl;
	return maybeHere;
}

// A bit ugly fix with static var. It has to be a singletone because
// festival does not provide a method for closing
SpeechSynth::Flags SpeechSynth::flags;

SpeechSynth::SpeechSynth(Novorado::Parking::Settings* s)
{
	if(!flags.initialized) {

		l_speechPath = getFestivalDir(s->speechPath());
		std::cout << "Loading speech from '" << l_speechPath << "'"<< std::endl;

		festival_libdir=l_speechPath.c_str();

		//set_libdir(NVSPEECH_PATH);

		int heap_size = 2100000;  // default scheme heap size
		int load_init_files = 1; // we want the festival init files loaded

		festival_initialize(load_init_files,heap_size);
		flags.initialized=1;
		std::cout << "Speech loaded"<< std::endl;
		}
}

SpeechSynth::~SpeechSynth()
{
	// festival_say_file puts the system in async mode so we better
	// wait for the spooler to reach the last waveform before exiting
	// This isn't necessary if only festival_say_text is being used (and
	// your own wave playing stuff)
	festival_wait_for_spooler();
}

void SpeechSynth::setLanguage(SpeechSynth::SST_LANG mylang)
{
	if(flags.voice_set) {
		std::cout << "WARNING! Voice can only be set once, command ignored" << std::endl;
		return;
		}

	#if	defined PRINT_CALLS
	std::cout  << "void SpeechSynth::setLanguage(SST_LANG mylang)" << std::endl;
	#endif

	switch(mylang)
	{
		case SST_RUSSIAN:
		#if	defined PRINT_CALLS
		std::cout << "Russian voice" << std::endl;
		#endif
		flags.voice_set=1;
		if(!festival_eval_command("(voice_msu_ru_nsh_clunits)")) festival_error();
		return;

		case SST_ENGLISH:
		// voice_ked_diphone
		// us2_mbrola
		//voice_kal_diphone
		flags.voice_set=1;
		if(!festival_eval_command("(voice_ked_diphone)")) festival_error();
		return;
	}

	throw Novorado::LogicErrorException("Unknown language code");
}

void SpeechSynth::say(QString qts)
{
	#if	defined PRINT_CALLS
	std::cout  << "void VoiceNotification::say() - '" << qts.toUtf8().constData() << "'" << std::endl;
	#endif

	festival_say_text(qts.toUtf8().constData());
	emit doneTalking();
}

void VoiceNotification::suspendSignals()
{
	#if	defined PRINT_CALLS
	std::cout  << "void VoiceNotification::suspendSignals()" << std::endl;
	#endif

	QMutexLocker lk(&mutex);

	disconnect(
		this, SIGNAL(setLanguage(SpeechSynth::SST_LANG)),
		rightOfSpeech, SLOT(setLanguage(SpeechSynth::SST_LANG))
		);

	disconnect(
		this, SIGNAL(sayThat(QString)),
		rightOfSpeech, SLOT(say(QString))
		);

	disconnect(
		rightOfSpeech,SIGNAL(doneTalking()),
		this, SLOT(talkAgain())
		);
}

void VoiceNotification::onDeviceDisconnected()
{
	int timeDelta=m_lastConnectionReport.secsTo(QTime::currentTime());
//	std::cout << "void VoiceNotification::onDeviceDisconnected() -- " << timeDelta << std::endl;
	if(timeDelta<60) return;
	QString msg=tr("Please plug in your USB device");
	sayThat(msg);
	m_lastConnectionReport=QTime::currentTime();
}

void VoiceNotification::onDeviceConnected()
{
	int timeDelta=m_lastConnectionReport.secsTo(QTime::currentTime());
//	std::cout << "void VoiceNotification::onDeviceConnected() -- " << timeDelta << std::endl;
	if(timeDelta<60) return;
	sayThat(tr("USB device connected"));
	m_lastConnectionReport=QTime::currentTime();
}

void VoiceNotification::run()
{
	{
	QMutexLocker lk(&mutex);

	if(!m_settings) throw Novorado::LogicErrorException("VoiceNotification::run - settings has to be set");
	if(!rightOfSpeech) rightOfSpeech=new SpeechSynth(m_settings);

	#if	defined PRINT_CALLS
	std::cout  << "void VoiceNotification::run()" << std::endl;
	#endif

	connect(
		this, SIGNAL(setLanguage(SpeechSynth::SST_LANG)),
		rightOfSpeech, SLOT(setLanguage(SpeechSynth::SST_LANG))
		);

	connect(
		this, SIGNAL(sayThat(const QString&)),
		rightOfSpeech, SLOT(say(const QString&))
		);

	connect(
		rightOfSpeech,SIGNAL(doneTalking()),
		this, SLOT(talkAgain())
		);
	}

	// Load proper language
	if(m_settings->lang()=="russian"){
		emit setLanguage(SpeechSynth::SST_RUSSIAN);
		} else {
			emit setLanguage(SpeechSynth::SST_ENGLISH);
			}

	// Do the talking
	startVoice();
	exec();
	finishVoice();
}

void VoiceNotification::startVoice()
{
	/*
	TRANSLATOR Novorado::Media::VoiceNotification
	*/

	QString msg = tr("Parking assistance application started");
	//std::cout << "Saying '" << msg.toAscii().constData() << "'" << std::endl;
	m_ros_busy=true;
	emit sayThat(msg);
}

void VoiceNotification::finishVoice()
{
	m_ros_busy=true;
	emit sayThat(tr("Exiting parking assistance application"));
}

void VoiceNotification::talkAgain()
{
	#ifdef	PRINT_CALLS
	std::cout << "VoiceNotification::talkAgain()" << std::endl;
	#endif
	m_ros_busy=false;
}

VoiceNotification::VoiceNotification(Novorado::Parking::Settings* s)
{
	m_lastConnectionReport=QTime::currentTime();

	m_min_dist=10000;
	srand(QTime::currentTime().msec());
	m_ros_busy= false;
	rightOfSpeech = NULL;
	m_settings = s;
}

VoiceNotification::~VoiceNotification()
{
}

void VoiceNotification::sayDistanceChange(const std::map<SECTOR,DistanceCM>& distances)
{
	#if	defined PRINT_CALLS
	std::cout  << "void VoiceNotification::sayDistanceChange(std::map<SECTOR,int> distances)" << std::endl;
	#endif

	QMutexLocker lk(&mutex);

	bool flagChange=false;

	std::map<SECTOR,DistanceCM>::const_iterator i=distances.begin();

	for(;i!=distances.end();i++){

		//
		// Round by 5
		//
		int dist=i->second;

		// Round it
		dist /= 5;
		dist *= 5;

		int prev_dist = m_prev_distances[i->first];
		m_prev_distances[i->first]=dist;

		if(dist<m_min_dist) {
			m_min_dist=dist;
			m_min_sector=i->first;
			//std::cout << "Min dist:" << m_min_dist << std::endl;
			}

		if(dist!=prev_dist) {
			flagChange=true;
			}
		}

	if(m_ros_busy) return;

	if(flagChange) {

		if(m_min_dist < 70){

			QString msg;

			if(m_min_dist<=30){
				msg += tr("Stop the vehicle");
				} else if(m_min_dist<50) {
					msg += tr("Move very carefully");
					}

			msg += " ";

			msg += tr("Obstacle");

			msg += " ";

			switch(m_min_sector){
				case VNS_REAR_LL: msg += tr("on rear left"); break;
				case VNS_REAR_LC: msg += tr("in rear left center"); break;
				case VNS_REAR_RC: msg += tr("in rear right center"); break;
				case VNS_REAR_RR: msg += tr("on rear right"); break;
				case VNS_FRONT_LL: msg += tr("on front left"); break;
				case VNS_FRONT_LC: msg += tr("in front left center"); break;
				case VNS_FRONT_RC: msg += tr("in front right center"); break;
				case VNS_FRONT_RR: msg += tr("on front right"); break;
				}

			if(m_min_dist>0) {
					msg += " ";
					msg += QVariant(m_min_dist).toString();
					msg += " ";
					msg += tr(" centimeters");
					}

			m_ros_busy=true;
			m_min_dist = 1000;
			emit sayThat(msg);
			} else {
				m_ros_busy=true;
				emit sayThat(randomFreeMessage());
				}
		}
}

QString VoiceNotification::randomFreeMessage()
{

	return tr("Go");
#if	0
	switch(rand()%11){
		case 0: return tr("There is no obstacles in 2 meters range");
		case 1: return tr("No visible obstacles");
		case 2: return tr("Beware of tiny subjects that radar may not always detect. You can move");
		case 3: return tr("Move it, racer");
		case 4: return tr("You may now exercise your freedom of move");
		case 5: return tr("You are free to move in any direction");
		case 6: return tr("Proceed");
		case 7: return tr("There seems to be no obstacles in close range");
		case 8: return tr("Run, Lola run");
		case 9: return tr("Freeway, road runner");
	default:
		return tr("Radar does not see any obstacles");
	}

	return tr("Just go");
#endif
}
