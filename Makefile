all: debug

debug:
	cd RTT_Common/Debug; $(MAKE)
	cd RTT_Server/Debug; $(MAKE)
	cd RTT_Client_Core/Debug; $(MAKE)
	cd RTT_Client_GTK/Debug; $(MAKE)
	cd RTT_Ogre_3D/Debug; $(MAKE)

clean:
	cd RTT_Common/Debug; $(MAKE) clean
	cd RTT_Server/Debug; $(MAKE) clean
	cd RTT_Client_Core/Debug; $(MAKE) clean
	cd RTT_Client_GTK/Debug; $(MAKE) clean
	cd RTT_Ogre_3D/Debug; $(MAKE) clean

install:
	install RTT_Client_Core/Debug/libRTT_Client_Core.so $(DESTDIR)/usr/lib/
	install RTT_Client_GTK/Debug/RTT_Client_GTK $(DESTDIR)/usr/bin/
	install RTT_Server/Debug/RTT_Server $(DESTDIR)/usr/bin/
	mkdir -p $(DESTDIR)/usr/share/RTT/GTK/UI/
	install RTT_Client_GTK/UI/* $(DESTDIR)/usr/share/RTT/GTK/UI/ --mode=644
	mkdir -p $(DESTDIR)/usr/share/RTT/Ogre/assets/materials/textures/
	mkdir -p $(DESTDIR)/usr/share/RTT/Ogre/assets/models/
	cp -r RTT_Ogre_3D/assets/ $(DESTDIR)/usr/share/RTT/Ogre/
	install RTT_Ogre_3D/resources_install.cfg $(DESTDIR)/usr/share/RTT/Ogre/ --mode=644
	mv $(DESTDIR)/usr/share/RTT/Ogre/resources_install.cfg $(DESTDIR)/usr/share/RTT/Ogre/resources.cfg
	install RTT_Ogre_3D/plugins.cfg $(DESTDIR)/usr/share/RTT/Ogre/ --mode=644
	install RTT_Ogre_3D/ogre.cfg $(DESTDIR)/usr/share/RTT/Ogre/ --mode=644
	install RTT_Ogre_3D/Debug/RTT_Ogre_3D $(DESTDIR)/usr/bin/
	
uninstall:
	rm -f $(DESTDIR)/usr/lib/libRTT_Client_Core.so
	rm -f $(DESTDIR)/usr/bin/RTT_Client_GTK
	rm -f $(DESTDIR)/usr/bin/RTT_Server
	rm -f $(DESTDIR)/usr/bin/RTT_Ogre_3D
	rm -rf $(DESTDIR)/usr/share/RTT/
