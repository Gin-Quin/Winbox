#include <os.h>
#include <SDL\SDL.h>

#include "Winbox.h"
#include "AllWidgets.h"






int main(void)
{
	SDL_Surface *scr = wInitSDL();
	DrawFillRect(scr, NULL, SDL_MapRGB(scr->format, 125, 212, 96));
	
	Widget *mainWindow = wWindow("COUCOU");
	Widget *req = wRequest("Request", NULL, REQUEST_INT, 5135);
	wRequest_SetMin(req, -123654);
	
	wAddWidget(mainWindow, req);
	
	wExecConstruct(mainWindow, NULL);
	
	SDL_Quit();
	return 1;
}





