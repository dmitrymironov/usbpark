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
#include	"audi.h"
#include	<QResizeEvent>
#include	<QApplication>
#include	<QPen>
#include	<QPainter>
#include	<math.h>
#include	<iostream>
#include	<QImage>
#include 	<QSystemTrayIcon>
#include	<QSettings>
#include	<sstream>
#include	<stdexcept>
#include	<QFile>
#include	<windows.h>
#include	<novover.h>
#include	<QMessageBox>
#include	<QFileInfo>
#include	<QDesktopWidget>
#include	<QPushButton>

#define	IDLE_POLL	1000
#define	WORK_POLL	100
#define	SWITCH_TO_SLOW_TIMEOUT 5000

int	VUTIL	=80;
int	HUTIL	=57;

#define	SHIFT_F 25
#define	SHIFT_R	15

using namespace Novorado::Audi;

float Sector::lookup_radprc(int distcm)
{
	if(distcm<=30) return 15;

	if(distcm<=100) return 80*distcm/100.;

	return 50+100*distcm/255;
}

// loc - center of ellypse coordinates
void DataSector::get2point(QPoint loc, QRect car, QRect& pix, Sector& sens)
{
	QRect& draw=sens.ellipseRect;
	QRect& shad=sens.shadowRect;
	QRect& sn=sens.sensRect;

	// Make bounding box of 4 points of the chord
	int
		irad = std::min(internal_radius.x()*car.width()/100,internal_radius.y()*car.height()/100),
		erad = std::max(external_radius.x()*car.width()/100,external_radius.y()*car.height()/100);
//		sens_rad=irad+sens.distancePerc*(erad-irad)/100;

	QPoint sensor_radius(
		(internal_radius.x()+sens.distancePerc*(external_radius.x()-internal_radius.x())/100)*car.width()/100,
		(internal_radius.y()+sens.distancePerc*(external_radius.y()-internal_radius.y())/100)*car.height()/100
		);

	double
		CA = cos(angle*M_PI/180), SA = -sin(angle*M_PI/180),
		CSA = cos((angle+span)*M_PI/180), SSA = -sin((angle+span)*M_PI/180); // - because qt coord sys rev on y

	pix = QRect(
		QPoint(static_cast<int>(irad * CA), static_cast<int>(irad * SA)),
		QPoint(static_cast<int>(erad * CA), static_cast<int>(erad * SA))
		);

	pix = pix.unite(
			QRect(
		QPoint(static_cast<int>(irad * CSA), static_cast<int>(irad * SSA)),
		QPoint(static_cast<int>(erad * CSA), static_cast<int>(erad * SSA))
			)
		);

	pix.translate(loc);

	// Find out size of the drawing rectangle
	draw.setLeft(loc.x()-external_radius.x()*car.width()/100);
	draw.setBottom(loc.y()+external_radius.y()*car.height()/100);
	draw.setRight(loc.x()+external_radius.x()*car.width()/100);
	draw.setTop(loc.y()-external_radius.y()*car.height()/100);

	shad.setLeft(loc.x()-internal_radius.x()*car.width()/100);
	shad.setBottom(loc.y()+internal_radius.y()*car.height()/100);
	shad.setRight(loc.x()+internal_radius.x()*car.width()/100);
	shad.setTop(loc.y()-internal_radius.y()*car.height()/100);

	if(sens.distancePerc<=100){
		sn.setLeft(loc.x()-sensor_radius.x());
		sn.setBottom(loc.y()+sensor_radius.y());
		sn.setRight(loc.x()+sensor_radius.x());
		sn.setTop(loc.y()-sensor_radius.y());
		sn.translate(-pix.x(),-pix.y());
		} else {
			sn=QRect(); //null rect
			}

	// Shift drawing rect to coordinates of pix
	draw.translate(-pix.x(),-pix.y());
	shad.translate(-pix.x(),-pix.y());
}

#define	F_VIR 55
#define F_HIR 29
#define F_HER 50
#define	F_VER 100

#define	R_VIR 55
#define R_HIR 17
#define	R_HER 50
#define R_VER 100

#define	EDGE_SHIFT_F 7
#define	EDGE_SHIFT 20

//5085
#define	CAR_LENGTH_CM	508
// 1984
#define	CAR_WIDTH_CM	198

static DataSector _sdata[8]={
	{Sector::FL,208,21,QPoint(F_HIR,F_VIR),QPoint(F_HER-EDGE_SHIFT_F,F_VER-EDGE_SHIFT_F)},
	{Sector::FLC,180,30,QPoint(F_HIR,F_VIR),QPoint(F_HER,F_VER)},
	{Sector::FRC,150,30,QPoint(F_HIR,F_VIR),QPoint(F_HER,F_VER)},
	{Sector::FR, 131,21,QPoint(F_HIR,F_VIR),QPoint(F_HER-EDGE_SHIFT_F,F_VER-EDGE_SHIFT_F)},
	{Sector::RL, -54,30,QPoint(R_HIR,R_VIR),QPoint(R_HER-EDGE_SHIFT,R_VER-EDGE_SHIFT)},
	{Sector::RLC, -30,30,QPoint(R_HIR,R_VIR),QPoint(R_HER,R_VER)},
	{Sector::RRC, 0,30,QPoint(R_HIR,R_VIR),QPoint(R_HER,R_VER)},
	{Sector::RR, 24,30,QPoint(R_HIR,R_VIR),QPoint(R_HER-EDGE_SHIFT,R_VER-EDGE_SHIFT)}
	};

void Car::init(float ar=1.0)
{
	QSettings m_settings("Novorado","Sections");
	QFileInfo fi(m_settings.value("Parking Assistant","%PROGRAMFILES%\\Novorado\\Parking Assistant\\").toString()+"mycar.png");

	QPixmap pic;
	if(fi.exists()) pic=QPixmap(fi.absoluteFilePath());
	else pic=QPixmap(":audi.png");

	m_picsz = QSize(pic.size().width(),(int)(pic.size().height()*(m_ar=ar)));
	setPixmap(pic);
	setScaledContents(true);
	m_SpanAngle = 0; // will be initialized after first full scan
}

Car::Car(QWidget* p):QLabel(p)
{
	QSettings s("Novorado","ParkingAssistant");
	s.beginGroup("gui");
	init(s.value("AspectRatio",1.0).toDouble());
	for(int i=0;i<8;i++) m_secs[_sdata[i].pos]=&_sdata[i];
	s.endGroup();
}

Car::~Car()
{
}

void Car::placeInside(QSize _sz)
{
	QSize sz(_sz);
	sz.rwidth() *= HUTIL;
	sz.rheight() *= VUTIL;
	sz/=100;

	QSize newSize(m_picsz);
	newSize.scale(sz,Qt::KeepAspectRatio);

	QRect r(QPoint(0,0),newSize);
	r.moveCenter(QPoint(_sz.width()/2,_sz.height()/2));

	setGeometry(r);
}

void Car::sizeSector(Sector& s)
{
	QPoint
		fc(geometry().left()+geometry().width()*SHIFT_F/100,geometry().center().y()),
		rc(geometry().right()-geometry().width()*SHIFT_R/100,geometry().center().y());

	QRect pix;

	switch(s.role()){
		case Sector::FL: case Sector::FLC: case Sector::FRC: case Sector::FR:
			s.data()->get2point(fc,geometry(),pix,s); break;

		default: s.data()->get2point(rc,geometry(),pix,s); break;
		}

	s.setAngle(s.data()->angle * 16, s.data()->span * 16);
	s.setGeometry(pix);
}

void Sector::setAngle(double s,double e)
{
	m_startAngle=s;
	m_spanAngle=e;
}

Sector::Sector(DataSector* d,QWidget* w):QLabel(w)
{
	m_data=d;
	distancePerc=110;
	distanceToObstacle=255;
	/*
	switch(d->pos){
		case Sector::RR:
			distancePerc=lookup_radprc(50.);
			break;
		case Sector::RRC:
			distancePerc=lookup_radprc(100.);
			break;
		case Sector::RLC:
			distancePerc=lookup_radprc(150.);
			break;
		case Sector::RL:
			distancePerc=lookup_radprc(160.);
			break;
		case Sector::FR:
			distancePerc=lookup_radprc(170.);
			break;
		case Sector::FRC:
			distancePerc=lookup_radprc(180.);
			break;
		case Sector::FLC:
			distancePerc=lookup_radprc(190.);
			break;
		case Sector::FL:
			distancePerc=lookup_radprc(220.);
			break;
	}
	*/
}

Sector::~Sector()
{
}

void Sector::drawOnResize()
{
	QPixmap pic(size());
	pic.fill(Qt::transparent);

	QColor darkC("#262626"), radC("#323232");

	{
	QPainter p(&pic);
	p.setRenderHint(QPainter::Antialiasing);

	p.setPen(QPen(darkC,5));
	p.setBrush(radC);
	p.drawPie(ellipseRect,int(m_startAngle),int(m_spanAngle));

	if(!sensRect.isNull()){

//		p.setPen(QPen(Qt::yellow,3));
//		p.drawLine(ellipseRect.bottomLeft(),ellipseRect.topRight());
//		p.drawLine(ellipseRect.bottomRight(),ellipseRect.topLeft());
//		p.drawRect(sensRect);

		p.setCompositionMode(QPainter::CompositionMode_SourceIn);

		QRect r(sensRect.bottomLeft()+QPoint(3,-3),sensRect.topRight()+QPoint(-3,3));
		p.setPen(QPen(Qt::black,3));
		p.drawArc(r,int(m_startAngle),int(m_spanAngle));//,m_startAngle,m_spanAngle);

		p.setPen(QPen(Qt::red,8));
		p.drawArc(sensRect,int(m_startAngle),int(m_spanAngle));//,m_startAngle,m_spanAngle);
		}

	p.setCompositionMode(QPainter::CompositionMode_DestinationOut);
	p.setPen(QPen(Qt::yellow,4));
	p.setBrush(Qt::yellow);
	p.drawEllipse(shadowRect);
	}
	setPixmap(pic);

	update();
}

void Sector::resizeEvent(QResizeEvent* e)
{
	resize(e->size());
	drawOnResize();
}


void Sector::drawSensorCm(int dist)
{
	distancePerc = int(lookup_radprc(dist));

	distanceToObstacle=dist;

	// trick to regenerate the image of particular sensor
	drawOnResize();
}

void Sector::setBgColor(QColor c)
{
	QPalette p = palette();
	p.setBrush(QPalette::Background, c);
	setPalette(p);
}

void Widget::onTrayActivated(QSystemTrayIcon::ActivationReason)
{
	qApp->quit();
}

Widget::Widget(QWidget*):m_car(this)
{
	m_minDistanceR=m_minDistanceF=300;
	m_hideDelay = pa_hide_delay();

	m_front_dist = new  QLabel(this);
	m_rear_dist = new QLabel(this);

	//setMinimumSize(QSize(640,480));
	// set background
	QPalette p = palette();
	p.setBrush(QPalette::Background, Qt::black);
	setPalette(p);
	for(Sector::Position p=Sector::FL;p<=Sector::RR;p=static_cast<Sector::Position>(p+1)){
		m_sectors[p]=new Sector(&m_car.data(p),this);
		}

	m_tray=new QSystemTrayIcon(QIcon(":logo.gif"),this);
	m_tray->setToolTip("Novorado Audi Parking");
	connect(m_tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(onTrayActivated(QSystemTrayIcon::ActivationReason)));
	m_tray->show();

	initPark();

	connect(this,SIGNAL(drawSensor(Sector::Position,int)),this,SLOT(onDrawSensor(Sector::Position,int)),Qt::QueuedConnection);

	last_change=QTime::currentTime();
	//std::cout << "Last change: " << last_change.toString("hh:mm:ss.zzz").toAscii().constData() << std::endl;

	connect(&pollTimer,SIGNAL(timeout()),this,SLOT(poll()));
	pollTimer.start(IDLE_POLL);

	// overlay buttons
	QPushButton* butClose=new QPushButton(this);
	butClose->move(10,10);
	butClose->setFlat(true);
	butClose->setIcon(QPixmap(":/images/bsdark.png").copy(0,0,136,124));
	butClose->setIconSize(QSize(96,96));
	butClose->resize(96,96);

	connect(butClose,SIGNAL(clicked()),qApp,SLOT(quit()));

	QPushButton* butAR=new QPushButton(this);
	butAR->move(105,10);
	butAR->setFlat(true);
	butAR->setIcon(QPixmap(":/images/bsdark.png").copy(0,124,136,124));
	butAR->setIconSize(QSize(96,96));
	butAR->resize(96,96);

	connect(butAR,SIGNAL(clicked()),this,SLOT(switchAspectRatio()));
}

void Widget::switchAspectRatio()
{
	float newAre=m_car.aspectRatio()+0.1;
	if(newAre>2.5) newAre=0.2;
	m_car.init(newAre);

	{
	QSettings s("Novorado","ParkingAssistant");
	s.beginGroup("gui");
	s.setValue("AspectRatio",newAre);
	s.endGroup();
	}

	QSize s=size();
	resize(100,100);
	resize(s);
}

Widget::~Widget()
{
}

void Widget::loadHeads()
{
	QSettings m_settings("Novorado","ParkingAssistant");
	m_ha.clear();
	QStringList groups=m_settings.childGroups();

	for(int g=0;g<groups.size();g++){

		QString grp=groups[g];
		if(grp.left(6)=="Device"){

			m_settings.beginGroup(grp);

			// Get the device id
			grp=grp.right(grp.size()-6);
			Novorado::LineId deviceId=grp.toInt();

			// Look for all the logic head connected to that device
			QStringList keys=m_settings.allKeys();
			for(int k=0;k<keys.size();k++) {
				QString key=keys[k];

				// spot Novorado::HeadId<x>=disabled
				QString l=key.left(6);

				if(l=="Novorado::HeadId"){

					QString v=m_settings.value(key).toString();

					l=key.right(key.size()-6);
					Novorado::HeadId headId=l.toInt();

					// Check if value is "disabled"
					if(v=="disabled"){
						m_unassigned_heads[deviceId].insert(headId);
						}

					continue;
					}

				// it's probably an assignment than
				l = key.left(9);
				if(l!="LogicHead") continue;

				l=key.right(key.size()-9);

				Novorado::HeadLogicNumber num = l.toInt();

				if(num<=0 || num>8){
					std::stringstream ss;
					ss << "Load heads assignment: value " << num
						<< " is out of range";
					throw std::logic_error(ss.str());
					}

				Novorado::HeadId hid = m_settings.value(key).toInt();

				m_ha[deviceId][hid]=num;
				}

			m_settings.endGroup();
			}
		}
}

void Widget::initPark()
{
	hasDevVersion=false;
	loadHeads();

	// Tweak. Prevent from going into OSCO mode. We know the lines, it's fixed board for now
	if(!line_assignment.size()) {
		line_assignment[4]=Novorado::ParkingProtocol::CHALLENGER_26;
		line_assignment[5]=Novorado::ParkingProtocol::CHALLENGER_26;
		}

	// Pthread 1: Launch USB ThreadProc loop
	m_park=new Novorado::Parktronic(this,&line_assignment);

	QString libVersion = m_park->libraryVersion().c_str();
	QString devVersion = m_park->firmwareVersion().c_str();

	libVersion + " " + devVersion;
}

void Widget::onDrawSensor(Sector::Position p,int distCm)
{
	if(pollTimer.interval()!=WORK_POLL) {
		//std::cout << "SWITCHING TO FAST MODE" << std::endl;
		//std::cout << "Cur time: " << QTime::currentTime().toString("hh:mm:ss.zzz").toAscii().constData() << std::endl;
		pollTimer.start(WORK_POLL);
		}

	if(!isVisible() && !showTimer.isActive()) {
		showTimer.singleShot(pa_show_delay(),this,SLOT(showMaximized()));
		}

	if(hideTimer.isActive()) hideTimer.stop();
	last_change=QTime::currentTime();

	m_car.sizeSector(*m_sectors[p]);
	// Do not less to draw less than 30 cm
	//m_sectors[p]->drawSensorCm(std::max(distCm,30));
	m_sectors[p]->drawSensorCm(distCm);

	// Recalculate minimal distance
	m_minDistanceF=m_minDistanceR=255;
	for(std::map<Sector::Position,Sector*>::iterator i=m_sectors.begin();i!=m_sectors.end();i++){
		Sector& s = *i->second;
		switch(i->first){

			case Sector::FL:
			case Sector::FLC:
			case Sector::FRC:
			case Sector::FR:
				if(s.distanceToObstacle<m_minDistanceF) m_minDistanceF=s.distanceToObstacle;
				break;

			case Sector::RL:
			case Sector::RLC:
			case Sector::RRC:
			case Sector::RR:
				if(s.distanceToObstacle<m_minDistanceR) m_minDistanceR=s.distanceToObstacle;
				break;
			}
		}

	// Update labels with distances
	QString clr="red";
	if(m_minDistanceF<35) clr="red"; else clr="green";
	if(m_minDistanceF<200) m_front_dist->setText(QString("<font color='red'>%1</font>").arg(m_minDistanceF));
	else m_front_dist->setText("");

	if(m_minDistanceR<35) clr="red"; else clr="green";
	if(m_minDistanceR<200)  m_rear_dist->setText(QString("<font color='red'>%1</font>").arg(m_minDistanceR));
	else m_rear_dist->setText("");

	int dist=minDistance();
	//std::cout << "md is " << dist << std::endl;

	// Set beeper
	if(dist>100) { beep.stop(); }
	else if(dist>50) beep.setBeepInterval(1000);
	else if(dist>45) beep.setBeepInterval(600);
	else if(dist>40) beep.setBeepInterval(500);
	else if(dist>35) beep.setBeepInterval(500);
	else if(dist>30) beep.setBeepInterval(400);
	else beep.loop();
}

int Widget::minDistance()
{
	int rv=255;
	for(std::map<Sector::Position,Sector*>::iterator i=m_sectors.begin();i!=m_sectors.end();i++){
		if(rv>i->second->distanceToObstacle) rv=i->second->distanceToObstacle;
		}
	return rv;
}

#define	log std::cout

void Widget::onDistanceChange(LineId l,const Pair& p)
{
	// Code is executed by USB thread
	//log << "Line #" << l << " Pair(" << p.headId << ")=" << p.distanceCM << std::endl;

	std::map<Novorado::LineId, std::map<Novorado::HeadId,Novorado::HeadLogicNumber> >::iterator i=m_ha.find(l);
	if(i==m_ha.end()) return;
	std::map<Novorado::HeadId,Novorado::HeadLogicNumber>::iterator j=i->second.find(p.headId);
	if(j==i->second.end()) return;

	Novorado::HeadLogicNumber h=m_ha[l][p.headId];
	if(h<1||h>8) throw std::logic_error("Number out of bound in 'onDistanceChange(Novorado::LineId l, const Novorado::Pair& p)'");

	emit drawSensor(static_cast<Sector::Position>(h-1),p.distanceCM);
}

void Widget::poll()
{
	m_park->quant();
	//std::cout << "q" << std::flush;

	// Switch to slow mode & start hide countdown
	if(last_change.msecsTo(QTime::currentTime())>SWITCH_TO_SLOW_TIMEOUT && pollTimer.interval()==WORK_POLL){
		//std::cout << "SWITCHING TO SLOW MODE\nLast change: " << last_change.toString("hh:mm:ss.zzz").toAscii().constData() << std::endl;
		//std::cout << "Cur time: " << QTime::currentTime().toString("hh:mm:ss.zzz").toAscii().constData() << std::endl;
		pollTimer.start(IDLE_POLL);
		beep.stop();
		if(showTimer.isActive()) showTimer.stop();
		if(!hideTimer.isActive()&&!flags.neverHide) hideTimer.singleShot(pa_hide_delay(),this,SLOT(hide()));
		}
}

void Widget::onStatusChange(STATUS st, std::string s)
{
	log << "STATUS CODE #" << (int)(st) << " " << std::flush;
	switch(st){
		case Novorado::NRDP_DATA_FEED_STARTED:
			log << "Data feed started" << std::endl;
			break;

		case Novorado::NRDP_DATA_FEED_STOPPED:
			log << "Data feed stopped" << std::endl;
			break;

		case Novorado::NRDP_RECEIVING_DATA:
			log << "Connected, incoming data packet" << std::endl;
			break;

		case Novorado::NRDP_GOT_PING:
			log << "Gotaping" << std::endl;
			break;

		case Novorado::NRDP_FIRMWARE_UPDATE_MODE:
			log << "Firmware update mode: " << s << std::endl;
			break;

		case Novorado::NRDP_CONNECTING:
			log << "Connecting to the device: " << s << std::endl;
			break;

		case Novorado::NRDP_NO_PARKTRONIC_SIGNAL:
			log << "No signal: " << s << std::endl;
			break;

		case Novorado::NRDP_USB_CRITICAL_ERROR:
			log << "Critical error: " << s << std::endl;
			break;

		case Novorado::NRDP_FIRMWARE_UPDATED_OK:
			log << "Finished firmware update: " << s << std::endl;
			break;

		case Novorado::NRDP_FIRMWARE_UPDATE_FAILED:
			log << "Unable to update the firmware: " << s << std::endl;
			break;

		case Novorado::NRDP_DEVICE_EMULATION:
			log << "Emulation mode: " << s << std::endl;
			break;

		case Novorado::NRDP_GOT_DEVICE_VERSION:
			log << "Received device version: " << s << std::endl;
			devVersion=s;
			hasDevVersion=true;
			break;

		case Novorado::NRDP_UNKNOWN_PROTOCOL:
			log << "Protocol unrecognized: " << s << std::endl;
			break;

		default:
			log << "STATUS " << st << ", unkown" << s << std::endl;
			break;
		}
}

Sector::Position Sector::role()
{
	return data()->pos;
}

void Widget::dualMonitorCheck()
{
	// Check if there is dual monitor constraint

	QSettings m_settings("Novorado","ParkingAssistant");
	m_settings.beginGroup("gui");
	int monitor=m_settings.value("Monitor","1").toInt();
	m_settings.endGroup();

	// Check if we have more than one monitor
	QDesktopWidget *desktop = QApplication::desktop();

	int numMonitors=desktop->numScreens();
	if ( 1== numMonitors) {
		if(monitor!=1) std::cout << "Ignoring monitor setting " << monitor << " on a single screen system" << std::endl;
		return;
		} else {
			if(monitor<0 || monitor>=numMonitors){
				QMessageBox::warning(NULL,tr("Incorrect monitor number"),
					tr("Please set correct number in HKCU \\Software\\Novorado\\ParkingAssistant\\gui Monitor")
					);
				return;
				}

			QRect rect = desktop->screenGeometry(monitor);
			move(rect.topLeft());
			setWindowState(Qt::WindowFullScreen);
			}
}

int Widget::pa_hide_delay()
{
	QSettings m_settings("Novorado","ParkingAssistant");
	m_settings.beginGroup("gui");

	//m_showDelay=m_settings.value("ShowDelay","1").toInt();
	int t=m_settings.value("AutoHideDelay","3").toInt();

	m_settings.endGroup();
	return t*1000;
};

int Widget::pa_show_delay()
{
	QSettings m_settings("Novorado","ParkingAssistant");
	m_settings.beginGroup("gui");

	//m_showDelay=m_settings.value("ShowDelay","1").toInt();
	int t=m_settings.value("ShowDelay","2").toInt();

	m_settings.endGroup();
	return t*1000;
};


// Drawing
void Widget::resizeEvent(QResizeEvent* e)
{
	m_car.placeInside(e->size());

	for(std::map<Sector::Position,Sector*>::iterator i=m_sectors.begin();i!=m_sectors.end();i++)

		m_car.sizeSector(*i->second);

	int H = rect().height() * .2;

	QFont lf("Times",H/3);
	m_front_dist->setFont(lf);
	m_rear_dist->setFont(lf);

	m_front_dist->resize(QSize(2*H,H/3+10));
	m_rear_dist->resize(QSize(2*H,H/3+10));
	m_front_dist->move(QPoint(50,rect().bottom()-H-10));
	m_rear_dist->move(QPoint(rect().right()-H,rect().bottom()-H-10));
}

int main(int argc,char ** argv)
{
	Novorado::Version ver("audi",MAJOR_VERSION,MINOR_VERSION,PATCH_VERSION);
	ver.cmdline(argc,argv);

	// Check if there is a show flag
	std::list<std::string> largs;
	std::copy(argv,argv+(argc),std::back_inserter(largs));

	std::list<std::string>::iterator l;

	//bool flagConfig = (std::find(largs.begin(),largs.end(),"config")!=largs.end());

	struct Flags {
		bool show: 1;
		Flags(){
			show=0;
			}
		} flags;
	if((l=std::find(largs.begin(),largs.end(),"--show"))!=largs.end()){
		flags.show=1;
		}

	if((l=std::find(largs.begin(),largs.end(),"--checkupdate"))!=largs.end()){
		if(ver.hasUpdate(argv[0],"Parking Assistant")){
			// that will restart the process
			ver.update();
			}
		}

	int retVal=0;
	std::string retMessage;

	try {

	char const *x[] = {"parker", "parkerD", "audi", "audiD", "oodd", "ooddD","loganalyz","loganalyzD"};
	std::set<std::string> apps(x, x + sizeof(x) / sizeof(*x));

	if(Novorado::Process::count(apps)>1) throw std::logic_error(
		QObject::tr("Another copy of application is already running")
			.toUtf8().constData());

	QApplication audiApp(argc,argv);
	qRegisterMetaType<Sector::Position>("Sector::Position");
	Widget myW;
	myW.resize(QSize(800,600));
	myW.dualMonitorCheck();

	myW.setWindowTitle((std::string("audi ")+ver.getVersion()).c_str());
	myW.setWindowFlags(Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint | Qt::FramelessWindowHint );

	if(!flags.show) myW.hide(); else {
		// debug mode
		myW.neverHide();
		myW.showMaximized();
		//myW.show();
		}

	return audiApp.exec();
	}
	catch ( const std::logic_error& err) {
		retMessage = "Application failed with logic error: ";
		retMessage += err.what();
		std::cerr << retMessage << std::endl;
		retVal= 3;
		}

	catch ( ... ) {
		retMessage ="Application failed: unknown exception thrown" ;
		std::cerr << "Application failed: unknown exception thrown"  << std::endl;
		retVal=2;
		}

	if(retVal){
		QApplication errorApp(argc,argv);
		// Error happened
		QMessageBox::critical(NULL, "Parktronic Application",
                                retMessage.c_str());
		}

	return retVal;
}
