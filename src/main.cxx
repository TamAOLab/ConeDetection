
#include <QApplication>
#include <itkTextOutput.h>

#include "radmainwindow.h"

int main(int argc, char** argv)
{
	itk::OutputWindow::SetInstance(itk::TextOutput::New());

	// Opening files from the command line, will work for drag and drop Split files on the Split icon
	QStringList inputNames;
	for (int i = 1; i < argc; i++) {
		inputNames.append(QString(argv[i]));
	}
	QStringList splitFileNames, detectionFileNames;
	decodeFileList(inputNames, splitFileNames, detectionFileNames);

	QApplication app(argc, argv);

	radMainWindow window;
	window.setWindowIcon(QIcon(":conedetect.png"));
	window.show();

	if (!splitFileNames.isEmpty()) {
		window.openSplitImages(splitFileNames, true);
		window.loadDetections(detectionFileNames);
	}
	else {
		window.checkWhatsNew();
	}

	app.exec();
  
	return EXIT_SUCCESS;
}
