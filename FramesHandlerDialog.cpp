//own
#include "FramesHandlerDialog.h"
//qt
#include <QtGui/QLayout>
#include <QtCore/QDebug>

FramesHandlerDialog::FramesHandlerDialog(QWidget *parent) : QDialog(parent) 
{
	createButtons();
	QVBoxLayout *framesHandlerLayout = new QVBoxLayout();
	framesHandlerLayout->addWidget(&frameHandlerWin);
	framesHandlerFrame.setLayout(framesHandlerLayout);
	framesHandlerFrame.setFrameShape(QFrame::StyledPanel);
	framesHandlerFrame.setFrameShadow(QFrame::Sunken);
	framesHandlerLayout->setMargin(0);
	framesHandlerLayout->setSpacing(0);

	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->addWidget(&framesHandlerFrame);
	mainLayout->addWidget(dialogButtonBox);

	mainLayout->setMargin(10);
	mainLayout->setSpacing(5);
	setMinimumSize(1024, 768);
	setMaximumSize(1024, 768);
}

void FramesHandlerDialog::createButtons()
{
	dialogButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
	connect( dialogButtonBox, SIGNAL( accepted() ), this, SLOT( acceptSlot() ) );
}

void FramesHandlerDialog::acceptSlot()
{
	emit setSensors( frameHandlerWin.getRoiCount() );
	accept();
}
