let is_playing = false;  // Variable to keep track of the play/pause state
let is_stopped = true;
let is_animating = false // Variable to store the animation state of the CD head 
let total_duration = 0;  // Total duration of the audio in seconds
let elapsed = 0 // Variable to store the elapsed time in seconds 
let selected_cd = 0; // Variable to store the currently selected CD

const pbar_indicies = 69.0; // float to store the number of indicies in the progress bar. 

const seg_delay = 100; // Variable to store the time delay between segment switches on transitions

const progressBar = document.getElementById("progress-bar");
const elapsedTimeText = document.getElementById("elapsed-time");
const remainingTimeText = document.getElementById("remaining-time");
const cds = document.querySelectorAll('.cd')
const cdHead = document.getElementById('cd-head')
const cd_icons = [document.getElementById('cd1'), document.getElementById('cd2'), document.getElementById('cd3'), document.getElementById('cd4'), document.getElementById('cd5'), document.getElementById('cd6')];
const cd_title = document.getElementById("song-text")

// Helper function to format time (seconds -> mm:ss)
function formatTime(seconds) {
    const minutes = Math.floor(seconds / 60);
    const sec = Math.floor(seconds % 60);
    return `${minutes}:${sec < 10 ? '0' : ''}${sec}`;
}

// Update the elapsed and remaining time display
function updateTimeDisplay(current_time) {
    const elapsed = current_time
    elapsedTimeText.textContent = formatTime(Math.trunc(elapsed)); // Update elapsed time
}

// Play/Pause button handler
document.getElementById("play-pause").addEventListener("click", function() {
    is_stopped = false;
    const playPauseImg = document.getElementById("play-pause-img");
    if (is_playing == true) {
        is_playing = false;
        playPauseImg.src = "images/play.png";  // Switch back to Play image
        playPauseImg.alt = "Play";
        eel.play_pause_music();  // Call Python function to handle Pause
    } else {
        is_playing = true;
        playPauseImg.src = "images/pause.png";  // Switch to Pause image
        playPauseImg.alt = "Pause";
        eel.play_pause_music();  // Call Python function to handle Play
    }
});

// Loop Handler
document.getElementById("loop").addEventListener("click", function() {
    const loop_img = document.querySelector('.loop-img'); // Correct method to select by class
    // Check the current opacity and toggle it
    if(loop_img.opacity == '0.3') {loop_img.opacity = '1.0'}
    else {loop_img.opacity = '0.3'}
    // Call python function to toggle the looping function
    eel.toggle_loop()
});

// Shuffle Handler
document.getElementById("shuffle").addEventListener("click", function() {
    const shuffle_img = document.querySelector('.shuffle-img'); // Correct method to select by class
    // Check the current opacity and toggle it
    if (shuffle_img.opacity == '0.3') {shuffle_img.opacity = '1.0';} 
    else {shuffle_img.opacity = '0.3';}
    // Call python function to toggle the shuffling function
    eel.toggle_shuffle();
});

// Stop button handler
document.getElementById("stop").addEventListener("click", function() {
    if(is_stopped == false) {
        is_stopped = true;
        eel.stop_music();  // Call Python function to handle stop
        is_playing = false;
    }
});

// Fast Forward button handler
document.getElementById("fast-forward").addEventListener("click", function() {
    eel.fast_forward_music();  // Call Python function to handle Fast Forward
});

// Rewind button handler
document.getElementById("rewind").addEventListener("click", function() {
    eel.rewind_music();  // Call Python function to handle Rewind
});

// Progress bar (scrubbing)
progressBar.addEventListener("input", function() {
    const new_time = progressBar.value / pbar_indicies * total_duration;  // New position based on scrub
    eel.update_elapsed_time(new_time);  // Send new position to Python
});

// Python callable function to trigger play/pause
eel.expose(toggle_play_pause);
function toggle_play_pause() {
    is_stopped = false
    const playPauseImg = document.getElementById("play-pause-img");
    if (is_playing == true) {
        is_playing = false;
        playPauseImg.src = "images/play.png";  // Switch back to Play image
        playPauseImg.alt = "Play";
    } else {
        is_playing = true;
        playPauseImg.src = "images/pause.png";  // Switch to Pause image
        playPauseImg.alt = "Pause";
    }
}

// Python callable function to update the total playback duration
eel.expose(update_total_duration);
function update_total_duration(duration) {
    total_duration = duration;  // Store total duration in seconds
    remainingTimeText.textContent = `${formatTime(duration)}`;  // Update remaining time display
}

// Python callable function to update the total playback duration
eel.expose(update_cd_text);
function update_cd_text(title) {
    cd_title.textContent = `${title}`;  // Update remaining time display
}

// Python callable function to update the current playback position
eel.expose(update_elapsed_time);
function update_elapsed_time(current_time) {
    progressBar.value = Math.trunc(current_time / total_duration * pbar_indicies) // Update the progress bars position
    updateTimeDisplay(current_time);  // Update the time display based on current position
}

// Python callable function to handle playback end
eel.expose(on_playback_end);
function on_playback_end() {}

eel.expose(get_play_state);
function get_play_state(){
    if(is_playing == true) {return "1";}
    else {return "0";}
}

eel.expose(get_elapsed_time);
function get_elapsed_time(){
    return elapsed;
}

// Add click event listeners to each CD
cds.forEach(cd => {
    cd.addEventListener('click', function() {
        // Get the CD number from data attribute
        const cdNumber = this.getAttribute('data-cd');

        // Only call this if it is not current animating the object 
        if(!is_animating){
            // Call Python function with the selected CD number
            eel.cd_selected(cdNumber);

            // Move CD head
            move_cd_head(Number(cdNumber));
        }
    });
});

// Sends python the selected CD and handles animation interrupts 
eel.expose(move_cd_head);
function move_cd_head(cd) {

    // Check if the current_cd is 0, if it's 0 then no CD is currently selected
    //  - In this case, enable the opacity of the outer frame and the CD with a slow fade.
    //  - Set the current CD to the selected one
    //  - Make blocking command so the CD can't be switched until the animation is done

    // If the CD is not 0, first check if the selected CD is a greater or smaller number than the current cd - if its the same do nothing
    //  - If the CD is greater:
    //   - Fade out the current CD
    //   - Begin fading the leftmost segment out
    //   - Fade the segments in order left to right until you reach the selected CD, then just fade them all in
    //   - Fade in the selected CD
    //   - Update the currently selected CD
    
    //  - If the CD is smaller
    //   - do the opposite 

    if(selected_cd == 0) {
        // Block other animation calls
        is_animating = true;

        for(let i = 1; i <= 6; i++) {
            // Get each group of segments
            const temp_seg = document.getElementsByClassName(`H${cd}${i}`);

            // Loop through each element in the collection
            for (let j = 0; j < temp_seg.length; j++) {
                temp_seg[j].style.opacity = '1';
            }
        }
        // Now update the opacity of the CD itself
        cd_icons[cd - 1].style.opacity = '1';

        // Create a callback that updates the animation blocker once the transition is done
        cd_icons[cd - 1].addEventListener('transitionend', () => { 
            is_animating=false; 
        }, { once: true });

        // Update the selected cd
        selected_cd = cd;
    }
    else if(selected_cd != cd) {
        // Block other animation calls
        is_animating = true;

        // Now update the opacity of the CD itself
        cd_icons[selected_cd - 1].style.opacity = '0.3';

        // Varibale to store the time offset 
        let offset = 5000;

        // Variable to store the time delays (in ms)
        let delay = 500;

        // Check if the cd is greater than the currently selected cd
        if(selected_cd < cd) {
            // For each of the segments from the current one to the selected
            for(let i = selected_cd; i <= cd; i++) {

                // Hacky Jank
                if(i == selected_cd + 1) {
                    delay -= seg_delay * 6;
                }

                // For each segment in the 
                for(let j = 1; j <= 6; j++) {

                    // Get each group of segments
                    const temp_seg = document.getElementsByClassName(`H${i}${j}`);

                    // Different animations based on place asdlkfkfll;;;a;;a;askdk
                    if(i == selected_cd) {
                        // If this is the currently selected one, just turn the segments off

                        // Loop through each element in the collection
                        for (let n = 0; n < temp_seg.length; n++) {
                            // After delay, disable the opacity
                            setTimeout(function() {
                                temp_seg[n].style.opacity = '0.3';
                            }, offset + delay); 
                        }
                        // Add to delay
                        delay += seg_delay;

                    } else if(i != cd) {
                        // If this is an in between segment

                        // Loop through each element in the collection
                        for (let n = 0; n < temp_seg.length; n++) {
                            // After delay, enable the opacity
                            setTimeout(function() {
                                temp_seg[n].style.opacity = '1.0';
                            }, offset + delay); 

                            // After delay + extra time, disable the opacity
                            setTimeout(function() {
                                temp_seg[n].style.opacity = '0.3';
                            }, offset + delay + 250); 
                        }
                        // Add to delay
                        delay += seg_delay;
                    } else {
                        // If this is the last CD to reach

                        // Loop through each element in the collection
                        for (let n = 0; n < temp_seg.length; n++) {
                            // After delay, disable the opacity
                            setTimeout(function() {
                                temp_seg[n].style.opacity = '1.0';
                            }, offset + delay); 
                        }
                        // Add to delay
                        delay += seg_delay;                        
                    }
                }
            }

            // Now update the opacity of the new CD
            setTimeout(function() {
                cd_icons[cd - 1].style.opacity = '1';
            }, offset + delay); 
            
            // Create a callback that updates the animation blocker once the transition is done
            cd_icons[cd - 1].addEventListener('transitionend', () => { 
                is_animating=false; 
            }, { once: true });

            // Update the selected cd
            selected_cd = cd;

        } else if(selected_cd > cd) {
            // For each of the segments from the current one to the selected
            for(let i = selected_cd; i >= cd; i--) {

                // Hacky Jank
                if(i == selected_cd - 1) {
                    delay -= seg_delay * 6;
                }

                // For each segment in the 
                for(let j = 6; j >= 1; j--) {

                    // Get each group of segments
                    const temp_seg = document.getElementsByClassName(`H${i}${j}`);

                    // Different animations based on place asdlkfkfll;;;a;;a;askdk
                    if(i == selected_cd) {
                        // If this is the currently selected one, just turn the segments off

                        // Loop through each element in the collection
                        for (let n = 0; n < temp_seg.length; n++) {
                            // After delay, disable the opacity
                            setTimeout(function() {
                                temp_seg[n].style.opacity = '0.3';
                            }, offset + delay); 
                        }
                        // Add to delay
                        delay += seg_delay;

                    } else if(i != cd) {
                        // If this is an in between segment

                        // Loop through each element in the collection
                        for (let n = 0; n < temp_seg.length; n++) {
                            // After delay, enable the opacity
                            setTimeout(function() {
                                temp_seg[n].style.opacity = '1.0';
                            }, offset + delay); 

                            // After delay + extra time, disable the opacity
                            setTimeout(function() {
                                temp_seg[n].style.opacity = '0.3';
                            }, offset + delay + 250); 
                        }
                        // Add to delay
                        delay += seg_delay;
                    } else {
                        // If this is the last CD to reach

                        // Loop through each element in the collection
                        for (let n = 0; n < temp_seg.length; n++) {
                            // After delay, disable the opacity
                            setTimeout(function() {
                                temp_seg[n].style.opacity = '1.0';
                            }, offset + delay); 
                        }
                        // Add to delay
                        delay += seg_delay;                        
                    }
                }
            }

            // Now update the opacity of the new CD
            setTimeout(function() {
                cd_icons[cd - 1].style.opacity = '1';
            }, offset + delay); 
            
            // Create a callback that updates the animation blocker once the transition is done
            cd_icons[cd - 1].addEventListener('transitionend', () => { 
                is_animating=false; 
            }, { once: true });

            // Update the selected cd
            selected_cd = cd;
        }

    }
}



    /*
    // Get the position of the clicked CD
    const cdRect = cd.getBoundingClientRect();
    const containerRect = document.querySelector('.cd-container').getBoundingClientRect();

    // Calculate the position of the CD relative to the container
    const topPosition = cdRect.top - containerRect.top - 63;
    const leftPosition = cdRect.left - containerRect.left + 9;

    console.log(`${topPosition}px`);
    console.log(`${leftPosition}px`);

    // Don't interrupt animation
    if (!is_animating) {
        // Start animating
        is_animating = true;
        cdHead.style.top = `${topPosition}px`;
        cdHead.style.left = `${leftPosition}px`;

        // After the animation ends, set isAnimating to false
        cdHead.addEventListener('transitionend', () => {
            is_animating = false;
        }, { once: true });
    }
}*/