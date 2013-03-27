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
#include	<list>
#include	<string>
#include	<deque>
#include	<iostream>
#include	"parkAssist.h"
#include	"parkSetup.h"

std::list<std::string> Novorado::Parking::WebCamTab::getCaptureDevices()
{
	std::list<std::string> rv;
	rv.push_back("v4l2://");
	return rv;
}
