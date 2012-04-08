//============================================================================
// Name        : Job.cpp
// Author      : AltF4
// Copyright   : 2011, GNU GPLv3
// Description : A Job is a type of unit. The parameters within this class define
//					what that job is and how it operates. In contrast, a Unit is
//					an particular instantiation of a Job type.
//============================================================================

#include "Job.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

using namespace RTT;
using namespace std;

//Constructor from XML Job file
Job::Job(string filePath)
{
	string fileName = filePath.substr( filePath.find_last_of( '\\' ) +1 );

    // Create an empty property tree object
    using boost::property_tree::ptree;
    ptree pt;

    //The SHA1 hash + ".job"
    if( fileName.size() != (SHA_DIGEST_LENGTH *2) + 4)
    {
    	//Error. Bad hash size
    	cerr << "ERROR: Job has an invalid Graphics Hash length\n";
    	return;
    }

	//Test to make sure that the file's hash matches the file name
	FILE *file;
	unsigned char buf[8192];
	unsigned char fileHash[SHA_DIGEST_LENGTH];
	SHA_CTX sc;
	int err;

	file = fopen(filePath.c_str(), "rb");
	if (file == NULL)
	{
		cerr << "ERROR: Job file not found\n";
		return;
	}
	SHA1_Init(&sc);
	for (;;)
	{
		size_t len;
		len = fread(buf, 1, sizeof buf, file);
		if (len == 0)
		{
			break;
		}
		SHA1_Update(&sc, buf, len);
	}
	err = ferror(file);
	fclose(file);
	if (err)
	{
		/* some I/O error was encountered; report the error */
		cerr << "ERROR: IO problem with given Job file\n";
		return;
	}
	SHA1_Final(fileHash, &sc);

    try
    {
		// Load the XML file into the property tree. If reading fails
		// (cannot open file, parse error), an exception is thrown.
		read_xml(filePath, pt);

		m_jobName = pt.get<std::string>("job.jobName");

		m_maxHealth = pt.get<uint>("job.maxHealth");
		m_maxEnergy = pt.get<uint>("job.maxEnergy");

		m_verticalMovement = pt.get<uint>("job.verticalMovement");
		m_horizontalMovement = pt.get<uint>("job.horizontalMovement");
		m_speed = pt.get<uint>("job.speed");

		m_baseVision = pt.get<uint>("job.baseVision");
		m_basicAttackRange = pt.get<uint>("job.basicAttackRange");

		m_basicAttackDamage = pt.get<double>("job.basicAttackDamage");

		string gHashString = pt.get<string>("job.graphicsHash");

		//Times two since a byte is two string characters
		if( gHashString.size() != (SHA_DIGEST_LENGTH *2))
		{
			//Error. Bad hash size
			cerr << "ERROR: Job has an invalid Graphics Hash length\n";
			return;
		}

		//Copy in the SHA1 hashes
		string hashByte;
		for( uint i = 0; i < SHA_DIGEST_LENGTH; i++ )
		{
			hashByte = gHashString.substr(i, 2);
			hashByte.append('\0');
			m_graphicsHash[i] = (unsigned char)strtol(hashByte.c_str(), NULL, 16);

			hashByte = fileName.substr(i, 2);
			hashByte.append('\0');
			m_IDHash[i] = (unsigned char)strtol(hashByte.c_str(), NULL, 16);
		}

		for( uint i = 0; i < SHA_DIGEST_LENGTH; i++ )
		{
			if( m_IDHash[i] != fileHash[i])
			{
				cerr << "ERROR: ID hash doesn't match file's computer hash\n";
				return;
			}
		}

    }
    catch(boost::property_tree::ptree_bad_data error)
    {
    	cerr << "ERROR: Parsing Job XML returned error.\n";
		return;
    }
    catch(boost::property_tree::ptree_bad_path error)
    {
    	cerr << "ERROR: Could not find Job XML file.\n";
		return;
    }

}

//Create a copy Job from an existing Job
//	May seem weird. But it's used to make a Unit object from an existing
//	Job object. Which needs to call a Job constructor
Job::Job( Job *copyJob )
{
	*this = *copyJob;
}
