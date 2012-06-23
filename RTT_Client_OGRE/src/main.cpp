/*
 * RTT_Client_OGRE.cpp
 *
 *	An OGRE based 3D frontend to the RTT project.
 *
 *
 *  Created on: Jun 22, 2012
 *      Author: Mark Petro
 *      Built from the Advanced OGRE Framework tutorial found here:
 *      http://www.ogre3d.org/tikiwiki/tiki-index.php?page=Advanced+Ogre+Framework&structure=Tutorials
 */

//|||||||||||||||||||||||||||||||||||||||||||||||

#include "DemoApp.hpp"

//|||||||||||||||||||||||||||||||||||||||||||||||

#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"

//|||||||||||||||||||||||||||||||||||||||||||||||

INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR strCmdLine, INT)
#else
int main(int argc, char **argv)
#endif
{
	DemoApp demo;
	try
	{
		demo.startDemo();
	}
	catch(std::exception& e)
    {
#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
        MessageBoxA(NULL, e.what(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
        fprintf(stderr, "An exception has occurred: %s\n", e.what());
#endif
    }

    return 0;
}

//|||||||||||||||||||||||||||||||||||||||||||||||
