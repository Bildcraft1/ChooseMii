#include <stdio.h>
#include <stdlib.h>
#include <gccore.h>
#include <asndlib.h>
#include <mp3player.h>
#include <wiiuse/wpad.h>
#include <unistd.h>

#include <fat.h>

#include <string.h>
#include <dirent.h>

#include "sample_mp3.h"
#define FILENAME "ChooseMii.txt"

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;
#define TITLE_ID(x, y) (((u64)(x) << 32) | (y))

int printConfig()
{
    FILE *fp;
    char line[256];

    // Mount the SD card
    if (!fatInitDefault())
    {
        printf("Failed to initialize FAT filesystem\n");
        return 1;
    }

    // Open the file for reading
    fp = fopen(FILENAME, "r");
    if (fp == NULL)
    {
        printf("Failed to open file %s\n", FILENAME);
        return 1;
    }

    // Read and print each line of the file
    while (fgets(line, sizeof(line), fp) != NULL)
    {
        printf("%s", line);
    }

    // Clean up
    fclose(fp);

    return 0;
}

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
	int numOptions = 4;
	char *options[] = {"Continue to Wii Menu", "Neek2o", "Homebrew", "USB LoaderGX"};
	bool blankMii = false;

	MP3Player_PlayBuffer(sample_mp3, sample_mp3_size, NULL);

	while (1)
	{
		// Call WPAD_ScanPads each loop, this reads the latest controller states
		if (!blankMii)
		{
			WPAD_ScanPads();
		}

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

		printf("  _____ __                       __  ___ _  _ \n");
		printf(" / ___// /  ___  ___   ___ ___  /  |/  /(_)(_) \n");
		printf("/ /__ / _ \\/ _ \\/ _ \\ (_-</ -_)/ /|_/ // // / \n");
		printf("\\___//_//_/\\___/\\___//___/\\__//_/  /_//_//_/ \n");
		printf("                                              \n");

		if (blankMii)
		{
			printf("BlankMii Mode Active");
		}
		else
		{

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
		}

		// Launch Neek2o if the "Neek2o" option is selected
		if (pressed & WPAD_BUTTON_A && selectedOption == 1)
		{
			MP3Player_Stop();
			WII_LaunchTitle(TITLE_ID(0x00010001, 0x4e4b324f)); // Europe
		}

		// Launch Homebrew if the "HB" option is selected
		if (pressed & WPAD_BUTTON_A && selectedOption == 2)
		{
			exit(0);
		}

		// Launch USB LoaderGX if the "Loader" option is selected
		if (pressed & WPAD_BUTTON_A && selectedOption == 3)
		{
			MP3Player_Stop();
			WII_LaunchTitle(TITLE_ID(0x00010001, 0x4944434c)); // Europe
		}

		// When B is pressed, stop the Music
		if (pressed & WPAD_BUTTON_B)
		{
			MP3Player_Stop();
		}

		if (pressed & WPAD_BUTTON_1)
		{
			MP3Player_Stop();
			WPAD_Shutdown();
			blankMii = true;
			printf("BlankMii Active, Restart your Wii");
		}

		if (pressed & WPAD_BUTTON_2)
		{
			MP3Player_Stop();
			
			// Clear the screen
			printf("\x1b[2J");
			
			printConfig();
		}
	}

	return 0;
}
