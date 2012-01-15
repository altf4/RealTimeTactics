//============================================================================
// Name        : Job.h
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A Job is a type of unit. The parameters within this class define
//					what that job is and how it operates. In contrast, a Unit is
//					an particular instantiation of a Job type.
//============================================================================

#ifndef JOB_H_
#define JOB_H_

#include <string>
#include "stdlib.h"
#include <openssl/sha.h>

using namespace std;

namespace RTT
{

class Job
{

public:
	//This is what is displayed to the user as the Unit's Job
	//IE: Sniper, Lancer, Black Mage, etc...
	//	Try to make it unique. But it's not required to be
	string jobName;

	//Starting stats for this Job. It can be modified by items / abilities / etc
	uint maxHealth;
	uint maxEnergy;

	uint verticalMovement;
	uint horizontalMovement;

	uint baseVision;

	uint basicAttackRange;
	double basicAttackDamage;

	//How many charge points accumulated per round
	//	IE: How quickly this unit gets to have a turn
	//	(Not how far it can walk)
	uint speed;

	//SHA1 hash of all graphics files hashed
	//TODO: Specify the order in which these files must be tar'ed
	unsigned char graphicsHash[SHA_DIGEST_LENGTH];

	//Globally unique identifier for this Job
	//	Consists of the SHA1 hash of the XML Job file
	unsigned char IDHash[SHA_DIGEST_LENGTH];

	//Constructor from XML Job file
	Job(string filePath);

	//Create a copy Job from an existing Job
	//	May seem weird. But it's used to make a Unit object from an existing
	//	Job object. Which needs to call a Job constructor
	Job( Job *copyJob );
};

}

#endif /* JOB_H_ */
