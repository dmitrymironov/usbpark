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
// $Id: parkAssist.cpp,v 1.61 2012/03/26 16:36:31 dmi Exp $

#include 	"parkAssist.h"
#include 	<stdexcept>
#include	<QPainter>
#include	<cmath>
#include	<QSound>
#include	<iostream>
#include	<QApplication>
#include	<QMessageBox>
#include	"VoiceNotification.h"
#include	"parkSetup.h"
#include	<QDesktopWidget>
#include	"PASettings.h"
#include	<sstream>
#include	"VlcPlayer.h"
#include	<algorithm>
#if	defined SYSTEM_IS_WIN32
#include	<windows.h>
#endif
#include	<novover.h>

//#define	PRINT_CALLS	1

// The following is Qt bug workaround proposed by Trolls

/*
	TRANSLATOR ParkAssist
*/

using namespace Novorado::Parking;

void ParkAssist::onWebCamUpdate(Novorado::Media::VideoFrame* viduxa, bool clearImage)
{
	QMutexLocker lk(&viduxa->camImageLock);
	if(clearImage) {
		QPixmap blackThing(m_webcamVideo.size());
		m_webcamVideo.setPixmap(blackThing);
		} else {
			m_webcamVideo.setPixmap(QPixmap::fromImage(viduxa->camImage));
			}
}

void ParkAssist::loadSettings()
{
	m_settings->load();
}

Novorado::Parktronic& ParkAssist::parktronic()
{
	return *m_parktronic;
}

extern void removeInitialMinus(std::string& s);

void ParkAssist::loadImages()
{
	// Load proper background image
	QString bgImagePath;
	bool flagDefaultImage=0;
	if(m_settings->bgImageFilePath()=="default"){
		flagDefaultImage=1;
		bgImagePath=
			m_settings->videoEnabled() ?
				":images/PARKTRONIC_EMPTY.jpg"
			:	":images/PARKTRONIC_EMPTY_NO_VIDEO.jpg";
		} else {
			bgImagePath=m_settings->bgImageFilePath();
			}

	backgroundImage=QPixmap(bgImagePath);
	okImage=QPixmap(":images/redball.png");
	waitImage=QPixmap(":images/greyball.png");
	systrayImage=QPixmap(":images/parking50.gif");

	// Make sure we display proper application background (with car image etc.)
	if(backgroundImage.isNull()) {
		std::stringstream r;
		r << "Parktronic image '"<< bgImagePath.toUtf8().constData() << "' is empty. Try running with administrative privileges.";
		if(!flagDefaultImage){
			//
			// Quick and dirty workaround for Chua
			//
			r << "\nUsing default image";
			QMessageBox::warning(this,"Image access problem",r.str().c_str());
			bgImagePath=
				m_settings->videoEnabled() ?
					":images/PARKTRONIC_EMPTY.jpg"
				:	":images/PARKTRONIC_EMPTY_NO_VIDEO.jpg";

			backgroundImage=QPixmap(bgImagePath);
			if(backgroundImage.isNull()) throw Novorado::LogicErrorException(r.str().c_str());
			} else {
				throw Novorado::LogicErrorException(r.str().c_str());
			}

		}
}

ParkAssist::ParkAssist(int argc,char** argv):
	QWidget(NULL/*,Qt::FramelessWindowHint*/),
	//m_soundBusy(false),
	m_argc(argc),
	m_argv(argv),
	soundThread(NULL),
	voiceThread(NULL),
	m_webcamVideo(this),
	theConnectionBall(this)
	//m_flagMuteSound(true)
{
	setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

	beep = new Media::nBeep;

	std::list<std::string> largs;
	std::copy(argv,argv+(argc),std::back_inserter(largs));
	std::for_each(largs.begin(),largs.end(),removeInitialMinus);
	std::list<std::string>::iterator l;

	bool flagConfig = (std::find(largs.begin(),largs.end(),"config")!=largs.end());

	std::cout << "ParkAssist (0x" << std::hex << this << std::dec << ")" << std::endl;
	showTimer.setObjectName("Assistant Show Timer");
	hideTimer.setObjectName("Assistant Hide Timer");

	//f_configRequested=false;
	//m_flagHaveFeed=false;
	m_settings = new Parking::Settings;

	initSensors(m_settings->m_sensors);

	loadSettings();

	// Initialize and run the thread
	// WEBCAM heavily depends on loaded settings
	if(m_settings->videoEnabled()) m_webcam = new Media::WebCamThread(this,argc,argv);

	//if(flagConfig){
	//	setupDialog();
	//	}

	std::map<LineId, ParkingProtocol::PROTOCOLS> line_assignment = m_settings->getLineAssignment();

	// Tweak. Prevent from going into OSCO mode. We know the lines, it's fixed board for now
	if(!line_assignment.size()) {
		line_assignment[4]=ParkingProtocol::CHALLENGER_26;
		line_assignment[5]=ParkingProtocol::CHALLENGER_26;
		}

	// Pthread 1: Launch USB ThreadProc loop
	m_parktronic = new Parktronic(this,line_assignment.size()?&line_assignment:NULL);

	m_prevUsbPoll=QTime::currentTime();

	parktronic().setPollDelay(1);

	connect(&showTimer,SIGNAL(timeout()),this,SLOT(show()));
	connect(&hideTimer,SIGNAL(timeout()),this,SLOT(hide()));

	connect(this,SIGNAL(feedStarted()), this, SLOT(onFeedStart()),Qt::QueuedConnection);
	connect(this,SIGNAL(feedStopped()), this, SLOT(onFeedStop()),Qt::QueuedConnection);
	connect(this,SIGNAL(deviceDisconnected()), this, SLOT(onDeviceDisconnected()),Qt::QueuedConnection);

	connect(this,SIGNAL(wantFirmwareString()),this,SLOT(askFirmwareString()),Qt::QueuedConnection);
	connect(this,SIGNAL(closeInfoWindow()),this,SLOT(onCloseInfoWindow()),Qt::QueuedConnection);
	connect(this,SIGNAL(showFirmwareInfo(std::string)),this,SLOT(onShowFirmwareInfo(std::string)),Qt::QueuedConnection);

	connect(
		this,SIGNAL(firmwareOutOfDate(std::string,std::string)),
		this,SLOT(onFirmwareOutdated(std::string,std::string)),
		Qt::QueuedConnection);

	createActions();
	createTrayIcon();

	connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(messageClicked()));

	connect(
		this,SIGNAL(signalHeadChange(Novorado::HeadLogicNumber,Novorado::DistanceCM)),
		this,SLOT(onHeadChange(Novorado::HeadLogicNumber,Novorado::DistanceCM)),
		Qt::QueuedConnection);

	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
		this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

	connect(this, SIGNAL(configRequested()), this, SLOT(setupDialog()),Qt::QueuedConnection);

	loadImages();

//			palette.setBrush(f.backgroundRole(), QBrush(emptyImage));
	QPoint p(backgroundImage.width(), backgroundImage.height());
	resize(p.x(),p.y());


	setMaximumSize(size());
	setMinimumSize(size());

	trayIcon->setIcon(systrayImage);
	setWindowIcon(systrayImage);
	trayIcon->show();

	theConnectionBall.resize(waitImage.size());
	theConnectionBall.show();
	theConnectionBall.setPixmap(waitImage);

	// Move to the center
	QRect r = qApp->desktop()->screenGeometry();
	QSize sz = r.size();
	sz -= size();
	move(sz.width()/2,sz.height()/2);

	m_webcamVideo.setScaledContents(true);

	// That will create buttons
	initActions();

	// It will pop-up on device connect and data feed
	hide();
	beep->stop();

	// Pthread 2: Launch sound notification. If launched simultaneously with Pthread 1,
	// it'll create pthread_create race condition and consequently pthread package abort & app failure
	setSoundNotificationMode(m_settings->soundNotification());

	// There are no line assignments. We need to perform line detection
	if(!line_assignment.size()||flagConfig) emit configRequested();//setupDialog();

	// Activate idle polling
	QObject::connect(
		&m_idleTimer,SIGNAL(timeout()),
		this,SLOT(onIdle())
		);

	m_idleTimer.start(USB_POLLS_MSEC);
	}

QTranslator& ParkAssist::translator()
{
	return m_settings->translator();
}

void ParkAssist::setButton(QPushButton* b)
{
	QColor compat(Qt::red);
	compat.setAlpha(18);

	QPalette f=b->palette();
	f.setBrush(buttonSound->backgroundRole(),QBrush(compat));
	b->setAutoFillBackground(true);
	b->setPalette(f);
}

void ParkAssist::initActions()
{
	//
	// Sound button
	//

	buttonSound = new QPushButton(this);
 	buttonSound->setFlat(true);
		{
	QPixmap ico(":/images/sound-on.png");
	buttonSound->setIcon(ico);
	buttonSound->setIconSize(ico.size());
		}
	buttonSound->setCheckable(true);
	buttonSound->move(width()-buttonSound->width(),50);
	setButton(buttonSound);

	//
	// Sound generation button
	//
#if	0
	buttonVoice = new QPushButton(this);
 	buttonVoice->setFlat(true);
		{
	QPixmap ico(":/images/beep.png");
	buttonVoice->setIcon(ico);
	buttonVoice->setIconSize(ico.size());
		}
	buttonVoice->setCheckable(true);

	buttonVoice->move(width()-buttonVoice->width(),110);
#endif

	//
	// Minimize button
	//
	buttonMinimize = new QPushButton(this);
	buttonMinimize->setFlat(true);
	buttonMinimize->setIcon(QPixmap(":/images/minimize.png"));
	buttonMinimize->setIconSize(QSize(48,48));
	buttonMinimize->move(width()-buttonMinimize->width(),190);
	setButton(buttonMinimize);

	//
	// Settings button
	//
	buttonSettings = new QPushButton(this);
	buttonSettings->setFlat(true);
	buttonSettings->setIcon(QPixmap(":/images/settings.png"));
	buttonSettings->setIconSize(QSize(48,48));
	buttonSettings->move(width()-buttonSettings->width(),260);
	setButton(buttonSettings);

	//
	// Exit button
	//

	buttonExit = new QPushButton(this);
	buttonExit->setFlat(true);
	buttonExit->setIcon(QPixmap(":/images/quit.png"));
	buttonExit->setIconSize(QSize(48,48));
	buttonExit->move(width()-buttonExit->width(),/*230*/120);
	setButton(buttonExit);

#if	0
	connect(buttonVoice,SIGNAL(toggled(bool)),this,SLOT(buttonVoice_toggled(bool)));
#endif
	connect(buttonSettings,SIGNAL(clicked(bool)),this,SLOT(buttonSettings_clicked(bool)));
	connect(buttonMinimize,SIGNAL(clicked(bool)),this,SLOT(buttonMinimize_clicked(bool)));
	connect(buttonSound,SIGNAL(toggled(bool)),this,SLOT(buttonSound_toggled(bool)));
	connect(buttonExit,SIGNAL(clicked(bool)),this,SLOT(buttonExit_clicked(bool)));
}

void ParkAssist::buttonSound_toggled(bool)
{
	if(buttonSound->isChecked()){
		QPixmap ico(":/images/sound-off.png");
		buttonSound->setIcon(ico);
		muteSound(true);
		} else {
			QPixmap ico(":/images/sound-on.png");
			buttonSound->setIcon(ico);
			muteSound(false);
			}
}

void ParkAssist::buttonMinimize_clicked(bool)
{
	// Just hide it in an eyeblink
	showTimer.stop();
	hideTimer.start(1);
	beep->stop();
}

void ParkAssist::buttonSettings_clicked(bool)
{
	setupDialog();
}

void ParkAssist::buttonExit_clicked(bool)
{
	// That is just way too rough. It will cause glitches due to the signal
	// propagation problems and accessing callback on destruction
	// -- if(m_parktronic) m_parktronic.clear(); // that will supposedly close it

	quitApplication();
}

void ParkAssist::initSensors(std::vector<SensorVisual>& sensors){

	sensors.resize(8);

	const int front_y=89;
	const int head_angle=30, half_ha=head_angle/2;
	const int left=130,right=220;

	//  Front left
	sensors[0].init(QPoint(left,front_y),110-half_ha,head_angle);

	// 1
	sensors[1].init(QPoint((int)(left+(right-left)/4.),front_y),90-half_ha,head_angle);
	// 2
	sensors[2].init(QPoint((int)(left+(right-left)*3./4.),front_y),90-half_ha,head_angle);

	// Front right
	sensors[3].init(QPoint(right,front_y),70-half_ha,head_angle);

	// ******************

	// Rear left, 1
	int rear_y=362;
	sensors[4].init(QPoint(left,rear_y-5),250-half_ha,head_angle);
	// 2
	sensors[5].init(QPoint((int)(left+(right-left)/4.),rear_y),270-half_ha,head_angle);
	// 3
	sensors[6].init(QPoint((int)(left+(right-left)*3./4.),rear_y),270-half_ha,head_angle);
	// Rear right, 4
	sensors[7].init(QPoint(right,rear_y-5),290-half_ha,head_angle);

	for(size_t i=0;i<sensors.size();i++) {
		// Warning! m_headChange address is HeadLogicNumber
		m_headChange[1+i]=300;
		}
	}

void ParkAssist::paintSensor(QPainter& p,HeadLogicNumber _num){
	// Constants for painting
	const float area_W=200, area_H=200, sensor_H=80;
	int num_waves=10;
	const float line_width=8;
	const float step_X=area_W/num_waves, step_Y=area_H/num_waves;

	const int id = _num - 1;

	const QPoint& loc = m_settings->m_sensors[id].loc;
	const int sAng=static_cast<int>(m_settings->m_sensors[id].startAng);
	const int spanAng=static_cast<int>(m_settings->m_sensors[id].spanAng);
	const int rotateAng=270-(sAng+spanAng/2)/16;

	for(float i=0;i<num_waves;i++) {

		QRectF arcRect=QRectF(QPointF(0,0),QSizeF(area_W - i*step_X,area_H-i*step_Y));
		arcRect.moveCenter(loc);

		p.setPen(QPen(Qt::darkYellow,line_width,Qt::SolidLine, Qt::RoundCap));
		p.drawArc(arcRect,sAng,spanAng);
		}

	if(m_headChange.find(_num)==m_headChange.end()) return;

	DistanceCM dist=m_headChange[_num];

	// Set color of the sensors depending on it's active (or inactive) state
	QColor clr=m_settings->m_sensors[id].active?Qt::green:Qt::gray;

	if(dist<30) {
		clr=(Qt::red);
		} else if(dist<50) {
			clr=Qt::yellow;
			}

	num_waves=(int)(0.5+10.*float(dist)/50.);
	num_waves=std::max(num_waves,1);
	num_waves=std::min(num_waves,10);

	for(float i=0;i<num_waves;i++) {

		QRectF arcRect=QRectF(QPointF(0,0),QSizeF(area_W - (9-i)*step_X,area_H-(9-i)*step_Y));
		arcRect.moveCenter(m_settings->m_sensors[id].loc);

		p.setPen(QPen(clr,line_width,Qt::SolidLine, Qt::RoundCap));
		p.drawArc(arcRect,
			(int)(m_settings->m_sensors[id].startAng),
			(int)(m_settings->m_sensors[id].spanAng));
		}

	const int stepUp=17;
	const int smMargin=5;

	p.translate(loc.x(),loc.y());

	p.rotate(rotateAng);
	p.setPen(QPen(Qt::black,line_width));
	p.drawLine(QPoint(0,stepUp),QPoint(0,static_cast<int>(sensor_H)));

	p.setPen(QPen(Qt::red,line_width-3,Qt::SolidLine,Qt::RoundCap));
	p.drawLine(QPoint(0,stepUp+smMargin),QPoint(0,static_cast<int>(sensor_H-smMargin)));

	int yellowH=smMargin+static_cast<int>(float(std::min(dist,210))*float(sensor_H-2*smMargin)/210.);
	//std::cout << "Dist - " << dist << " Yellow Dog " << yellowH << std::endl;

	p.setPen(QPen(Qt::yellow,line_width-3,Qt::SolidLine,Qt::RoundCap));
	p.drawLine(QPoint(0,stepUp+smMargin),QPoint(0,yellowH));
	p.rotate(-rotateAng);

	p.translate(-loc.x(),-loc.y());
	}

//struct __X{__X(){ std::cout << "[>" << std::flush; } ~__X(){ std::cout << ">]" << std::flush; }};

ParkAssist::Flags::Flags(){
	soundBusy=0;
	configRequested=0;
	muteSound=1;
	haveFeed=0;
	}

void ParkAssist::onIdle()
{
	//__X __x;

	QTime mark=QTime::currentTime();

	//
	// When application is not visible, that means no active data feed
	// is present. Rate of update can be reduced to minimize controller
	// board current (120ma to somewhat 65ma on 18LF series running on 3.6 volts).
	//

	if(m_prevUsbPoll.msecsTo(mark)>(isVisible() ? USB_POLLS_MSEC : USB_IDLE_POLL)){
		//std::cout << "p+" << std::flush;
		usbPoll();//doProcess cancels it
		//std::cout << "-" << std::flush;
		m_prevUsbPoll=mark;
		}
}

void ParkAssist::setupNotifications()
{
	if(m_settings->soundNotification()==SNM_VOICE){

		if(!voiceThread) throw
			Novorado::LogicErrorException(
			"ParkAssist::setupNotifications() - voice thread has to be set in advance");

		std::map<Media::VoiceNotification::SECTOR,DistanceCM> dat;

		// Copy from head logic number to sectors
		for(std::map<HeadLogicNumber,DistanceCM>::iterator
			i=m_headChange.begin();
			i!=m_headChange.end();
			i++) {

			Media::VoiceNotification::SECTOR code=Media::VoiceNotification::VNS_FRONT_LL;

			switch(i->first){
				case 1: code=Media::VoiceNotification::VNS_FRONT_LL; break;
				case 2: code=Media::VoiceNotification::VNS_FRONT_LC; break;
				case 3: code=Media::VoiceNotification::VNS_FRONT_RC; break;
				case 4: code=Media::VoiceNotification::VNS_FRONT_RR; break;
				case 5: code=Media::VoiceNotification::VNS_REAR_LL; break;
				case 6: code=Media::VoiceNotification::VNS_REAR_LC; break;
				case 7: code=Media::VoiceNotification::VNS_REAR_RC; break;
				case 8: code=Media::VoiceNotification::VNS_REAR_RR; break;
				}

			dat[code]=i->second;
			}

		voiceThread->sayDistanceChange(dat);
		}

	int dist=1000;
	for(std::map<HeadLogicNumber,DistanceCM>::iterator
			i=m_headChange.begin();i!=m_headChange.end();i++) {
		dist=std::min(i->second,dist);
		}

	if(m_settings->soundNotification()==SNM_TONES) {

		if(dist>1000){
			msg="";
			return;
			}

		//std::cout << "dist=" << dist << std::endl;

		// Set beeper
		if(dist>100) { beep->stop(); }
		else if(dist>50) beep->setBeepInterval(1000);
		else if(dist>45) beep->setBeepInterval(600);
		else if(dist>40) beep->setBeepInterval(500);
		else if(dist>35) beep->setBeepInterval(500);
		else if(dist>30) beep->setBeepInterval(400);
		else beep->loop();
		}

	// dist: 0 .. 100
	// tones: 8 .. 1

	if(dist<10) {
		textPen=QPen(Qt::red);
		msg = "STOP";
		} else if(dist<30) {
			textPen=QPen(Qt::red);
			msg = QString("%1").arg(float(dist)/100.);
			} else if(dist<50) {
				textPen=QPen(Qt::yellow);
				msg = QString("%1").arg(float(dist)/100.);
				} else if(dist<200) {
					textPen=QPen(Qt::green);
					msg = QString("%1").arg(float(dist)/100.);
					} else {
						textPen=QPen(Qt::green);
						msg = "GO";
						}

}

void ParkAssist::paintDistance(QPainter& p)
{
	int dist=100;
	for(std::map<HeadLogicNumber,DistanceCM>::iterator
			i=m_headChange.begin();i!=m_headChange.end();i++) {

		dist=std::min(i->second,dist);
		}

#if	SYSTEM_IS_WIN32
	p.setFont(QFont("Times",78));
#else
	p.setFont(QFont("Times",70));
#endif

	QRectF textR(QPoint(7,150),QSize(300,90));

	QPen borderPen(textPen.color(),2,Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
	p.setPen(borderPen);
	QColor bgclr(30,0,0);
	bgclr.setAlpha(200);
	p.setBrush(bgclr);
	p.drawRoundedRect(textR,5,5);

	p.setPen(textPen);
	p.drawText(textR,Qt::AlignCenter,msg);
}

void ParkAssist::paintEvent ( QPaintEvent * event ){

	// Parking assistant record
	QPainter p(this);
	p.setClipRegion(event->region());

	p.drawPixmap(rect(),backgroundImage,backgroundImage.rect());
	p.setRenderHint(QPainter::Antialiasing);

	// Draw park Assistant text
	p.setPen(QPen(Qt::white));
	p.setFont(QFont("Times",24));
	p.drawText(QRectF(QPoint(350,2),QSize(420,400)),Qt::AlignCenter,"Park Assistant");

	for(size_t sensor_id=0;sensor_id<m_settings->m_sensors.size();sensor_id++) {
		if(m_settings->m_sensors[sensor_id].visible) paintSensor(p,1+sensor_id);
		}

	paintDistance(p);
	}


void ParkAssist::onDeviceSignalLost(Novorado::LineId lid)
{
	// Get all logic head numbers that associated with the given
	// line id
	std::list<Novorado::HeadLogicNumber> nums= m_settings->getHeadNumbers(lid);

	for(std::list<Novorado::HeadLogicNumber>::iterator j=nums.begin();
		j!=nums.end();j++){

		Novorado::HeadLogicNumber head=*j;

		m_settings->m_sensors[head-1].setActive(false);
		m_headChange[head]=301; // Very big distance
		}

	// Make sure sensors are updated
	update();
}

void ParkAssist::onHeadChange(HeadLogicNumber head,DistanceCM distance)
{
	if(!isVisible() && !showTimer.isActive()){
		theConnectionBall.setPixmap(okImage);
		showTimer.start(m_settings->showDelay()*1000);
		hideTimer.stop();
		}

	#if	defined PRINT_CALLS
	std::cout << "void ParkAssist::onHeadChange(int head=" << head << ",int distance=" << distance << ")" << std::endl;
	#endif

	m_headChange[head]=distance;
	m_settings->m_sensors[head-1].setActive(true);

	if(isVisible()) {
		setupNotifications();
		update();
		}
}

void ParkAssist::onNewHead(LineId lid,HeadId hid)
{
	#if defined PRINT_CALLS
	std::cout << "void ParkAssist::onNewHead(LineId lid=" << lid << ",HeadId hid" << hid << ")" << std::endl;
	#endif

	emit headConnected(lid,hid);

	// Further we will check if head is registred with the configuration.
	// If not, we have to wake the configuration dialog; It is necessary
	// for sitations when user switches the input lines and devices
	if(!m_settings->loadedHeadAssignments()) return;

	// Try reading it from the settings
	if(!m_settings->hasLine(lid)) {

		m_unknown_devices.insert(lid);

		if(!flags.configRequested){
			flags.configRequested=true;
			std::cout << "There is no device line " << lid << ", launching config" << std::endl;
			emit configRequested();
			}

		//m_settings->m_allHeads[lid][hid]=-1;
		} else {

			m_unknown_heads[lid].insert(hid);

			if(!m_settings->hasHead(lid,hid)	&& !flags.configRequested){
				flags.configRequested=true;
				//m_settings->m_allHeads[lid][hid]=-1;
				std::cout << "There is no head " << hid << " (line id "
					<< lid << "), launching config" << std::endl;
				emit configRequested();
				}
			}
			/* or else it was red from the settings already */
}

void ParkAssist::onDistanceChange(LineId lid,const Pair& p)
{
	#if	defined PRINT_CALLS
	std::cout
		<< "void ParkAssist::onDistanceChange(LineId lid=" << lid
		<< ",const Pair& p={"<< p.headId << "," << p.distanceCM << "})" << std::endl;
	#endif

	if(!m_settings->loadedHeadAssignments()) return;

	// There is no such device id (wake up a config dialog here)
	if(!m_settings->hasLine(lid)) {
		if(!flags.configRequested){
			flags.configRequested=true;
			std::cout << "Unknown device " << lid << ", launching config" << std::endl;
			emit configRequested();
			}

		return;
		}

	// There is no such head assignment (wake up a config dialog there)
	if(!m_settings->hasHead(lid,p.headId)){
		if(!flags.configRequested){
			flags.configRequested=true;
			std::cout << "Unknown head " << static_cast<int>(p.headId) << ", launching config" << std::endl;
			emit configRequested();
			}
		return;
		}

	// Here we need to use the assignment to find out which head to update
	emit signalHeadChange(m_settings->getHeadNumber(lid,p.headId),p.distanceCM);
}

void ParkAssist::fixFirmware()
{
	// Read the vals
	QSettings fs("Novorado","ParkingAssistant");
	fs.beginGroup("Firmware");
	int f = fs.value("frequency",30).toInt();
	int l = fs.value("loudness",50).toInt();
	std::vector<unsigned char> shifts(8);
	for(int i=0;i<8;i++)
		shifts[i]=fs.value(QString("transponder%1").arg(i),0).toInt();
	fs.endGroup();
	// Program back into firmware
	std::cout << "Programming buzzer (" << f << "," << l << ")" << std::endl;
	m_parktronic->configureBuzzer(f,l);
	std::cout << "Programming transponder shifts" << std::endl;
	m_parktronic->saveShift(shifts);
}

void ParkAssist::onCloseInfoWindow()
{
	if(infoWindow) {
		// Fix the firmware
		fixFirmware();

		// Close the window
		infoWindow.clear();
	}
}

void ParkAssist::onShowFirmwareInfo(std::string s)
{
	//std::cout << "void ParkAssist::onShowFirmwareInfo(std::string s=" << s << ")" << std::endl;
	if(infoWindow) infoWindow->setInformativeText(s.c_str());
}

void ParkAssist::onFirmwareOutdated(std::string d,std::string r)
{
	if(Parking::SetupDialog::visible()){
		Parking::SetupDialog::visible()->close();
		}

	QMessageBox m;
	m.setFont(BIG_FONT);
	m.setText(tr("Firmware out of date"));
	m.setInformativeText(QString(tr("Need to update firmware %1 on your device to most recent version %2").arg(d.c_str()).arg(r.c_str())));
	m.setStandardButtons(QMessageBox::Ok|QMessageBox::Cancel);
	m.setMinimumWidth(400);
	m.setIcon(QMessageBox::Critical);

	 if ( m.exec() == QMessageBox::Ok) {
	 	if(infoWindow) infoWindow.clear();
		infoWindow = new QMessageBox();
		infoWindow->setFont(BIG_FONT);
		infoWindow->setText(tr("Firmware update in progress"));
		infoWindow->setInformativeText(tr("ATTENTION!\nDO NOT TURN OFF YOUR DEIVCE OR STOP THIS PROGRAM.\nTUNRING OFF WILL BRAKE THE DEVICE"));
		infoWindow->setStandardButtons(QMessageBox::NoButton);
		infoWindow->setMinimumWidth(400);
		infoWindow->setIcon(QMessageBox::Critical);
		infoWindow->show();

		if(m_firmware) m_firmware.clear();
		m_firmware = new Novorado::Devices::Firmware;
		m_parktronic->firmwareUpdate(m_firmware->loadFirmware(r));
		}
}

void ParkAssist::onStatusChange(STATUS st, std::string s)
{
	LineId id=-1;
	PtrHolder<Novorado::Devices::Firmware> f;
	PtrHolder<VersionNumber> currentVN;
	PtrHolder<VersionNumber> recentVN;

// 	std::cout << s << std::endl;
	switch(st){
		case NRDP_DATA_FEED_STARTED:
			#if defined PRINT_CALLS
			std::cout << "Data feed started" << std::endl;
			#endif
			//
			// It means that we recognized the protocol and started to feed the numeric data
			//
			emit feedStarted();
			break;

		case NRDP_DATA_FEED_STOPPED:
			#if defined PRINT_CALLS
			std::cout << "Data feed stopped" << std::endl;
			#endif
			emit feedStopped();
			break;

		case NRDP_RECEIVING_DATA:
			#if	defined PRINT_CALLS
			std::cout << "Connected, incoming data packet" << std::endl;
			#endif
			emit deviceConnected();
			break;

		case NRDP_GOT_PING:
			emit ping();
			break;

		case NRDP_FIRMWARE_UPDATE_MODE:
			emit firmwareUpdateProgress(QString(s.c_str()));
			if(infoWindow){
				if(s=="Programming complete"||s=="Programming done") emit closeInfoWindow();
				else emit showFirmwareInfo(s);
				}
			break;

		case NRDP_CONNECTING:
			#if	defined PRINT_CALLS
			std::cout << "Connecting to the device " << s << std::endl;
			#endif
			// In detection mode, it will just keep saying that
			emit deviceDisconnected();
			break;

		case NRDP_NO_PARKTRONIC_SIGNAL:
			#if	defined PRINT_CALLS
			std::cout << "No signal: " << s << std::endl;
			#endif
			// Check message format: LINE<ID> is timeout on specific line
			if(s.find("LINE")==0){
				std::istringstream i(s);
				i.seekg(4);
				i>> id;
				emit deviceSignalLost(id);
				onDeviceSignalLost(id);
				}
			break;

		case NRDP_USB_CRITICAL_ERROR:
			#if	defined PRINT_CALLS
			std::cout << "Critical error: " << s << std::endl;
			#endif
			emit deviceDisconnected();
			break;

		case NRDP_FIRMWARE_UPDATED_OK:
			#if	defined PRINT_CALLS
			std::cout << "Finished firmware update: " << s << std::endl;
			#endif
			emit firmwareUpdateFinished(true);
			break;

		case NRDP_FIRMWARE_UPDATE_FAILED:
			#if	defined PRINT_CALLS
			std::cout << "Unable to update the firmware: " << s << std::endl;
			#endif
			emit firmwareUpdateFinished(false);
			break;

		case NRDP_DEVICE_EMULATION:
			#if	defined PRINT_CALLS
			std::cout << "Emulation mode: " << s << std::endl;
			#endif
			break;

		case NRDP_GOT_DEVICE_VERSION:
			#if	defined PRINT_CALLS
			std::cout << "Received device version: " << s << std::endl;
			#endif
			f = new Novorado::Devices::Firmware;
			currentVN=new VersionNumber(s);
			recentVN=new VersionNumber(f->recentVersion());

			if(*recentVN>*currentVN) emit firmwareOutOfDate(s,f->recentVersion());
			break;

		case NRDP_UNKNOWN_PROTOCOL:
			#if	defined PRINT_CALLS
			std::cout << "Protocol unrecognized: " << s << std::endl;
			#endif
			break;

		case	NRDP_DEVICE_ERROR:
			std::cout << "Ultrasonic device error:" << s << std::endl;
			break;

		default:
			#if	defined PRINT_CALLS
			std::cout << "STATUS " << st << ", Disconnected" << std::endl;
			#endif
			break;
		}
}

void ParkAssist::reloadApplication()
{
	std::cout << "Reloading .." << std::endl;
	qApp->exit(APP_RELOAD_CODE);
}

void ParkAssist::quitApplication()
{
	std::cout << "Exiting .." << std::endl;
	qApp->exit(0);
}

void ParkAssist::createActions()
{
	quitAction = new QAction(QObject::tr("Quit"), this);
	QObject::connect(quitAction, SIGNAL(triggered()), this, SLOT(quitApplication()));

	setupAction = new QAction(QObject::tr("Setup"), this);
	QObject::connect(setupAction, SIGNAL(triggered()), this, SLOT(setupDialog()));

	restoreAction = new QAction(QObject::tr("Show"), this);
	QObject::connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

}

void ParkAssist::createTrayIcon()
{
	trayIconMenu = new QMenu(this);

	trayIconMenu->setFont(BIG_FONT);

	trayIconMenu->addSeparator();
	trayIconMenu->addAction(setupAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	trayIcon = new QSystemTrayIcon(this);
	trayIcon->setContextMenu(trayIconMenu);
}

 void ParkAssist::setIcon(int /*index*/)
 {
/*     QIcon icon = iconComboBox->itemIcon(index);
     trayIcon->setIcon(icon);
     setWindowIcon(icon);

     trayIcon->setToolTip(iconComboBox->itemText(index));*/
 }

 void ParkAssist::showMessage()
 {
/*     QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(
             typeComboBox->itemData(typeComboBox->currentIndex()).toInt());*/
/*     trayIcon->showMessage(titleEdit->text(), bodyEdit->toPlainText(), icon,
                           durationSpinBox->value() * 1000);*/
 }

 void ParkAssist::messageClicked()
 {
     QMessageBox::information(0, tr("Systray"),
                              tr("Sorry, I already gave what help I could."
                                 "Maybe you should try asking a human?"));
 }

