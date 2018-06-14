#pragma once
#include <RaspberryPiShared.h>
#include "LANNetworkCheck.h"

class EmailNotifier
{
public:
	EmailNotifier();
	~EmailNotifier();

	void SendEmail(RaspberryPi::VideoSurveillance::DoorStatus ds, bool bTakePicture);



};

