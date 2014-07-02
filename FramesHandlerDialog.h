#pragma once
//own
#include "frameshandlerMW.h"
//qt 
#include <QtGui/QDialog>
#include <QtGui/QFrame>
#include <QtGui/QDialogButtonBox>

class QDialogButtonBox;
class QTextEdit;

class FramesHandlerDialog : public QDialog
{
	Q_OBJECT

public:
	FramesHandlerDialog(QWidget *parent = 0); 

signals:
	void setSensors(int);

private:
	void createButtons();

private slots:
	void acceptSlot();

private:
	QFrame framesHandlerFrame;
	FrameHandlerMWindow frameHandlerWin;
	QDialogButtonBox *dialogButtonBox;
	QTextEdit *resultsEdit; 
};