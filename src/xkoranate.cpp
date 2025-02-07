#include "application.h"

int main(int argc, char * * argv)
{
	
	XkorApplication app(argc, argv);
    app.loadSports();
	
	return app.exec();
}
