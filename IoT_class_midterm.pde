import processing.serial.*;
import gifAnimation.*;
Serial myPort;  // Create object from Serial class
int max_people = 5;  // maximum numbers of people can be detected
int package_size =(3*max_people+2)*3; // first 3 means (x,y,w). last 3 means three decimal digits.
String portName = "COM8";
int Baud = 115200;                    // set baud
Gif face_gif;

void setup(){
myPort = new Serial(this, portName, Baud);    // open Serial-port, and set baud
size(1920, 1080);                    // set frame size
face_gif = new Gif(this, "face.gif");
noStroke();                  
smooth();
}

void draw(){
    int count = 0;
    int []tmp = new int[package_size];      // packages received from the serial port
    
    /*RECEIVING PACKAGES*/
    while(myPort.available() > 0 && count<package_size)
    {
      int _tmp = myPort.read();
      if(_tmp != -1)
      {
        tmp[count] = _tmp;
        count++;
      }
    }
    
    int count2 = 0;
    int []val = new int[int(package_size/3)];  // Data decoded from the serial port
    /*DECODING PACKAGES to DECIMAL*/
    for(int i=0;i<package_size;i++)
    {
      if(i%3==2)
      {
        val[count2] = (tmp[i-2]-48)*100 + (tmp[i-1]-48)*10 + (tmp[i]-48); 
        //println(val[count2]);
        count2++;
      }
    }

  /*DRAW PEOPLE FACES*/
  if(val[int(package_size/3) -1]==255)    //stop bytes received
  {
    val[int(package_size/3) -1] = 0;
    background(255);
    for(int i=1;i<max_people;i++)
    {
      if(val[3*i]==0)                    // no face
        continue;
      float []_val = new float[3];
      _val[0] = (float)val[3*i-2];  // mid-x
      _val[1] = (float)val[3*i-1];  // mid-y
      _val[2] = (float)val[3*i];    // width
      print(_val[0]);
      print(", ");
      print(_val[1]);
      print(", ");
      println(_val[2]);
      
      image(face_gif, int(_val[0]*7.53-(_val[2]/2)*4.23), int(_val[1]*4.23-(_val[2]/2)*4.23), int(_val[2]*4.23), int(_val[2]*4.23));  // (img, x, y, w, h)
      
      ///*DRAW FACE*/
      //fill(245, 205, 152);
      //ellipse(_val[0], _val[1], _val[2], _val[2]);
      ///*DRAW EYES BALCK*/
      //fill(255, 255, 255);
      //ellipse(_val[0]-_val[2]/5, _val[1]-_val[2]/5, _val[2]/5, _val[2]/7);
      //ellipse(_val[0]+_val[2]/5, _val[1]-_val[2]/5, _val[2]/5, _val[2]/7);
      ///*DRAW EYES WHITE*/
      //fill(0, 0, 0);
      //ellipse(_val[0]-_val[2]/5, _val[1]-_val[2]/5, _val[2]/8, _val[2]/8);
      //ellipse(_val[0]+_val[2]/5, _val[1]-_val[2]/5, _val[2]/8, _val[2]/8);
      ///*DRAW NOSE*/
      //beginShape(TRIANGLE_STRIP);
      //fill(210, 170, 125);
      //vertex(_val[0]-_val[2]/10, _val[1]+_val[2]/8);
      //vertex(_val[0]+_val[2]/10, _val[1]+_val[2]/8);
      //vertex(_val[0], _val[1]-_val[2]/8);
      //endShape();
      ///*DRAW LIPS*/
      //beginShape(TRIANGLE_STRIP);
      //fill(210, 100, 105);
      //ellipse(_val[0]-_val[2]/6, _val[1]+_val[2]/4, _val[2]/3, _val[2]/18);
      //ellipse(_val[0]+_val[2]/6, _val[1]+_val[2]/4, _val[2]/3, _val[2]/18);
      //vertex(_val[0]-_val[2]/3, _val[1]+_val[2]/4);
      //vertex(_val[0]+_val[2]/3, _val[1]+_val[2]/4);
      //vertex(_val[0], _val[1]+_val[2]/3);
      //endShape();
    }
    face_gif.loop();
  }
}
