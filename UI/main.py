import eel
import RPi.GPIO as GPIO
import serial
import vlc
import random
from pathlib import Path
import cdio, pycdio
import asyncio

GPIO.setmode(GPIO.BCM)
GPIO.setup(14, GPIO.OUT)

###########################################################################
#       Global Variables
# Variable to store the current position of the playback in seconds
cur_pos = -1

# Variable to store the current selected slot. 0 on initialization
cur_slot = 0

# Variable to store the state of playback. 
playing = False

# Variable to store if the carriage has been stopped
stopped = True

# Variable to store the current track number that is on
cur_track = 0

# Variable to store the length of the current track that is on
cur_track_len = 0

# Variable to store the VLC tracks array
tracks = []

# Variable to store the number of tracks on the current disc
num_tracks = 0

# Variable to track when to automatically switch to the next disc i.e. when a disc ends or you FF on the last song of a disc
next_disc = False

# Variable to store if we are shuffling
shuffling = False

# Variabel to store if we are looping
looping = False

# Variable to store the track title 
track_title = None

# Variable to store the serial port of the pi pico
ser = serial.Serial('/dev/ttyACM0', baudrate=115200, timeout=1)

# Variable to store the cd player object
instance = vlc.Instance()
drive = instance.media_player_new()
listplayer = instance.media_list_player_new()
listplayer.set_media_player(drive)

###########################################################################

###########################################################################
#       Functions that control the CD player

def wait_for_pico():
    # Waits for the move to be complete 
    res = b''
    while not res:
        if ser.in_waiting > 0:
            # Read in the serial buffer
            res = ser.read_all()
            # If the correct word is returned, continue
            if res.decode('utf-8') == 'done':
                break
            else:
                res = b''
        # Non blocking 100ms sleep
        eel.sleep(0.1)

def wait_for_drive():

    # Ensuring that local references actually call the global variables. 
    global instance
    global drive
    global listplayer
    global tracks
    global num_tracks
    global track_title
    global cur_track
    
    d = cdio.Device('/dev/sr0')
    
    while True:
        eel.sleep(0.25)
        try:
            temp = d.get_num_tracks() 
            num_tracks = temp
            break
        except:
            continue
    
    medialist = instance.media_list_new()
    
    for i in range(num_tracks):
        # Add all the tracks on the CD to the medialist
        temp_track = instance.media_new('cdda:///dev/sr0', (":cdda-track="+str(i)))
        medialist.add_media(temp_track)
        
        # Placeholder in case i ever figure out how to get the track name!
        tracks.append(f"Track!{i+1}") 
    
    listplayer.set_media_list(medialist)
    
    eel.sleep(0.2)
    
    track_title = tracks[cur_track - 1]
    eel.update_cd_text(format_title(track_title))


def cd_on():
    # Enables the GPIO on the CD player to turn it on
    GPIO.output(14, GPIO.HIGH)

def cd_off():
    # Disables the GPIO on the CD player to turn it off
    GPIO.output(14, GPIO.LOW)

def get_play_state():
    temp_state = eel.get_play_state()()
    return True if int(temp_state) == 1 else False

def raise_slot(slot):
    # First, Check if the slot is the current slot, if not, stop playback
    # Serial write the command to the pico, enable CD player GPIO, wait for valid VLC object

    # Ensuring that local references actually call the global variables. 
    global instance
    global cur_slot
    global playing
    global stopped
    global drive
    global listplayer
    global cur_track
    global cur_track_len
    global track_title
    global cur_pos

    # If a new slot is being selected
    if slot != cur_slot:
        # Stop the disc!
        if not stopped:
            print("I SHOULD NOT BE HERE BRUH!")
            playing = False
            stopped = True

            # Toggle the play/pause icons
            cur_state = get_play_state()
            if cur_state:
                eel.toggle_play_pause()
            # Stop actual playback (Hopefully spins disc down faster)
            try:
                listplayer.stop()
                eel.sleep(0.5)
                d = cdio.Device('/dev/sr0')
                d.eject_media()
                eel.sleep(5)
            except Exception as e:
                print(f"Error stopping drive! - \n{e}")

        # Disable the CD player
        cd_off()
        
        # Wait 500ms 
        eel.sleep(0.5)
        # Reset Pos
        cur_pos = 0
    
    # Send the command to the pico to move the carriage
    cur_slot = slot
    ser.write(bytes([cur_slot, 1]))

    # Waits for the move to be complete 
    wait_for_pico()
    # Turn on the CD player
    cd_on()

    # Waits for the drive to read the TOC on the disc.
    cur_track = 1
    wait_for_drive()
    eel.sleep(1)
    listplayer.play()
    eel.sleep(2)

    # Gets the length of the disc
    drive_len = listplayer.get_media_player().get_length()

    cur_track_len = int(drive_len / 1000)

    # Update the duration and toggle the playstate
    eel.update_total_duration(cur_track_len)
    # Toggle the play/pause icons
    cur_state = get_play_state()
    print(f'CUR STATE ON RAISE {True if cur_state == True else False}')
    if not cur_state:
        print("TOGGLING JAWN")
        eel.toggle_play_pause()

    # Update state variables
    stopped = False
    playing = True


def lower_slot():
    # disable CD player GPIO, Serial write the command to the pico, P.S. lowers on currently selected slot
    
    # Ensuring that local references actually call the global variables. 
    global cur_slot
    global playing
    global stopped
    global listplayer
    global drive
    global instance
    
    # If the track is currently playing or paused
    if not stopped:
        stopped = True
        # Stop actual playback (Hopefully spins disc down faster)
        listplayer.stop()
        eel.sleep(0.5)
        d = cdio.Device('/dev/sr0')
        d.eject_media()
        eel.sleep(5)

    cd_off()
    # Update the current track text
    eel.update_cd_text(format_title('!'))
    # Wait 500ms 
    eel.sleep(0.5)
    
    # Send the command to the pico to move the carriage
    ser.write(bytes([cur_slot, 0]))
    # Waits for the move to be complete 
    wait_for_pico()


def format_title(title):
    if len(title) > 20:
        # Trim the string to the desired length
        return title[:20]
    else:
        # Center the string and fill with the specified character
        return title.center(20, '!')


###########################################################################

# Variable to update the global playing  from Lambda function from JS
def update_playing(state):
    global playing
    playing = state

# Initialize the eel app with the "web" folder
eel.init('web', allowed_extensions=['.js', '.html', '.css'])

# Empty Python function for Play/Pause
@eel.expose
def play_pause_music():
    print("Play/Pause button pressed")

    global playing
    global stopped
    global drive
    global listplayer
    global cur_slot

    playing = get_play_state()
    if playing: 
        # If stopped, raise then play, if just paused resume playback
        if stopped:
            raise_slot(cur_slot)
        else:
            listplayer.set_pause(0)
    else:
        # If playing (not stopped) pause playback
        if not stopped:
            print('pausing!')
            listplayer.set_pause(1)


# Empty Python function for Play/Pause
@eel.expose
def stop_music():
    print("Stop button pressed")

    # Ensuring that local references actually call the global variables. 
    global playing
    global cur_pos
    global cur_track
    global cur_track_len
    global tracks
    global num_tracks

    # Update the playing and progress bar states
    playing = False

    cur_pos = 0
    eel.update_elapsed_time(0)

    if not stopped:
        cur_track = 1
        cur_track_len = -1
        tracks = []
        num_tracks = -1
        # Lower the carriage in the current slot
        lower_slot()
        eel.toggle_play_pause()

# Empty Python function for Fast Forward
@eel.expose
def fast_forward_music():
    print("Fast Forward button pressed")
    # Implement your fast forward logic here

    global cur_track
    global cur_pos
    global num_tracks
    global drive
    global listplayer
    global playing
    global next_disc
    global track_title

    if playing: 
        # If this isn't the last disc
        if cur_track < num_tracks:
            # Set the playback position to 0 
            cur_pos = 0
            # Move to the next track!
            listplayer.next()
            eel.sleep(1)

            drive_len = listplayer.get_media_player().get_length()

            cur_track_len = int(drive_len / 1000)

            # Update the duration and toggle the playstate
            eel.update_total_duration(cur_track_len)

            track_title = tracks[cur_track]
            eel.update_cd_text(format_title(track_title))
            cur_track += 1
        else:
            # Trigger automatically switching to the next disc
            next_disc = True


# Empty Python function for Rewind
@eel.expose
def rewind_music():
    print("Rewind button pressed")
    # Implement your rewind logic here

    # if current track is not the first, go back 1

    global playing
    global cur_pos
    global drive
    global cur_track
    global listplayer
    global track_title

    if playing: 
        if cur_track > 1:
            cur_pos = 0
            listplayer.previous()
            eel.sleep(1)

            drive_len = listplayer.get_media_player().get_length()

            cur_track_len = int(drive_len / 1000)

            # Update the duration and toggle the playstate
            eel.update_total_duration(cur_track_len)

            track_title = tracks[cur_track-2]
            eel.update_cd_text(format_title(track_title))
            cur_track -= 1

# Python function called when shuffling is toggled
@eel.expose
def toggle_shuffle():
    global shuffling
    if shuffling:
        shuffling = False
    else:
        shuffling = True

# Python function called when looping is toggled
@eel.expose
def toggle_loop():
    global looping
    if looping:
        looping = False
    else:
        looping = True

# Python function to handle scrubbing to a new position
@eel.expose
def update_elapsed_time(current_time):
    print(f"Scrubbed to new position: {current_time}")
    # Implement your logic to update the playback position here
    global cur_pos
    global listplayer
    global drive

    cur_pos = int(current_time)
    listplayer.get_media_player().set_time(cur_pos*1000)
    eel.sleep(0.1)

# Python function to update the total duration of the audio
@eel.expose
def update_total_duration(duration):
    print(f"Total duration set to: {duration}")
    # Store the total duration to manage the progress bar

# Python function called when playback ends
@eel.expose
def on_playback_end():
    print("Playback has ended")
    # Implement logic to handle when the track ends
    # Not sure anything actually needs to be done here... 

# Python function called when a CD is selected
@eel.expose
def cd_selected(cd_number):
    print(f"CD {cd_number} selected")
    
    # NOTE: Logic to prevent selecting the current slot is on the JS side

    # Here implement the logic of actually switching the CD SO:
    """
    - If there is currently a CD player, Stop playback
    - Disable the CD player GPIO
    - Send Serial command to move to the selected slot 
    - Wait for the carriage to reach the slot blocking ! ! !
    - Enable GPIO
    - Wait for TOC on disc to be read so media player can be set up  <---- NOTE: Up to here is handeled by the 'raise_slot' function
    - Set stopped to false
    - Read in NEW TOC info 
    - Begin playback
    - Update the playback duration
    """
    raise_slot(int(cd_number))



if __name__ == "__main__":
    # Step 1 is initialize the CD player, don't even enable the UI until it has calibrated the carriage. 

    #CARRIAGE CALIBRATION HERE
    # Waits for the init to be complete 
    res = b''
    while not res:
        if ser.in_waiting > 0:
            # Read in the serial buffer
            res = ser.read_all()
            # If the correct word is returned, continue
            if res.decode('utf-8') == 'init':
                break
            else:
                res = b''
        # Non blocking 100ms sleep
        eel.sleep(0.1)

    # Command-line options for full-screen kiosk mode
    options = [
        '--kiosk',          # Full-screen mode
        '--disable-infobars',  # Remove info bars
    ]

    # Start the eel application (opens a window with the UI)
    eel.start('index.html', mode='chrome', size=(1024, 600), block=False, cmdline_args=options)
    eel.sleep(1)

    """
    UI Control logic:

    - If not next_disc 
        - If playing 
            - Temp variable to grab the time from the drive. 
            - Check if that temp variable is less than cur_pos
            - If it is, the track has progressed forward one
                - If on the last track on the disc, Go into next_disc mode
                - Else, Add 1 to the current track, Grab new track info 
            - Else, we're still on the same disc
                - Set cur pos to the temp variable
                - eel.update_elapsed_time(cur_pos)
    - Else, we need to switch discs. 
        - If Loop keep incrementing 1 then roll over back to 1 at 6
        - If Shuffle, Randomly pick another disc that is not the current one
        - If neither, Stop music

    """
    while True:
        eel.sleep(0.5)
        if not next_disc:
            if playing:
                temp_time = listplayer.get_media_player().get_time()
                temp_time = temp_time // 1000 if temp_time != -1 else 0
                
                # If the queried time is less than the current pos, the next track is being player
                if temp_time < cur_pos:
                    cur_pos = 0
                    # If the current track is not the last track
                    if cur_track < num_tracks:
                        # Increment the current track
                        cur_track += 1

                        eel.sleep(1)

                        drive_len = listplayer.get_media_player().get_length()

                        cur_track_len = int(drive_len / 1000)

                        # Update the duration and toggle the playstate
                        eel.update_total_duration(cur_track_len)

                        track_title = tracks[cur_track - 1]
                        eel.update_cd_text(format_title(track_title))
                    # Else, this is the last track on the CD, so handle the end of the CD
                    else:
                        next_disc = True
                else:
                    cur_pos = temp_time
                    eel.update_elapsed_time(cur_pos)
        else:            
            if shuffling:
                # Initializing list
                slots = [1,2,3,4,5,6]
                
                # Generate random number except cur_slot in list
                # using choice() + list comprehension
                # Idea taken from https://www.geeksforgeeks.org/python-generate-random-number-except-k-in-list/
                shuf_slot = random.choice([s for s in slots if s != cur_slot])
                raise_slot(shuf_slot)
            elif looping:
                loop_slot = cur_slot
                loop_slot += 1
                if loop_slot > 6:
                    loop_slot = 1

                raise_slot(loop_slot)
            else:
                stop_music()

            next_disc = False





