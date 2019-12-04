import processing.video.*;
import processing.serial.*;

Serial usb; // from arduino

Movie BGVideo;
Movie Video1;
Movie Video2;
Movie Video3;
Movie Video4;
Movie Video5;
void setup() {
  size(1280, 720);
  usb = connectUSBSerial(115200);

  BGVideo = new Movie(this, "ZXT_6250.mp4");
  Video1 = new Movie(this, "Video1.mp4");
  Video2 = new Movie(this, "Video2.mp4");
  Video3 = new Movie(this, "Video3.mp4");
  Video4 = new Movie(this, "Video4.mp4");
  Video5 = new Movie(this, "Video5.mp4");

  Video1.loop();
  Video2.loop();
  Video3.loop();
  Video4.loop();
  Video5.loop();

  //myMovie.play();
  BGVideo.loop();
  BGVideo.volume(1);
  BGVideo.speed(1);
  frameRate(24);
}

void draw() {
  image(BGVideo, 0, 0, 1280, 720);
  int which_wire = which_is_touched();

  char which_key;  
  if (keyPressed) { 
    which_key =  key;
  } else { 
    which_key = ' ';
  };

  if (which_key != ' ' || which_wire != -1) {
    print("Touch ");
    print(which_wire);
    println();

    if (which_key == 'a' || which_wire == 0) {
      image(Video1, 450, 150, 273, 429);
      Video1.loop();
    } else if (which_key == 's' || which_wire == 1) {
      image(Video2, 960, 0, 320, 240);
      Video2.loop();
    } else if (which_key == 'd' || which_wire == 2) {
      image(Video3, 0, 480, 320, 240);
      Video3.loop();
    } else if (which_key == 'f' || which_wire == 3) {
      image(Video4, 960, 480, 320, 240);
      Video4.loop();
    } else if (which_key == 'g' || which_wire == 4) {
      image(Video5, 480, 200, 320, 240);
      Video5.loop();
    }
  } else {
    Video1.stop();
    Video2.stop();
    Video3.stop();
    Video4.stop();
    Video5.stop();
  }



  //if (keyPressed == true) {
  //  image(topMovie, 560, 300, 320, 240);
  //  topMovie.loop();
  //} else {
  //  topMovie.pause();
  //}
}

int touch_reader_state =0;
int touch_reader_digit = -1;
boolean touch_reader_gotit = false;

int which_is_touched() {
  // the arduino sends 0+ for touch start, 0- for release
  // and 1+, .. 4+ etc
  // We return a number: 0..4 if that wire is being touched
  // We return -1 if no one is being touched
  char next_char; // someplace to read

  while (usb.available() > 1 ) {
    next_char = usb.readChar();
    print("  >[");
    print(touch_reader_state);
    print("] ");
    print(next_char);

    switch (touch_reader_state) {
    case 0 : // ready for digit
      if ( next_char < '0' || next_char > '9' ) { // further than I need
        touch_reader_state = 0;// bad char, so ignore
      } else {
        touch_reader_digit = next_char - '0';
        print("D=");
        print(touch_reader_digit);
        print(" ");
        // HACK: ignore everything but 1
        if (touch_reader_digit != 1) { 
          touch_reader_state = 0; 
          break;
        } // END HACK
        touch_reader_state = 1;
      }
      break;

    case 1 : // ready for +|-
      if ( next_char == '+') {
        touch_reader_gotit = true;
        print(" GOT ");
      } else if (next_char == '-' ) { 
        // got n-, so just set ourselves to "not touched"
        touch_reader_state = 0;// bad char, so ignore
        touch_reader_digit = -1;
        touch_reader_gotit = false;
        print(" REL ");
      } else {  
        print(" BAD ");print(next_char);print(" ");
        touch_reader_state = 0;// bad char, so ignore
      }
      break;
    }
    print(" S");
    print(touch_reader_state);
    println();
  }

  if (touch_reader_gotit) {
    //print (" on ");
    //println();
    return touch_reader_digit;
  } else {
    //println();
    return -1;
  }
}

void movieEvent(Movie m) {
  m.read();
}



//void draw() {
////  char object = Serial.readln();

////  switch (char) {
////    case "1+" :
////      start video for #1
////    case "1-" :
////      stop video for #1
////    case "2+" :
////      ...
////    otherwise:
////      play attractor video
////  }
////...
//}
