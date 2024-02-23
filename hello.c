#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define RAND_MAXM 2500
#define RAND_MINM 500

#define l0Tringer "/sys/class/leds/beaglebone:green:usr0/trigger"  //file path for trigger file
#define l1Tringer "/sys/class/leds/beaglebone:green:usr1/trigger"  //none, heartbeat, timmer
#define l2Tringer "/sys/class/leds/beaglebone:green:usr2/trigger"
#define l3Tringer "/sys/class/leds/beaglebone:green:usr3/trigger"

#define l0DelayOn "/sys/class/leds/beaglebone:green:usr0/delay_on" //path file for delay on
#define l1DelayOn "/sys/class/leds/beaglebone:green:usr1/delay_on" //delay on in ms
#define l2DelayOn "/sys/class/leds/beaglebone:green:usr2/delay_on"
#define l3DelayOn "/sys/class/leds/beaglebone:green:usr3/delay_on"

#define l0DelayOff "/sys/class/leds/beaglebone:green:usr0/delay_off" //pathfile for delay off
#define l1DelayOff "/sys/class/leds/beaglebone:green:usr1/delay_off" //delay off in ms
#define l2DelayOff "/sys/class/leds/beaglebone:green:usr2/delay_off"
#define l3DelayOff "/sys/class/leds/beaglebone:green:usr3/delay_off"

#define l0Brightness "/sys/class/leds/beaglebone:green:usr0/brightness" //file path for brightness
#define l1Brightness "/sys/class/leds/beaglebone:green:usr1/brightness" ////0 off or 1 on
#define l2Brightness "/sys/class/leds/beaglebone:green:usr2/brightness"
#define l3Brightness "/sys/class/leds/beaglebone:green:usr3/brightness"

#define JSUP "/sys/class/gpio/gpio26/value" //filepath for press value of joystick
#define JSRT "/sys/class/gpio/gpio47/value" ////1 is not pressed 0 is pressed
#define JSDN "/sys/class/gpio/gpio46/value"
#define JSLFT "/sys/class/gpio/gpio65/value"

#define JSUPD "/sys/class/gpio/gpio26/direction"
#define JSRTD "/sys/class/gpio/gpio47/direction"
#define JSDND "/sys/class/gpio/gpio46/direction"
#define JSLFTD "/sys/class/gpio/gpio65/direction"

//snum of directions
enum JoystickDirection  {up, right, down, left, none};

//read from file code provided by Dr.Brian
int readFromFileToScreen(char *path)
{
    FILE *pFile = fopen(path, "r");
    if (pFile == NULL) {
        printf("ERROR: Unable to open file (%s) for read\n", path);
        exit(-1);
    }
    // Read string (line)
    const int MAX_LENGTH = 1024;
    char buff[MAX_LENGTH];
    fgets(buff, MAX_LENGTH, pFile);
    // Close
    fclose(pFile);
    return atoi(buff);
}

//gets the direction of the key pressed
static enum JoystickDirection getDirection(void)
{
    if(readFromFileToScreen(JSUP) == 0){
        return up;
    } else if (readFromFileToScreen(JSRT) == 0){
        return right;
    }   else if (readFromFileToScreen(JSDN) == 0){
        return down;
    }   else if (readFromFileToScreen(JSLFT) == 0){
        return left;
    } else{
        return none;
    }
}

//run command provided by Dr.Brian
static void runCommand(char* command)
{
    // Execute the shell command (output into pipe)
    FILE *pipe = popen(command, "r");
    // Ignore output of the command; but consume it
    // so we don't get an error when closing the pipe.
    char buffer[1024];
    while (!feof(pipe) && !ferror(pipe)) {
    if (fgets(buffer, sizeof(buffer), pipe) == NULL)
    break;
    // printf("--> %s", buffer); // Uncomment for debugging
    }
    // Get the exit code from the pipe; non-zero is an error:
    int exitCode = WEXITSTATUS(pclose(pipe));
    if (exitCode != 0) {
    perror("Unable to execute command:");
    printf(" command: %s\n", command);
    printf(" exit code: %d\n", exitCode);
    }
}

//time to ms provided by Dr.Brian
static long long getTimeInMs(void)
{
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    long long seconds = spec.tv_sec;
    long long nanoSeconds = spec.tv_nsec;
    long long milliSeconds = seconds * 1000
    + nanoSeconds / 1000000;
    return milliSeconds;
}

//sleep for x ms provided by Dr.Brian
static void sleepForMs(long long delayInMs)
{
    const long long NS_PER_MS = 1000 * 1000;
    const long long NS_PER_SECOND = 1000000000;
    long long delayNs = delayInMs * NS_PER_MS;
    int seconds = delayNs / NS_PER_SECOND;
    int nanoseconds = delayNs % NS_PER_SECOND;
    struct timespec reqDelay = {seconds, nanoseconds};
    nanosleep(&reqDelay, (struct timespec *) NULL);
}

//trigger led command taking file path and a string value used for gpio directions and trigger
static void pathToFileString(char *path, char *command) 
{
    FILE *pLedTriggerFile = fopen(path, "w");
    if (pLedTriggerFile == NULL) {
        printf("ERROR OPENING %s.\n", path);
        exit(1);
    }
    int charWritten = fprintf(pLedTriggerFile, "%s", command); // Fixed fprintf format
    if (charWritten <= 0) {
        printf("ERROR WRITING DATA\n");
        exit(1);
    }
    fclose(pLedTriggerFile);
}

//trigger led command taking file path and a string int. used for brightness and delay
static void pathToFileInt(char* path, int n)
{
    FILE *pLedTriggerFile = fopen(path, "w");
    if (pLedTriggerFile == NULL) {
        printf("ERROR OPENING %s.", path);
        exit(1);
    }
    int charWritten = fprintf(pLedTriggerFile,"%d",n);
    if (charWritten <= 0) {
    printf("ERROR WRITING DATA");
    exit(1);
    }
    fclose(pLedTriggerFile);
}

//function to return the personal best.
static long long personalBest(long long best, long long time ){
    if(best > time){
        printf("New best time!\n");
        return time;
    } else{
        return best;
    }
}

//funciton to flash led with input of time and hz
static void flashLED(int time, int hz)
{
    int delay = 1000/hz;
    pathToFileInt(l0Brightness, 0);
    pathToFileInt(l1Brightness, 0);
    pathToFileInt(l2Brightness, 0);
    pathToFileInt(l3Brightness, 0);

    pathToFileString(l0Tringer, "timer");
    pathToFileString(l1Tringer, "timer");
    pathToFileString(l2Tringer, "timer");
    pathToFileString(l3Tringer, "timer");

    sleepForMs(200);

    pathToFileInt(l0DelayOn, delay);
    pathToFileInt(l1DelayOn, delay);
    pathToFileInt(l2DelayOn, delay);
    pathToFileInt(l3DelayOn, delay);

    pathToFileInt(l0DelayOff, delay);
    pathToFileInt(l1DelayOff, delay);
    pathToFileInt(l2DelayOff, delay);
    pathToFileInt(l3DelayOff, delay);

    sleepForMs(time);
}

//function to set game up
static void settingGameUp(void)
{
    //setting up the joystick config
    runCommand("config-pin p8.43 gpio");
    pathToFileString(JSUPD, "in");
    pathToFileString(JSRTD, "in");
    pathToFileString(JSDND, "in");
    pathToFileString(JSLFTD, "in");

    //set led to none
    pathToFileString(l0Tringer, "none");
    pathToFileString(l1Tringer, "none");
    pathToFileString(l2Tringer, "none");
    pathToFileString(l3Tringer, "none");

    //setting first and last led to be off and middle on
    pathToFileInt(l0Brightness, 0);
    pathToFileInt(l1Brightness, 1);
    pathToFileInt(l2Brightness, 1);
    pathToFileInt(l3Brightness, 0);
}

//returns the led lights to system settings
static void returnOriginalBoardSettings(void)
{
    pathToFileString(l0Tringer, "heartbeat");
    pathToFileString(l1Tringer, "mmc0");
    pathToFileString(l2Tringer, "cpu0");
    pathToFileString(l3Tringer, "mmc1");
}

//set brightness to 0
static void setBrightnessOff(void)
{
        pathToFileInt(l0Brightness, 0);
        pathToFileInt(l1Brightness, 0);
        pathToFileInt(l2Brightness, 0);
        pathToFileInt(l3Brightness, 0);
}

int main(void)
{
    long long maxTime = 5000; //set max time
    long long fastestTime = maxTime; //set personal best to maxtime
    long long reactionTime = maxTime; //set reactiontime to maxtime
    long long startTime = 0;
    long long currentTime = 0; //set current time

    bool playing = false;
    bool waitingInput = false;
    bool waitingForLight = false;

    settingGameUp();

    printf("Hello embedded world, from Tegnoor Gill!\n\n");

    printf("How to play:\n");
    printf("Move the joystick UP if the UPPER light is lit\n");
    printf("Move the joystick DOWN if the LOWER light is lit\n");
    printf("Move the joystick LEFT or RIGHT to QUIT\n");
    
    //sleep for 5 seconds to give time for player to read how to play
    sleepForMs(maxTime);

    playing = true;

    while(playing){
        
        setBrightnessOff();
        
        sleepForMs(200);
        printf("Get ready...\n");

        float delay = (rand() % RAND_MAXM) + RAND_MINM;
        int light = rand() % 2;

        waitingInput = true;
        waitingForLight = true;

        startTime = getTimeInMs();

        //while waiting for random time from 0.5 to 3 s  it checks player inputs and then truns on light
        while(waitingForLight){
            currentTime = getTimeInMs()-startTime;
            if(currentTime>=delay){
                if (light == 1){
                    waitingForLight = false;
                    pathToFileInt(l0Brightness, 1);
                    printf("Press UP now!\n");
                }else{
                    waitingForLight = false;
                    pathToFileInt(l3Brightness, 1);
                    printf("Press DOWN now!\n");
                    
            }
            }else if(getDirection() == up || getDirection() == down){
                waitingForLight = false;
                waitingInput = false;
                printf("Too soon!\n");
            } else if(getDirection() == left || getDirection() == right){
                playing = false;
                waitingInput = false;
                waitingForLight = false;
                printf("User selected to quit.\n");
            }
        }

        startTime = getTimeInMs();
        //records reaction time and input if it was correct or not
        while(waitingInput){

            currentTime = getTimeInMs()-startTime;

            if(currentTime >= maxTime){
                
                printf("No input within 5000ms; quitting!\n");
                
                playing = false;
                waitingInput = false;

            } else if((light == 1 && getDirection() == up) || (light != 1 && getDirection() == down)){

                reactionTime = getTimeInMs() - startTime;
                fastestTime = personalBest(fastestTime, reactionTime);
                waitingInput = false;

                printf("Correct!\n");
                printf("Your reaction time was %lldms; best so far in game is %lldms.\n", reactionTime, fastestTime);

                flashLED(500,4);

            } else if((getDirection() == left) || (getDirection() == right)){
                
                playing = false;
                waitingInput = false;

                printf("User selected to quit.\n");

            } else if((light != 1 && getDirection() == up) || (light == 1 && getDirection() == down)){
                waitingInput = false;

                printf("Incorrect.\n");

                flashLED(1000,10);
   
            }
        }
    }
    returnOriginalBoardSettings();
    return 0;
}