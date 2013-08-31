QMAKE=qmake-qt4
LRELEASE=lrelease-qt4

test:
	cd send-msg && $(QMAKE) && make
	#cd send-msg && $(LRELEASE) sendmsg.pro

clean:
	cd send-msg && make clean
	-rm send-msg/sendmsg
	-rm send-msg/Makefile

