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
//
// $Id: parkSetup.cpp,v 1.43 2012/03/26 16:36:31 dmi Exp $
//

#include	"parkSetup.h"
#include	<QTabWidget>
#include	<QVBoxLayout>
#include	<QHBoxLayout>
#include	<QPushButton>
#include	<QPainter>
#include	<QPaintEvent>
#include	<sstream>
#include	<QLabel>
#include	<QBitmap>
#include	<iostream>
#include	"ui_SoundsTab.h"
#include	"ui_GUITab.h"
#include	"ui_WebCamTab.h"
#include	"ui_FirmwareUpdateTab.h"
#include	"ui_DiagnosticsTab.h"
#include	"ui_thermoTab.h"
#include	"parkAssist.h"
#include	<QFileDialog>
#include	<QSpinBox>
#include	<QMessageBox>
#include	<usbDevice.h>
#include	"PASettings.h"
#include	<QFtp>
#include	<QProgressDialog>
#include	<QSpinBox>
#include	<buzzertable.h>
#if	defined	SYSTEM_IS_WIN32
#include	<windows.h>
#endif

using namespace Novorado::Devices;

std::string Firmware::recentVersion()
{
	if(!m_most_recent.length()) versions();

	return m_most_recent;
}

std::map<std::string,std::string> Firmware::versions()
{
	//std::cout << "Getting versions of firmware" << std::endl;

	// Read file list from resource
	QString name=":";
	name +="firmware";
	name += ".listqz";
	QFile f(name);
	f.open(QIODevice::ReadOnly);
	QByteArray in=f.readAll();
	f.close();

	// Decompress the list
	QByteArray txt = qUncompress(in);

	if(txt.size()==0){
		std::cout << "Unable to decompress logic listing" << std::endl;
		throw std::logic_error("Unable to decompress logic listing");
		}

	QString theLines = txt.constData();

	QStringList sl = theLines.split("\n");

	m_list.clear();

	for(int i=0;i<sl.size();i++){
		QString fn = sl[i];

		QStringList toVer = fn.split("_");

		// Ignore tha thing
		if(toVer.size()<4) continue;

		QStringList a=toVer[3].split(".");

		QString ver=toVer[1] + "." + toVer[2] + "." + a[0];

		// Call the default constructor; empty array will mean firmware is not yet loaded
		m_most_recent = ver.toAscii().constData();

		//std::cout << (i+1) << ") " << fn.toAscii().constData() << "\t" << m_most_recent << std::endl;

		m_list[m_most_recent] = fn.toAscii().constData();
		}

	return m_list;
}

const char** Firmware::loadFirmware(const std::string& version)
{
	// Make sure list is loaded
	if(!m_list.size()) versions();

	// Load data
	QString name=":";
	name += m_list[version].c_str();
	/*
	std::cout
		<< "Requested verion '" << version << "', Reading '"
		<< name.toAscii().constData() << "'" << std::endl;
	*/

	QFile f(name);
	f.open(QIODevice::ReadOnly);
	QByteArray in=f.readAll();
	f.close();

	// Decompress the list
	QByteArray txt = qUncompress(in);

	if(txt.size()==0){
		std::stringstream r;
		r << "Unable to decompress version '" << version << "' (file '" << name.toAscii().constData() << "')";
		std::cout << r.str() << std::endl;
		throw std::logic_error(r.str());
		}

	QString theLines = txt.constData();

	QStringList sl = theLines.split("\n");

	m_data.resize(sl.size());
	m_nullTerminatedStrPointers.resize(sl.size()+1);
	m_nullTerminatedStrPointers[sl.size()]=NULL;

	for(int i=0;i<sl.size();i++){
		QString l = sl[i];
		// Skip empty strings
		if(l.length()==0) continue;
		m_data[i]=l.toAscii().constData();
		m_nullTerminatedStrPointers[i]=m_data[i].c_str();
		//std::cout << "'" << m_data[i] << "'" << std::endl;
		}

	return &m_nullTerminatedStrPointers[0];
}


using namespace Novorado::Parking;

#define	HM_MARGIN 5

#define	HM_W	(15+HM_MARGIN)
#define	HM_H	(15+HM_MARGIN)

SetupDialog* SetupDialog::pt_setuDialog = NULL;

SetupDialog::~SetupDialog()
{
	// Unlock the singleton
	SetupDialog::setVisible(NULL);
}

void SetupDialog::setVisible(SetupDialog* p){

	if(pt_setuDialog&&p!=NULL) throw Novorado::LogicErrorException("Attempting to allocate second instance of setup dialog");
	pt_setuDialog=p;
	}

SetupDialog* SetupDialog::visible(){
	return pt_setuDialog;
	}

void HeadMark::setLocation(const QPoint& p)
{
	m_loc=p;
	move(m_loc-QPoint(HM_W,HM_H));
	resize(HM_W*2,HM_H*2);
}

void Car::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

	QRect r = rect();
	pixSize=carImage.size();
	pixSize.scale(rect().size(),Qt::KeepAspectRatio);
	m_rect=QRect(r.topLeft(),pixSize);

	int H=m_rect.height();
	int W=m_rect.width();

	HeadsTab* owner = (HeadsTab*)(parent());

	for(int headNum=1;headNum<=8;headNum++){

		HeadMark& m = m_marks[headNum];
		m.setNumber(headNum);
		connect(
			&m, SIGNAL(userSelected(HeadMark*)),
			owner,SLOT(onHeadMarkSelected(HeadMark*))
			);

		connect(
			&m, SIGNAL(userCleared(HeadMark*)),
			owner,SLOT(onHeadMarkCleared(HeadMark*))
			);

		m.setParent(this);

		int x_id=headNum<=4?headNum:headNum-4;

		int Y = (int)(m_rect.top()+headNum<=4?H/16.:H*15./16.);
		int X = m_rect.left()+x_id*(W/5);

		m.setLocation(QPoint(X,Y));
		}
}

QPoint HeadMark::location() const
{
	return m_loc;
}

Novorado::HeadLogicNumber HeadMark::number() const
{
	return m_num;
}

void HeadMark::setNumber(Novorado::HeadLogicNumber i)
{
	m_num=i;
}

void HeadMark::paintEvent(QPaintEvent* /*e*/)
{
	QPainter p(this);
	p.setRenderHint(QPainter::HighQualityAntialiasing,true);

	p.setBrush(isLinked()?Qt::red:Qt::green);
	p.setPen(Qt::yellow);

	std::stringstream txt;
	txt << (int)(m_num);

	QRect m_rect=rect();

	m_rect.setLeft(m_rect.left()+HM_MARGIN);
	m_rect.setTop(m_rect.top()+HM_MARGIN);

	m_rect.setRight(m_rect.right()-HM_MARGIN);
	m_rect.setBottom(m_rect.bottom()-HM_MARGIN);

	p.drawEllipse(m_rect);

	p.setPen(Qt::white);
	p.drawText(m_rect,Qt::AlignHCenter|Qt::AlignVCenter,txt.str().c_str());
}

Car::Car(QWidget* p):QWidget(p)
{
	carImage=QImage(":images/PARKTRONIC_EMPTY.jpg").copy(QRect(QPoint(100,65),QSize(150,320)));
	setMinimumWidth(70);//TODO: was 200
}

void Car::paintEvent(QPaintEvent* e)
{
	QPainter p(this);

	p.setClipRegion(e->region());

	p.drawImage(m_rect,carImage);
}

DeviceHead* Device::markedHead()
{
	DeviceHead* rv = NULL;

	for(std::map<Novorado::HeadId,DeviceHead>::iterator i=m_heads.begin();
		i!=m_heads.end();
		i++){

		rv = &i->second;

		if(rv->isMarked()) return rv;
		}

	return rv;
}

void HeadsTab::onHeadMarkCleared(HeadMark* hm)
{
	if(hm->isLinked()){
		DeviceHead* _h = hm->head();
		removeHeadReferences(_h);
		hm->setHead(NULL);
		updateTopLevel();
		saveSettings();
		}
}

void HeadsTab::onHeadMarkSelected(HeadMark* hm)
{
	DeviceHead* dh=NULL;

	// Look for active device
	if(m_selectedDevice) {
		// than look for marked device head
		dh =m_selectedDevice->markedHead();
		}

	// clean otherwise
	removeHeadReferences(dh);
	hm->setHead(dh);

	updateTopLevel();

	saveSettings();
}

void HeadsTab::removeHeadReferences(DeviceHead* dh)
{
	for(std::map<Novorado::HeadId,HeadMark>::iterator i=m_car->m_marks.begin();
		i!=m_car->m_marks.end(); i++){
		// remove all possible duplicate references
		if(i->second.head()==dh) i->second.setHead(NULL);
		}
}

void HeadMark::mousePressEvent(QMouseEvent*)
{
	emit userSelected(this);
}

void HeadMark::mouseDoubleClickEvent(QMouseEvent*)
{
	emit userCleared(this);
}

QRect Car::freeZone()
{
	QRect r=geometry();
	r.setRight(r.left()+pixSize.width());
	r.setBottom(r.top()+pixSize.height());
	r.setTop(static_cast<int>(r.top()+HM_H*2.5));
	r.setBottom(static_cast<int>(r.bottom()-HM_H*2.5));

	return r;
}

void Device::deselect()
{
	label->setChecked(false);

	for(std::map<Novorado::HeadId,DeviceHead>::iterator i=m_heads.begin();
		i!=m_heads.end();
		i++){

		i->second.button().setEnabled(false);
		i->second.clearMark();
		}
}

void HeadsTab::onUserSelected(Device* d)
{
	m_selectedDevice = d;

	for(std::map<Novorado::LineId,Device>::iterator i=m_devices.begin();
		i!=m_devices.end(); i++) {

		if(&i->second==d) continue;

		i->second.deselect();
		}

	updateTopLevel();
}

void HeadsTab::addHead(Novorado::LineId lid,Novorado::HeadId hid)
{
	if(m_devices.find(lid)==m_devices.end()){
		Device& d = m_devices[lid];
		d.setId(lid);
		d.setParent(this);
		d.setOwner(this);

		layout()->addWidget(&d);

		connect(
			&d,SIGNAL(userSelected(Device*)),
			this,SLOT(onUserSelected(Device*))
			);

		topLabel->raise();
		}

	if(!m_devices[lid].getDeviceHead(hid)) {
		m_devices[lid].addHead(hid);
	}

	// Prevent flooding
	#if	0
		else {
		std::cout << "Ignoring attempt to add " << lid << ":" << hid << " which is already registered"
		<< std::endl;
		}
	#endif
}

void Device::clearMarks()
{
	for(std::map<Novorado::HeadId,DeviceHead>::iterator i=m_heads.begin();
		i!=m_heads.end();
		i++)

		i->second.clearMark();
}

void DeviceHead::onButtonPressed()
{
	HeadsTab& hw = *(HeadsTab*)(papa->parent());

	HeadMark* mark = hw.getHeadMark(this);

	if(mark) mark->setHead(this);

	owner()->clearMarks();

	m_mark=true;

	hw.updateTopLevel();
}

void HeadsTab::updateTopLevel()
{
	Device* dev = selectedDevice();

	if(!dev) {
		topLabel->setGeometry(QRect(QPoint(0,0),QSize(0,0)));
		return;
		}

	QRect br = dev->buttonRect();

	QRect g;
	g.setLeft(m_car->geometry().right());
	g.setTop(br.top());
	g.setRight(br.left()+15);
	g.setBottom(br.bottom());

	// Car pixmap
	QRect cp = m_car->freeZone();

	g.setBottom(std::min(cp.bottom(),g.bottom()));
	g.setLeft(cp.left());

	topLabel->setGeometry(g);

	topLabel->update();

	if(m_car) m_car->update();
}


void DeviceHead::setSpinBox(QSpinBox& sb)
{
	m_sb = &sb;
	sb.setEnabled(false); // it will only be enabled when received data from firmware
}

QSpinBox& DeviceHead::spinBox()
{
	if(!m_sb) throw Novorado::LogicErrorException("DeviceHead::spinBox() - spinBox is not set");
	return *m_sb;
}

void DeviceHead::setButton(QPushButton& b)
{
	m_button = &b;

	connect(
		m_button, SIGNAL(pressed()),
		this,SLOT(onButtonPressed())
		);
}

QPushButton& DeviceHead::button()
{
	if(!m_button) throw Novorado::LogicErrorException("DeviceHead::button() - button is not set");
	return *m_button;
}


void DeviceHead::onShiftChanged(int s)
{
	int ownerId =owner()->id();

	if(ownerId<4 || ownerId>5) throw Novorado::LogicErrorException("Unsupported line id for heads shift");

	Device* o1=owner();
	if(!o1) throw Novorado::LogicErrorException("Device is not set");
	HeadsTab* o2=o1->owner();
	if(!o2) throw Novorado::LogicErrorException("HeadsTab is not set");
	Parktronic& p=o2->parktronic();
	if(!&p) throw Novorado::LogicErrorException("Parktronic is not set");

	p.saveShift((ownerId-4)*4 + id(),s);

	QSettings fs("Novorado","ParkingAssistant");
	fs.beginGroup("Firmware");
	fs.setValue(QString("transponder%1").arg((ownerId-4)*4 + id()),s);
	fs.endGroup();
}

void Device::addHead(Novorado::HeadId hid)
{
	if(m_heads.find(hid)!=m_heads.end()) throw Novorado::LogicErrorException("Device::addHead(Novorado::HeadId hid) - adding existing head");

	DeviceHead& h = m_heads[hid];
	h.setOwner(this);
	h.setId(hid);
//	h.setParent(this);
	/*
	h.setButton(*new QPushButton(t.str().c_str(),this));
	h.button().setEnabled(false);
	h.button().setCheckable(true);
	((QVBoxLayout*)layout())->insertWidget(layout()->count()-1,&h.button());
	*/

	QHBoxLayout* horiz1=new QHBoxLayout;

	h.setButton(*new QPushButton(QString('A'+(char)(hid)),this));
	h.button().setEnabled(false);
	h.button().setCheckable(true);

	QSpinBox* cb=new QSpinBox(this);
	h.setSpinBox(*cb);

	connect(
		cb,SIGNAL(valueChanged(int)),
		&h,SLOT(onShiftChanged(int))
		);

	cb->setMinimum(0);
	cb->setMaximum(50);
	cb->setValue(0);
	cb->setMaximumWidth(55);

	horiz1->addWidget(&h.button());
	horiz1->addWidget(cb);

	((QVBoxLayout*)layout())->insertLayout(layout()->count()-1,horiz1);

}

bool DeviceHead::isMarked() const
{
	return m_mark;
}

void DeviceHead::clearMark()
{
	m_mark=false;
}


DeviceHead::DeviceHead(const DeviceHead& d):QObject(NULL)
{
	m_headId=d.m_headId;
	m_button=NULL;
	papa=NULL;
	m_mark = false;
}

Device::Device(const Device& d):QWidget(NULL)
{
	m_owner=NULL;
	deviceId=d.deviceId;
	m_heads=d.m_heads;
}

Novorado::HeadId DeviceHead::id() const
{
	return m_headId;
}

void DeviceHead::setId(Novorado::HeadId i)
{
	m_headId=i;
}

void Device::labelClicked()
{
	for(std::map<Novorado::HeadId,DeviceHead>::iterator i=m_heads.begin();
		i!=m_heads.end();
		i++){

		i->second.button().setEnabled(true);
		}

	emit userSelected(this);
}

void Device::setId(Novorado::LineId i)
{
	deviceId=i;
	if(!layout()) {
		QVBoxLayout* lay = new QVBoxLayout(this);
		lay->setAlignment(Qt::AlignJustify);
		QString lbl=tr("Device %1").arg(i);
		if(i==4) lbl="R"; else if(i==5) lbl="F";

		label = new QPushButton(lbl,this);

		label->setCheckable(true);
		lay->addWidget(label);
		lay->addStretch(-1); // to the end
		connect(
			label,SIGNAL(clicked()),
			this,SLOT(labelClicked())
			);
		return;
		}

	if(!label) throw Novorado::LogicErrorException("Device::setId(Novorado::LineId) - label is not set");
	std::stringstream t;
	t << "Device " << i;
	label->setText(t.str().c_str());
}

QPoint DeviceHead::center() const
{
	if(m_button) return m_button->mapToGlobal(m_button->rect().center());

	return QPoint(0,0);
}
Novorado::LineId Device::id() const
{
	return deviceId;
}

Device::Device()
{
	m_owner=NULL;
}

DeviceHead::DeviceHead()
{
	m_button=NULL;
	papa = NULL;
	m_mark = false;
}

void DeviceHead::setOwner(Device* p)
{
	papa=p;
}

std::set<DeviceHead*> Device::getAllHeads()
{
	std::map<Novorado::HeadId,DeviceHead>::iterator i=m_heads.begin();

	std::set<DeviceHead*> rv;

	for(;i!=m_heads.end();i++) rv.insert(&i->second);

	return rv;
}

DeviceHead* Device::getDeviceHead(Novorado::HeadId headId)
{
	std::map<Novorado::HeadId,DeviceHead>::iterator i=m_heads.find(headId);

	if(i==m_heads.end()) return NULL;

	return &i->second;
}

DeviceHead* HeadsTab::getDeviceHead(Novorado::LineId deviceId,Novorado::HeadId headId)
{
	std::map<Novorado::LineId,Device>::iterator i=m_devices.find(deviceId);

	if(i==m_devices.end()) return NULL;

	Device& dev = i->second;

	return dev.getDeviceHead(headId);
}

void HeadsTab::loadSettings()
{
	std::map<LineId, std::map<Novorado::HeadId,Novorado::HeadLogicNumber> > h = m_settings->heads();

	for(std::map<LineId, std::map<Novorado::HeadId,Novorado::HeadLogicNumber> >::iterator
			i=h.begin();i!=h.end();i++){

		Novorado::LineId deviceId=i->first;

		std::map<Novorado::HeadId,Novorado::HeadLogicNumber>& a = i->second;

		for(std::map<Novorado::HeadId,Novorado::HeadLogicNumber>::iterator j=a.begin();j!=a.end();j++){

			Novorado::HeadLogicNumber logic_head = j->second;

			Novorado::HeadId hid = j->first;

			DeviceHead* dh = getDeviceHead(deviceId,hid);

			if(!dh) {
				addHead(deviceId,hid);
				dh = getDeviceHead(deviceId,hid);
				if(!dh) throw Novorado::LogicErrorException("HeadsTab::loadSettings() - unable to get device head");
				}

			std::map<Novorado::HeadId,HeadMark>::iterator it=
				m_car->m_marks.find(logic_head);

			if(it!=m_car->m_marks.end()) throw Novorado::LogicErrorException("HeadsTab::loadSettings() - mark already set");

			removeHeadReferences(dh);
			m_car->m_marks[logic_head].setHead(dh);
			}
		}

	// Init wild heads
	std::map<LineId, std::set<HeadId> > wh = m_settings->getUnassignedHeads();

	for(std::map<LineId, std::set<HeadId> >::iterator i=wh.begin();i!=wh.end();i++){
		std::set<HeadId>& h = i->second;

		LineId deviceId = i->first;

		for(std::set<HeadId>::iterator j=h.begin();j!=h.end();j++){

			HeadId hid = *j;

			// Create disconnected head
			DeviceHead* dh = getDeviceHead(deviceId,hid);

			if(!dh) {
				addHead(deviceId,hid);
				dh = getDeviceHead(deviceId,hid);
				if(!dh) throw Novorado::LogicErrorException("HeadsTab::loadSettings() - unable to get device head");
				}

			// Clear existing head now
			removeHeadReferences(dh);
			}
		}
}

HeadsTab::~HeadsTab()
{
//	saveSettings();
}

void HeadsTab::saveSettings()
{
	std::map<LineId, ParkingProtocol::PROTOCOLS> la = m_settings->getLineAssignment();

	// Clean the output data - file, memory
	m_settings->removeDevices();

	if(!m_car) return;

	std::map<Novorado::HeadId,HeadMark>::iterator i=m_car->m_marks.begin();

	std::map<LineId, std::set<HeadId> > blockList, usedList;

	for(;i!=m_car->m_marks.end();i++) {

		HeadMark& hm = i->second;

		if(!hm.isLinked()) continue;

		m_settings->assignLogicHead(hm.deviceId(),hm.headId(),hm.number());

		usedList[hm.deviceId()].insert(hm.headId());
		}

	// If user conseously decided not to link the device head (for example,
	// due to the head malfunction), than we need to enter it into the block
	// list

	for(std::map<Novorado::LineId,Device>::iterator j=m_devices.begin();
		j!=m_devices.end();j++){

		std::set<DeviceHead*> s=j->second.getAllHeads();

		for(std::set<DeviceHead*>::iterator l=s.begin();l!=s.end();l++)

			blockList[j->first].insert((*l)->id());

		// All heads for the device
		std::set<HeadId>& shi=usedList[j->first];
		for(std::set<HeadId>::iterator g=shi.begin();g!=shi.end();g++)
			blockList[j->first].erase(*g);
		}

	m_settings->saveDevices(blockList);

	// Re-cover preserved line assignment and add the new one
	std::map<LineId, std::map<Novorado::HeadId,Novorado::HeadLogicNumber> > h=m_settings->heads();

	for(std::map<LineId, std::map<Novorado::HeadId,Novorado::HeadLogicNumber> >::iterator j=h.begin();
		j!=h.end();j++){

		LineId lineId = j->first;

		// The only line assignment we can have right now is Challenger 26
		la[lineId]=ParkingProtocol::CHALLENGER_26;
		}

	m_settings->setLineAssignment(la);
}

void HeadMark::setHead(DeviceHead* h)
{
	m_head=h;
}

DeviceHead* HeadMark::head()
{
	return m_head;
}

Device* HeadsTab::selectedDevice()
{
	return m_selectedDevice;
}

HeadMark* HeadsTab::getHeadMark(DeviceHead* dh)
{
	HeadMark* hm = NULL;

	if(m_car) {
		for(std::map<Novorado::HeadId,HeadMark>::iterator i=m_car->m_marks.begin();
			i!=m_car->m_marks.end(); i++){

			if(i->second.isLinked()) {
				if(i->second.head()==dh) return &i->second;
				} else if(!hm) {
					hm=&i->second;
					}
			}
		}

	return hm;
}

void CoverLabel::drawLink(QPainter& p, HeadMark& h)
{
	if(!h.isLinked()) throw Novorado::LogicErrorException("CoverLabel::drawLink - head mark is not linked");

	DeviceHead& j = *h.head();

	QRect hmgeo= h.rect();

	QPoint
		c1 = h.mapToGlobal(hmgeo.center()),
		c2 = j.center();

	c1 = mapFromGlobal(c1);
	c2 = mapFromGlobal(c2);

	QPoint c(c1.x(),c2.y());

// 	std::cout << "c1=" << c1 << " c2=" << c2 << " c=" << c << std::endl;

	QPen p1(Qt::gray,10);
	p1.setCapStyle(Qt::RoundCap);

	QPen p2(j.isMarked()?Qt::yellow:Qt::red,9);
	p2.setCapStyle(Qt::RoundCap);

	QPolygon poly;
	poly << c2 << c << c1;

	p.setPen(p1);
	p.drawPolyline(poly);

	p.setPen(p2);
	p.drawPolyline(poly);
}

HeadMark::HeadMark(const HeadMark& hm):QWidget(NULL)
{
	m_flag_active = hm.m_flag_active;
	m_loc = hm.m_loc;
	m_head=NULL;
}

bool HeadMark::isLinked() {
	return m_head!=NULL;
	}

Novorado::LineId HeadMark::deviceId() {

	if(m_head==NULL) return -1;

	return m_head->owner()->id();
	}

Novorado::HeadId HeadMark::headId() {
	return m_head==NULL?-1:m_head->id();
	}

HeadMark::HeadMark()
{
	m_head=NULL;
}

Novorado::Parktronic& SetupDialog::parktronic()
{
	return ((ParkAssist*)(parent()))->parktronic();
}

QTranslator& SetupDialog::translator()
{
	return ((ParkAssist*)(parent()))->translator();
}

void FirmwareUpdateTab::syncDevice()
{
	QString libVersion = m_parktronic.libraryVersion().c_str();

	QString devVersion = m_parktronic.firmwareVersion().c_str();

	QLabel* l = qFindChild<QLabel*>(this, "label_FirmwareVersion");
	if(!devVersion.length()) {
		l->setText("<font color=red>Not connected</font>");
		qFindChild<QPushButton*>(this,"pushButton_Ping")->setEnabled(false);
		qFindChild<QPushButton*>(this,"pushButton_Reboot")->setEnabled(false);
		qFindChild<QPushButton*>(this,"pushButton_Upload")->setEnabled(false);
		qFindChild<QComboBox*>(this,"comboBox")->setEnabled(false);

		} else {
			l->setText(QString("<font color=green>Library: %1 Firmware: %2</font>")
				.arg(libVersion)
				.arg(devVersion));
			qFindChild<QPushButton*>(this,"pushButton_Ping")->setEnabled(true);
			}
}

HeadsTab& SetupDialog::headsTab()
{
	if(!heads) throw Novorado::LogicErrorException("SetupDialog::headsTab() - heads unset");
	return *heads;
}

void SetupDialog::cleanHeadsSettings(Settings* m_settings)
{
	m_settings->removeDevices();
}

void SetupDialog::saveSettings()
{
	sounds->saveSettings();
	gui->saveSettings();
	heads->saveSettings();
	webcams->saveSettings();
	diagnostics->saveSettings();
}

void SetupDialog::saveAndAccept()
{
	saveSettings();
	accept();
}

SetupDialog::SetupDialog(
	std::set<LineId>& m_unknown_devices,
	std::map<LineId, std::set<HeadId> >& m_unknown_heads,
	QWidget* p,
	Settings* s):QDialog(p),m_settings_p(s)
{
	SetupDialog::setVisible(this);

	int W=s->getConfigWidth(),H=s->getConfigHeight();
	resize(W,H);
	//std::cout << "Set size " <<  W << "x" << H << std::endl;

	setFont(BIG_FONT);

	QVBoxLayout* vlay = new QVBoxLayout(this);

	QTabWidget* tabs = new QTabWidget(this);

	vlay->addWidget(tabs);

	sounds=new SoundsTab(parktronic(),tabs,s);
	gui=new GUITab(tabs,translator(),s);
	heads=new HeadsTab(parktronic(),tabs,s);
	webcams=new WebCamTab(tabs,s);
	diagnostics = new DiagnosticsTab(tabs,s);
	firmupdater = new FirmwareUpdateTab(tabs,parktronic());
	thermals=new ThermoTab(tabs,parktronic());

	// That will remove the records from the config file (Win32 - registry) as well
	cleanHeadsSettings(s);

	connect(
		p,SIGNAL(gotHeadsShift(std::vector<unsigned char>)),
		heads,SLOT(onHeadsShift(std::vector<unsigned char>))
		);

	connect(
		gui,SIGNAL(clearSettingsAndReboot()),
		this,SLOT(onClearSettingsAndReboot()),
		Qt::QueuedConnection
		);

	connect(
		p,SIGNAL(ping()),
		firmupdater,SLOT(onPing()),
		Qt::QueuedConnection
		);

	connect(
		p,SIGNAL(firmwareUpdateProgress(const QString&)),
		firmupdater,SLOT(progress(const QString&)),
		Qt::QueuedConnection
		);

	connect(
		p,SIGNAL(firmwareUpdateFinished(bool)),
		firmupdater,SLOT(onFirmwareUpdateFinished(bool)),
		Qt::QueuedConnection
		);

	connect(
		p,SIGNAL(headConnected(LineId,HeadId)),
		heads,SLOT(addHead(LineId,HeadId)),
		Qt::QueuedConnection
		);

	// Cause chain of events, that will eventually signal
	// buzzer parameters into SoundsTab
	connect(
		p, SIGNAL(gotBuzzerFL(unsigned char,unsigned char, int)),
		sounds,SLOT(buzzerFL(unsigned char, unsigned char, int))
		);

	connect(
		p,SIGNAL(gotHeadsShift(std::vector<unsigned char>)),
		heads,SLOT(onHeadsShift(std::vector<unsigned char>))
		);

	parktronic().readBuzzer();
	parktronic().readHeadsShift();

	connect(
		qFindChild<QSlider*>(this,"horizontalSlider_Frequency"),SIGNAL(valueChanged(int)),
		sounds,SLOT(onFrequencyChanged(int))
		);

	connect(
		qFindChild<QSlider*>(this,"horizontalSlider_Loudness"),SIGNAL(valueChanged(int)),
		sounds,SLOT(onLoudnessChanged(int))
		);

	tabs->addTab(heads,	QString(" ")+QDialog::tr("Heads")+" ");
	tabs->addTab(sounds,	QString(" ")+QDialog::tr("Sounds")+" ");
	tabs->addTab(gui,	QString(" ")+QDialog::tr("GUI")+" ");
	tabs->addTab(webcams,	QString(" ")+QDialog::tr("Cams")+" ");
	tabs->addTab(thermals,	QString(" ")+QDialog::tr("Temperature")+" ");
	tabs->addTab(firmupdater,QString(" ")+QDialog::tr("Firmware")+" ");
	tabs->addTab(diagnostics,QString(" ")+QDialog::tr("Diagnostics")+" ");

	QHBoxLayout* buttonLay = new QHBoxLayout();

	vlay->addLayout(buttonLay);

	QPushButton* cancelButt= new QPushButton(QPushButton::tr("Continue"),this);
	buttonLay->addWidget(cancelButt);

	connect(
		cancelButt,SIGNAL(pressed()),
		this,SLOT(saveAndAccept()),
		Qt::QueuedConnection
		);

	QPushButton* exitButt= new QPushButton(QPushButton::tr("Drop"),this);
	exitButt->setMaximumWidth(90);
	buttonLay->addWidget(exitButt);

	connect(
		exitButt,SIGNAL(pressed()),
		this,SLOT(reject()),
		Qt::QueuedConnection
		);

	// Add heads previously discovered
	m_unknown_devices.clear();

	for(std::map<LineId, std::set<HeadId> >::iterator i=m_unknown_heads.begin();
		i!=m_unknown_heads.end(); i++){
		std::set<HeadId>& k= i->second;
		for(std::set<HeadId>::iterator j=k.begin();j!=k.end();j++){
			// Here we'll add the heads
			headsTab().addHead(i->first,*j);
			}
		}

	m_unknown_heads.clear();
	resize(W,H);
}

void SetupDialog::onClearSettingsAndReboot()
{
	std::cout << "Clearing settings and rebooting the app" << std::endl;
	m_settings_p->clear();
	accept();
	//	qApp->exit(APP_RELOAD_CODE);
}

void CoverLabel::setMarks(std::map<Novorado::HeadId,HeadMark>* mm)
{
	m_marks=mm;
}

CoverLabel::CoverLabel(QWidget* w):QWidget(w)
{
	m_marks=NULL;
}

Device* DeviceHead::owner()
{
	return papa;
}

void CoverLabel::paintEvent(QPaintEvent* /*e*/)
{
	if(!m_marks) return;

	HeadsTab* ht = (HeadsTab*)(parent());

	QPainter p(this);
	for(std::map<Novorado::HeadId,HeadMark>::iterator i=m_marks->begin();
		i!=m_marks->end(); i++){
		HeadMark& h =i->second;
		if(h.isLinked() && h.head()->owner()==ht->selectedDevice()) {
			drawLink(p,h);
			}
		}

	#if	0
	QPen redp(Qt::red,4);
	p.setPen(redp);
	p.drawRect(rect());
	#endif
}

QRect Device::buttonRect()
{
	QRect r=geometry();

	r.setTop(label->geometry().bottom());

	return r;
}

void HeadsTab::resizeEvent(QResizeEvent* e)
{
	QWidget::resizeEvent(e);

	updateTopLevel();
}

void HeadsTab::onHeadsShift(std::vector<unsigned char> v)
{
	// Line 4
	std::map<Novorado::LineId,Device>::iterator i=m_devices.find(4);
	if(i!=m_devices.end()){
		for(int h=0;h<4;h++) {
			// Read shift
			i->second.getDeviceHead(h)->spinBox().setValue(v[h]);
			i->second.getDeviceHead(h)->spinBox().setEnabled(true);
			}
		}

	// Line 5
	i=m_devices.find(5);
	if(i!=m_devices.end()){
		for(int h=0;h<4;h++) {
			// Read shift
			DeviceHead* dh=i->second.getDeviceHead(h);
			// When just started reading, that may not be avaialable yet
			if(dh) {
				dh->spinBox().setValue(v[h+4]);
				dh->spinBox().setEnabled(true);
			} else std::cout << "Warning: Device head #" << h << " not yet loaded" << std::endl;
			}
		}
}

HeadsTab::HeadsTab(Parktronic& prk,QWidget* p,Settings* s) : QWidget(p), TabSettings(s),m_parktronic(prk)
{
	m_selectedDevice=NULL;

	QHBoxLayout* lay=new QHBoxLayout(this);

	m_car = new Car(this);

	lay->addWidget(m_car);

	topLabel = new CoverLabel(this);
	topLabel->setMarks(&m_car->m_marks);

	updateTopLevel();

	loadSettings();
}
void SoundsTab::onFrequencyChanged(int x)
{
	qFindChild<QSlider*>(this,"horizontalSlider_Loudness")->setValue(50);

	QSettings fs("Novorado","ParkingAssistant");
	fs.beginGroup("Firmware");
	fs.setValue("frequency",x);
	fs.endGroup();

	onLoudnessChanged(x);
}

void SoundsTab::onLoudnessChanged(int)
{
	unsigned char
		f=qFindChild<QSlider*>(this,"horizontalSlider_Frequency")->value(),
		v=qFindChild<QSlider*>(this,"horizontalSlider_Loudness")->value();

	m_parktronic.configureBuzzer(f,v);

	BuzzerFreqs bf;
	const BuzzerTable& b = bf.getFrequency(f);

	qFindChild<QLabel*>(this,"label_Frequency")->setText(QString("%1").arg(b.Frequency));
	qFindChild<QLabel*>(this,"label_Loudness")->setText(QString("%1").arg(v));

	QSettings fs("Novorado","ParkingAssistant");
	fs.beginGroup("Firmware");
	fs.setValue("frequency",f);
	fs.setValue("loudness",v);
	fs.endGroup();
}

void SoundsTab::buzzerFL(unsigned char f,unsigned char l, int /*fHz*/)
{
	QSlider
		*sf =qFindChild<QSlider*>(this,"horizontalSlider_Frequency"),
		*sl=qFindChild<QSlider*>(this,"horizontalSlider_Loudness");

	sf->setValue(f);
	sl->setValue(l);

	if(!sf->isEnabled()) sf->setEnabled(true);
	if(!sl->isEnabled()) sl->setEnabled(true);

	std::cout << "Got Frequency " << (int)(f) << "% and loudness " << (int)(l) << "%" << std::endl;
}

SoundsTab::~SoundsTab()
{
//	saveSettings();
}

void SoundsTab::readSettings()
{
	if(m_settings->soundNotification()==ParkAssist::SNM_VOICE){
//		std::cout << "Voice notification" << std::endl;
		qFindChild<QRadioButton*>(this, "radioButton_Voice")->setChecked(true);
		qFindChild<QRadioButton*>(this, "radioButton_Beeps")->setChecked(false);
		}else {
			qFindChild<QRadioButton*>(this, "radioButton_Voice")->setChecked(false);
			qFindChild<QRadioButton*>(this, "radioButton_Beeps")->setChecked(true);
			}
}

void SoundsTab::getFile(const QDir& p,QLabel& l)
{
// 	std::cout << "Looking here: " << p.absolutePath().toAscii().constData() <<std::endl;
	QFileDialog d(
		this,
     		tr("Select Sound"),
		p.absolutePath(),
		tr("Sound Files (*.wav)")
		);

	d.setFont(BIG_FONT);

	QStringList fileNames;
	if (d.exec())
		fileNames = d.selectedFiles();

	if(fileNames.size()) l.setText(p.relativeFilePath(fileNames[0]));
}

void SoundsTab::saveSettings()
{
	if(qFindChild<QRadioButton*>(this, "radioButton_Voice")->isChecked()){
		m_settings->setNotification("voice");
		} else {
			m_settings->setNotification("beeps");
			}
}

SoundsTab::SoundsTab(Parktronic& prk, QWidget* p, Settings* s) : QWidget(p),  TabSettings(s), m_parktronic(prk)
{
	Ui::SoundsTabForm ui;
	ui.setupUi(this);
	readSettings();
}

GUITab::GUITab(QWidget* p,QTranslator& t, Settings* s) : QWidget(p), TabSettings(s), m_translator(t)
{
	Ui::GUITabForm ui;
	ui.setupUi(this);
	readSettings();
}

GUITab::~GUITab()
{
//	saveSettings();
}

void GUITab::on_pushButton_imageFile_clicked()
{
	// Launch file dialog
	 QString fileName = QFileDialog::getOpenFileName(this, tr("Choose background image"),
				 "",
				 tr("Image files (*.bmp *.gif *.jpg)"));

	if(fileName.length()) {
		m_settings->setBgImageFilePath(fileName);
		qFindChild<QLineEdit*>(this, "lineEdit_backgroundImage")->setText(m_settings->bgImageFilePath());
		}
}

void GUITab::on_pushButton_clearSettings_clicked()
{
	emit clearSettingsAndReboot();
}

void GUITab::on_comboBox_SelectLanguage_activated(int idx)
{
	if(idx==0) {
		// Russian
		m_settings->setLang("russian");
		QSettings set("Novorado","Sections");
		QFileInfo fi(set.value("Parking Assistant","%PROGRAMFILES%\\Novorado\\Parking Assistant\\").toString()+"parktronic_ru.qm");

		if(!m_translator.load(fi.absoluteFilePath())) {
			QMessageBox::warning(
				this,
				tr("Unable to load Russian translation"),
				tr("No parktronic_ru.qm file found"));
			}

		std::cout << "Application loaded translator" << std::endl;
		qApp->installTranslator(&m_translator);

		} else {
			m_settings->setLang("english");
			QTranslator theEmptyOne;
			qApp->installTranslator(&theEmptyOne);
			}

}

void FirmwareUpdateTab::on_pushButton_CheckConnection_clicked()
{
	syncDevice();
}

void FirmwareUpdateTab::fixFirmware()
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
	m_parktronic.configureBuzzer(f,l);
	std::cout << "Programming transponder shifts" << std::endl;
	m_parktronic.saveShift(shifts);
}

void FirmwareUpdateTab::onFirmwareUpdateFinished(bool f)
{
	std::cout << "void FirmwareUpdateTab::onFirmwareUpdateFinished(bool f=" << (f?"success":"failed") << ")" << std::endl;

	// Fix the firmware
	fixFirmware();

	// Unfreeze shit
	qFindChild<QPushButton*>(this,"pushButton_CheckConnection")->setEnabled(true);

	qFindChild<QLabel*>(this, "label_Ping")->setText("");
	qFindChild<QProgressBar*>(this, "progressBar_Update")->setValue(0);
	qFindChild<QLabel*>(this, "label_FirmwareVersion")->
		setText("<font color=red>Not connected</font>");

	qFindChild<QPushButton*>(this,"pushButton_Ping")->setEnabled(false);
	qFindChild<QPushButton*>(this,"pushButton_Reboot")->setEnabled(false);
	qFindChild<QPushButton*>(this,"pushButton_Upload")->setEnabled(false);
	qFindChild<QComboBox*>(this,"comboBox")->setEnabled(false);
}

ThermoTab::ThermoTab(QWidget* p,Parktronic& park):QWidget(p), m_parktronic(park)
{
	Ui::thermoForm ui;
	ui.setupUi(this);
}

ThermoTab::~ThermoTab()
{
}

QTableWidget* ThermoTab::table()
{
	return qFindChild<QTableWidget*>(this,"thermoWidget");
}

void ThermoTab::on_pushButton_thermo_clicked()
{
	// Clear the table
	table()->clear();
//	m_idleTimer.stop();
	qApp->processEvents();
	m_parktronic.readThermoSensors();
}

void FirmwareUpdateTab::on_pushButton_Ping_clicked()
{
	m_parktronic.ping();
	qFindChild<QLabel*>(this, "label_Ping")->setText(QWidget::tr("Waiting"));
}

void FirmwareUpdateTab::on_pushButton_Reboot_clicked()
{
	qFindChild<QLabel*>(this, "label_FirmwareVersion")->
		setText("<font color=red>Not connected</font>");

	qFindChild<QPushButton*>(this,"pushButton_Ping")->setEnabled(false);
	qFindChild<QPushButton*>(this,"pushButton_Reboot")->setEnabled(false);
	qFindChild<QPushButton*>(this,"pushButton_Upload")->setEnabled(false);
	qFindChild<QComboBox*>(this,"comboBox")->setEnabled(false);

	m_parktronic.reboot();
}

void FirmwareUpdateTab::progress(const QString& msg)
{
 //	std::cout << "Progress message: " << msg.toAscii().constData() << std::endl;
	if(!msg.startsWith("Progress")||!msg.endsWith("%")){

		QString shortMsg=msg.left(26);
		// qFindChild<QProgressBar*>(this, "progressBar_Update")->setValue(0);
		qFindChild<QLabel*>(this,"label_FirmwareVersion")->setText(shortMsg);
		return;
		}

	int
		lg=msg.length(),
		val = msg.mid(9,lg-1-9).toInt();

	qFindChild<QProgressBar*>(this, "progressBar_Update")
			->
		setValue(val);
}

void FirmwareUpdateTab::on_lineEdit_Password_returnPressed()
{
	if(qFindChild<QLineEdit*>(this,"lineEdit_Password")->text()==FIRMWARE_PASSWORD)

		qFindChild<QPushButton*>(this,"pushButton_Upload")->setEnabled(true);
}

void FirmwareUpdateTab::onPing()
{
	qFindChild<QLabel*>(this, "label_Ping")->setText(tr("Got ping"));

	qFindChild<QPushButton*>(this,"pushButton_Reboot")->setEnabled(true);
	//qFindChild<QPushButton*>(this,"pushButton_Upload")->setEnabled(true);
	qFindChild<QLineEdit*>(this,"lineEdit_Password")->setEnabled(true);
	qFindChild<QComboBox*>(this,"comboBox")->setEnabled(true);
}

/*
class FirmwareData {
	public:
		std::string version;
		const char** data;

		FirmwareData(){
			data=NULL;
			}

		void load(const char* nm,const char**d){
			version = nm;
			data=d;
			}
	};

std::map<std::string,FirmwareData> fdata;
*/

void FirmwareUpdateTab::on_pushButton_Upload_clicked()
{
	qFindChild<QPushButton*>(this,"pushButton_CheckConnection")->setEnabled(false);
	qFindChild<QPushButton*>(this,"pushButton_Ping")->setEnabled(false);
	qFindChild<QPushButton*>(this,"pushButton_Reboot")->setEnabled(false);
	qFindChild<QPushButton*>(this,"pushButton_Upload")->setEnabled(false);
	qFindChild<QComboBox*>(this,"comboBox")->setEnabled(false);

	std::string version=qFindChild<QComboBox*>(this, "comboBox")->
		currentText().toAscii().constData();

	std::cout << "Flashing firmware of version " << version << std::endl;

	if(m_firmware) m_firmware.clear();

	m_firmware = new Novorado::Devices::Firmware;

	m_parktronic.firmwareUpdate(m_firmware->loadFirmware(version));
}

#if	0
struct statLoader {
	static std::string lastFirmware;
	statLoader(const char* a, const char**d) {
		fdata[lastFirmware=a].load(a,d);
		}
	};

std::string statLoader::lastFirmware;
#endif

void ParkAssist::usbPoll()
{
	if(m_parktronic) {
		//static int quantcnt = 0;
		//std::cout << "Parktronic Quant #" << ++quantcnt << std::endl;
		m_parktronic->quant();
		}
}

/*

	Compressed firmware to save static storage space

#define	LOAD_FIRMWARE(obj,data)	static statLoader __tmp##data(obj,data);
#include	<firmware_export.h>
*/

void ParkAssist::askFirmwareString()
{
	m_firmwareString=NULL;

	QMessageBox msgBox;
	msgBox.setIcon(QMessageBox::Warning);
	msgBox.setText(tr("!!! ATTENTION !!!"));
	msgBox.setInformativeText(tr("When RED indicator is on, device is broken. We will now try fixing it for you.\nPress OK to flash new firmware.\nIf you cancel, it may break the device"));
	msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
	msgBox.setFont(BIG_FONT);

	msgBox.raise();

	if(QMessageBox::Ok==msgBox.exec()) {

		if(m_firmware) m_firmware.clear();

		m_firmware = new Novorado::Devices::Firmware;
		m_firmware->versions();
		m_firmwareString = m_firmware->loadFirmware(m_firmware->recentVersion());

		infoWindow = new QMessageBox();
		infoWindow->setFont(BIG_FONT);
		infoWindow->setText(tr("Firmware update in progress"));
		infoWindow->setInformativeText(tr("ATTENTION!\nDO NOT TURN OFF YOUR DEIVCE OR STOP THIS PROGRAM.\nTUNRING OFF WILL BRAKE THE DEVICE"));
		infoWindow->setStandardButtons(QMessageBox::NoButton);
		infoWindow->setMinimumWidth(400);
		infoWindow->setIcon(QMessageBox::Critical);
		infoWindow->show();

		} else m_firmwareString = NULL;

	m_waitFirmwareString->wakeOne();
}

const char** ParkAssist::onFirmwareMode()
{
	m_firmwareString = NULL;

	emit wantFirmwareString();

	m_waitFirmwareMutex.lock();

	std::cout << "Waiting for firmware .." << std::endl;
	m_waitFirmwareString = new QWaitCondition;

	m_waitFirmwareString->wait(&m_waitFirmwareMutex);
	m_waitFirmwareMutex.unlock();

	// Returning NULL from that function will just reboot the device, no programming done
	std::cout << "Park Assist: automatic firmware flash " << (m_firmwareString?" in progrress":"disabled") << std::endl;
	return m_firmwareString;
}

FirmwareUpdateTab::FirmwareUpdateTab(QWidget* p,Parktronic& park):QWidget(p), m_parktronic(park)
{
	Ui::FirmwareUpdateForm ui;
	ui.setupUi(this);

	qFindChild<QLabel*>(this, "label_FirmwareVersion")->
		setText("<font color=red>Not connected</font>");
	qFindChild<QLabel*>(this, "label_Ping")->setText(" ");
	qFindChild<QLabel*>(this, "label_Reboot")->setText(" ");
	qFindChild<QProgressBar*>(this, "progressBar_Update")->setRange(0,100);

	Novorado::Devices::Firmware frm;

	std::map<std::string,std::string> v = frm.versions();

	for(std::map<std::string,std::string>::reverse_iterator i=v.rbegin();i!=v.rend();i++){
		qFindChild<QComboBox*>(this, "comboBox")->addItem(i->first.c_str());
		}
}

FirmwareUpdateTab::~FirmwareUpdateTab()
{
}

void GUITab::readSettings()
{
	if(m_settings->lang()=="russian"){
		qFindChild<QComboBox*>(this, "comboBox_SelectLanguage")->setCurrentIndex(0);
		} else {
			qFindChild<QComboBox*>(this, "comboBox_SelectLanguage")->setCurrentIndex(1);
			}

	qFindChild<QSpinBox*>(this, "spinBox_AutoHideDelay")->setValue(m_settings->hideDelay());
	qFindChild<QSpinBox*>(this, "spinBox_ShowDelay")->setValue(m_settings->showDelay());
	qFindChild<QLineEdit*>(this, "lineEdit_backgroundImage")->setText(m_settings->bgImageFilePath());
}

void GUITab::saveSettings()
{
	m_settings->setHideDelay(qFindChild<QSpinBox*>(this, "spinBox_AutoHideDelay")->value());
	m_settings->setShowDelay(qFindChild<QSpinBox*>(this, "spinBox_ShowDelay")->value());
	m_settings->setBgImageFilePath(qFindChild<QLineEdit*>(this, "lineEdit_backgroundImage")->text());
}

void WebCamTab::onStateChanged(int state)
{
	if(state==Qt::Checked){
		qFindChild<QComboBox*>(this,"comboBox_cam")->setEnabled(true);
		} else {
			qFindChild<QComboBox*>(this,"comboBox_cam")->setEnabled(false);
			}
}

int WebCamTab::isVideoEnabled()
{
	QCheckBox* enableVideo = qFindChild<QCheckBox*>(this,"checkBox_enableVideo");
	return enableVideo->isChecked();
}

WebCamTab::WebCamTab(QWidget* p, Settings* s):QWidget(p), TabSettings(s)
{
	Ui::WebCamTabForm ui;
	ui.setupUi(this);
	readSettings();

	std::list<std::string> testDshow = getCaptureDevices();

	QComboBox* ccam = qFindChild<QComboBox*>(this,"comboBox_cam");
	if(!ccam) throw LogicErrorException("Check WebCamTab user interface");

	QCheckBox* enableVideo = qFindChild<QCheckBox*>(this,"checkBox_enableVideo");
	connect(enableVideo,SIGNAL(stateChanged(int)),this,SLOT(onStateChanged(int)));

	ccam->clear();
	ccam->setDuplicatesEnabled(false);

	ccam->addItem(m_settings->videoSourceMRL());

	for(std::list<std::string>::iterator i=testDshow.begin();i!=testDshow.end();i++){
		if(ccam->findText(i->c_str())==-1) ccam->addItem(QString::fromUtf8(i->c_str()));
		}

	if(ccam->findText("fake://")==-1) ccam->addItem("fake://");
	if(ccam->findText("dshow://")==-1) ccam->addItem("dshow://");
	if(ccam->findText("v4l2://")==-1) ccam->addItem("v4l2://");
}

void WebCamTab::readSettings()
{
	videoSourceMRL=m_settings->videoSourceMRL();

	QComboBox* c = qFindChild<QComboBox*>(this, "comboBox_cam");

	int i = c->findText(videoSourceMRL);

	if(i<=-1) c->addItem(videoSourceMRL);

	c->setCurrentIndex(c->findText(videoSourceMRL));

	qFindChild<QCheckBox*>(this,"checkBox_enableVideo")->setCheckState(m_settings->videoEnabled()?Qt::Checked:Qt::Unchecked);

	c->setEnabled(isVideoEnabled());
}

WebCamTab::~WebCamTab()
{
//	saveSettings();
}

void WebCamTab::saveSettings()
{
	m_settings->setVideoSourceMRL(qFindChild<QComboBox*>(this, "comboBox_cam")->currentText());
	m_settings->setVideoEnabled(isVideoEnabled());
}

void SoundsTab::on_radioButton_Voice_clicked()
{
}

void SoundsTab::on_radioButton_Beeps_clicked()
{
	// Enable the beep selection
}

void DTDrive::set(DiagnosticsTab* dt, const QString& k)
{
	owner=(dt),key=(k);
}

void DTDrive::callMe()
{
	owner->saveOnDrive(key);
}

void DiagnosticsTab::saveOnDrive(const QString& d)
{
	QString s=d;
	s += "/";
	s += generateFileName();
	s += ".dat";

	std::cout << "Saving " << s.toAscii().constData() << std::endl;

	if(!saveCompressedLog(s)) {

		QMessageBox::warning(this,tr("Saving log file"),tr("Unable to save the file"));

		} else QMessageBox::warning(this,tr("Saving log file"),
			QString(tr("File %1 saved.Please submit your report to support@novorado.com").arg(s)));

}

void DiagnosticsTab::clearLog()
{
	m_settings->setCleanLog(true);

	QMessageBox::information(this,tr("Log file reset requested"),tr("Log file will be started over next time you run the application"));
}

void DiagnosticsTab::askLogFileName()
{
	 QString fileName = QFileDialog::getSaveFileName(this, tr("Choose log File"),
				 "",
				 tr("Text files (*.log *.txt)"));

	if(fileName.length()) {
		m_settings->setLogFileName(fileName);
		qFindChild<QLabel*>(this,"label_logFileName")->setText(m_settings->logFileName());
		}
}

void DiagnosticsTab::refreshData()
{
	QFileInfo fi(m_settings->logFileName());
	qFindChild<QLabel*>(this,"label_logFileSize")->setText(tr("Log file size is %1KB").arg(fi.size()/1024));
}

bool DiagnosticsTab::saveCompressedLog(const QString& fn)
{
	QByteArray cmp;
	int inputSize=0;
	{
	QFile f(m_settings->logFileName());
	f.open(QIODevice::ReadOnly);
	QByteArray inp=f.readAll();
	f.close();
	inputSize=inp.size();
	cmp = qCompress(inp,9); // gzip compression

	if(!cmp.size()) {
		QMessageBox::warning(this,tr("Opening log file"),QString(tr("Unable to read log file %1").arg(m_settings->logFileName())));
		return false;
		}
	}

	QFile o(fn);
	bool r = o.open(QIODevice::WriteOnly);
	size_t sz=0;
	if(r) r = ((sz=o.write(cmp))>0);
	o.close();

#if	0
	if(r) 	QMessageBox::information(this,QString(tr("Saving compressed log file %1")).arg(fn),QString(tr("File has been saved: %1, compressed %2Kb to %3Kb")
				.arg(fn).arg(inputSize/1024).arg(sz/1024)));
	else {
			QMessageBox::warning(this,QString(tr("Saving compressed log file %1").arg(fn)),tr("Unable to save compressed log file, check the path"));
			return false;
	}
#endif

	return r;
}

void DiagnosticsTab::onDataTransferProgress ( qint64 done, qint64 /*total*/ )
{
	std::cout << "Ftp % - " << done << std::endl;
	prog->setValue(done);
}


QString DiagnosticsTab::stateStr()
{
switch(ftp->state()){
case QFtp::Unconnected: return tr("There is no connection to the host.");
case QFtp::HostLookup: return tr("A host name lookup is in progress.");
case QFtp::Connecting: return tr("An attempt to connect to the host is in progress.");
case QFtp::Connected: return tr("Connection to the host has been achieved.");
case QFtp::LoggedIn: return tr("Connection and user login have been achieved.");
case QFtp::Closing: return tr("The connection is closing down, but it is not yet closed. (The state will be Unconnected when the connection is closed.)");
};
	return tr("Unknown");
}

void DiagnosticsTab::onFtpStateChanged( int /*si*/)
{

	QString i = stateStr();
	std::cout << "Ftp state changed: " << i.toAscii().constData() << std::endl;
	prog->setLabelText(i);
}

void DiagnosticsTab::onCommandStarted ( int id  )
{
	std::cout << "Ftp command started: " << id << std::endl;
}

void DiagnosticsTab::onCommandFinished (  int id, bool error )
{
	std::cout << "Ftp command finished: " << id << ", error: " << (error?"Yes":"Nop")<< std::endl;

}

void DiagnosticsTab::onFtpDone(bool error)
{
	prog->close();

	if(error) QMessageBox::warning(this,tr("Sending log file"),tr("Unable to complete sending the file"));
	else QMessageBox::warning(this,tr("Sending log file"),tr("Thank you for submitting report"));
}

void DiagnosticsTab::sendToNovorado()
{
	if(prog || ftp) {
		QMessageBox::warning(this,tr("Sending log file"),tr("Another transfer is in progress"));
		return;
		}

	int inputSize=0;
	QByteArray ftp_data;
	{
	QFile f(m_settings->logFileName());
	f.open(QIODevice::ReadOnly);
	QByteArray inp=f.readAll();
	f.close();
	inputSize=inp.size();

	ftp_data = qCompress(inp,9); // gzip compression

	if(!ftp_data.size()) {
		QMessageBox::warning(this,tr("Opening log file"),QString(tr("Unable to read log file %1").arg(m_settings->logFileName())));
		return;
		}
	}

	// Send it over the ftp to Novorado
	ftp = new QFtp(this);

	prog = new QProgressDialog("Sending report over internet...", "Abort Send", 0, ftp_data.size(), this);
	prog->setWindowModality(Qt::WindowModal);

	prog->setRange(0,ftp_data.size());

	connect((QFtp*)(ftp),SIGNAL(dataTransferProgress(qint64 done, qint64 total)), this, SLOT(onDataTransferProgress(qint64 done, qint64 total)));
	connect((QFtp*)(ftp),SIGNAL(commandFinished ( int id, bool error )),this,SLOT(onCommandFinished ( int id, bool error )));
	connect((QFtp*)(ftp),SIGNAL(commandStarted ( int id )),this,SLOT(onCommandStarted ( int id )));
	connect((QFtp*)(ftp),SIGNAL(stateChanged ( int)),this,SLOT( onFtpStateChanged( int )));
	connect((QFtp*)(ftp),SIGNAL(done ( bool)),this,SLOT( onFtpDone( bool)));

	std::cout << "Connecting to Novorado server" << std::endl;
	ftp->connectToHost("ftp.novorado.com");  // id == 1
	std::cout << "Logging in" << std::endl;
	ftp->login("nrdfeedback","lOg!2010");                         // id == 2
	QString s=generateFileName()+".dat";
	std::cout << "Sending file " << s.toAscii().constData() << std::endl;
	put_command_id = ftp->put(ftp_data,s);
	prog->exec();
	ftp->close();

	std::cout << "Done FTP sending" << std::endl;

	// Looks like user wants to exit
	if(ftp) ftp.clear();
	if(prog) prog.clear();
}

QString DiagnosticsTab::generateFileName()
{
	QString s="nrdlog-";
	s+= QDate::currentDate().toString("dd-MM-yyyy");
	s+="-";
	s+= QTime::currentTime().toString("hh-mm-ss-zzz");
	return s;
}

DiagnosticsTab::DiagnosticsTab(QWidget* p, Settings* s):QWidget(p), TabSettings(s)
{
	Ui::DiagnosticsTabForm ui;
	ui.setupUi(this);

	// Populate list of the drives
	#if	defined	MINGW
	QFileInfoList d = QDir::drives ();
	#else
	QFileInfoList d;
	d << QString("/tmp")+QString(" ");
	if(getenv("HOME")) d << QString(getenv("HOME"))+QString(" ");
	#endif

	QGroupBox* groc = qFindChild<QGroupBox*>(this, "groupBox_oneClick");
	if(!groc) throw LogicErrorException("No such group box");

	QHBoxLayout* l = qFindChild<QHBoxLayout*>(this, "horizontalLayout_driveButtons");
	if(!l) throw LogicErrorException("No such layout");

	QPushButton* thing = qFindChild<QPushButton*>(this,"pushButton_diskC");
	thing->hide();
	if(!thing) throw LogicErrorException("No such button");

	// Insert drive buttons into the layout
	for(int i=std::min(d.size()-1,5);i>=0;i--) {
		QPushButton* pb = new QPushButton(groc);
		QString s = d[i].absoluteFilePath();
		s.chop(1);
		pb->setText(s);
		l->insertWidget(0,pb);
		m_drives[s].set(this,s);
		connect(pb,SIGNAL(clicked()),&m_drives[s],SLOT(callMe()));
		}

	// Read the settings
	qFindChild<QCheckBox*>(this,"checkBox_detailedUSBLog")->setChecked(m_settings->logUSB());
	qFindChild<QCheckBox*>(this,"checkBox_applicationLog")->setChecked(m_settings->applicationDebugMessages());
	qFindChild<QCheckBox*>(this,"checkBox_turnOff")->setChecked(m_settings->logToConsoleOnly());
	qFindChild<QCheckBox*>(this,"checkBox_logTo")->setChecked(m_settings->logToFile());
	qFindChild<QLabel*>(this,"label_logFileName")->setText(m_settings->logFileName());

	// Setup the slots
	connect(qFindChild<QPushButton*>(this,"pushButton_clearLog"),SIGNAL(clicked()), this, SLOT(clearLog()));
	connect(qFindChild<QPushButton*>(this,"pushButton_sendToNovorado"),SIGNAL(clicked()), this, SLOT(sendToNovorado()));
	connect(qFindChild<QPushButton*>(this,"pushButton_selectLogFile"),SIGNAL(clicked()), this, SLOT(askLogFileName()));

	// Find out log file size
	QFileInfo fi(m_settings->logFileName());
	qFindChild<QLabel*>(this,"label_logFileSize")->setText(tr("Log file size is %1KB").arg(fi.size()/1024));

	QTimer * t = new QTimer(this);
	t->setObjectName("Diagnostic Timer");

	connect(t,SIGNAL(timeout()), this, SLOT(refreshData()));

	t->start(5000);
}

void DiagnosticsTab::saveSettings()
{
	m_settings->setLogUSB(qFindChild<QCheckBox*>(this,"checkBox_detailedUSBLog")->isChecked());
	m_settings->setApplicationDebugMessages(qFindChild<QCheckBox*>(this,"checkBox_applicationLog")->isChecked());
	m_settings->setLogToConsoleOnly(qFindChild<QCheckBox*>(this,"checkBox_turnOff")->isChecked());
	m_settings->setLogToFile(qFindChild<QCheckBox*>(this,"checkBox_logTo")->isChecked());
	m_settings->setLogFileName(qFindChild<QLabel*>(this,"label_logFileName")->text());
}

DiagnosticsTab::~DiagnosticsTab()
{
//	saveSettings();
}
