//qt
#include <QtGui/QApplication>
//own
#include "frameshandlerMW.h"

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);
	FrameHandlerMWindow mw;
	mw.show();
	return app.exec();
}
