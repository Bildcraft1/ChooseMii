#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <asndlib.h>
#include <mp3player.h>
#include <wiiuse/wpad.h>

#include "sample_mp3.h"


static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
#define TITLE_ID(x,y) (((u64)(x) << 32) | (y))

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	//---------------------------------------------------------------------------------

	// Initialise the video system
	VIDEO_Init();

	// This function initialises the attached controllers
	WPAD_Init();

	// Initialise the audio subsystem
	ASND_Init();
	MP3Player_Init();

	// Obtain the preferred video mode from the system
	// This will correspond to the settings in the Wii menu
	rmode = VIDEO_GetPreferredMode(NULL);

	// Allocate memory for the display in the uncached region
	xfb = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

	// Initialise the console, required for printf
	console_init(xfb, 20, 40, rmode->fbWidth, rmode->xfbHeight, rmode->fbWidth * VI_DISPLAY_PIX_SZ);

	// Set up the video registers with the chosen mode
	VIDEO_Configure(rmode);

	// Tell the video hardware where our display memory is
	VIDEO_SetNextFramebuffer(xfb);

	// Make the display visible
	VIDEO_SetBlack(FALSE);

	// Flush the video register changes to the hardware
	VIDEO_Flush();

	// Wait for Video setup to complete
	VIDEO_WaitVSync();
	if (rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();

	// The console understands VT terminal escape codes
	// This positions the cursor on row 2, column 0
	// we can use variables for this with format codes too
	// e.g. printf ("\x1b[%d;%dH", row, column );
	printf("\x1b[2;0H");

	int selectedOption = 0;
	int numOptions = 3;
	char *options[] = {"Wii Menu", "Neek2o", "Homebrew Launcher"};

	MP3Player_PlayBuffer(sample_mp3, sample_mp3_size, NULL);

	while (1)
	{
		// Call WPAD_ScanPads each loop, this reads the latest controller states
		WPAD_ScanPads();

		// WPAD_ButtonsDown tells us which buttons were pressed in this loop
		// this is a "one shot" state which will not fire again until the button has been released
		u32 pressed = WPAD_ButtonsDown(0);

		// Check if the user has pressed the left or right buttons
		if (pressed & WPAD_BUTTON_UP)
		{
			selectedOption = (selectedOption - 1 + numOptions) % numOptions;
		}
		else if (pressed & WPAD_BUTTON_DOWN)
		{
			selectedOption = (selectedOption + 1) % numOptions;
		}

		// Clear the screen
		printf("\x1b[2J");

		printf("Wii Bootloader\n");


		// Print the menu options
		for (int i = 0; i < numOptions; i++)
		{
			if (i == selectedOption)
			{
				printf("> %s\n", options[i]);
			}
			else
			{
				printf("  %s\n", options[i]);
			}
		}

		// Wait for the next frame
		VIDEO_WaitVSync();

		// We return to the launcher application via exit
		if (pressed & WPAD_BUTTON_HOME)
		{
			break;
		}

		// Exit the program if the "Wii Menu" option is selected
		if (pressed & WPAD_BUTTON_A && selectedOption == 0)
		{
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
			printf("Exiting to Wii Menu...\n");
		}

		// Launch Neek2o if the "Neek2o" option is selected
		if (pressed & WPAD_BUTTON_A && selectedOption == 1)
		{
			MP3Player_Stop();
			WII_LaunchTitle(TITLE_ID(0x00010001,0x4e4b324f)); // Europe
		}

        // Launch Homebrew if the "HB" option is selected
        if (pressed & WPAD_BUTTON_A && selectedOption == 2)
        {
            exit(0);
        }

		// When B is pressed, stop the Music
		if(pressed & WPAD_BUTTON_B) {
			MP3Player_Stop();
		}


	}

	return 0;
}
