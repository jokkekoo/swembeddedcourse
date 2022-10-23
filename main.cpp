#include "EventFlags.h"
#include "Mutex.h"
#include "ThisThread.h"
#include "mbed.h"
#include "mstd_iterator"
#include <cstdint>
#include <string>
// Task in general
//#define DEBUG // If defined DEBUG is on! ..

/*
 *There are four tasks:
 *T1: GPS Parse - Parse (simplified) NMEA messages from serial port (UART) to get timestamp and GPS coordinates of a data point. Create separate parser function/library as in Exercise 4. More instructions further below.
 *Example message is "$GPGGA,134732.000,5540.3244,N,01231.2941,E", where the fields are:
 *$GPGGA - message identifier
 *134732.000 - timestamp in format hhmmss.000
 *5540.3244 - Latitude 
 *N - North hemisphere
 *01231.2941 - Longitude 
 *E - East hemisphere
 *
 *T2: Air Quality Sensor - Sensor values (data points) as randomized number between 0 and 100.
 *
 *T3: Data Aggregator - Create data point that has combined GPS information and air quality measurement.
 *
 *
 *T4: Data analysis - Analysis of data to get the minimum and maximum values for air quality, see debug interface below. T4 controls the execution of T3 with Events.
 */

// Debug Interface
/*
 *Debug Interface
 *The idea is the get, through the serial port, the minimum ja maximum values (so far) of air quality data and the number of data items handled so far (i.e. how many GPS messages were handled). The are three commands in the interface:
 *- "?min", return minimum air quality value
 *- "?max" return maximum air quality value
 *-"?cnt" return the number of handled GPS messages
 *
 *Below is an example code, how to print out floating point numbers to the serial port.
 *
 *You don't need to write test cases for the debug interface. 
 */
#define FLAG1 (0x01)
#define FLAG2 (0x02)
EventFlags event_flags;

typedef struct {
    double timestamp;
    double longitude;
    double latitude;
    double sensorvalue;
} data_msg_t;
CircularBuffer<data_msg_t, 10> data_a; // data_aggregator
// Data should be sent between threads using Queue
// hmm
Queue<double,20> timestampqueue;
Queue<double,20> longitudequeue;
Queue<double,20> latitudequeue;
Queue<double,20> sensorqueue;

Mutex muteksi;

/* You can start using the data structure in task T1. Fill out everything in except the sensor value that you receive from task T2 in task T3. 
 *Then task T3 does data point matching based on received values. Matching in simple, just one sensor value per timestamp and coordinate pair. 
 *Then deliver the filled data structure to task T4.
 */

// 4 Threads Check
Thread t1;
Thread t2;
Thread t3;
Thread t4;

double minSensorValue = 100;
double maxSensorValue = 0;
char command[100];
int command_count = 0;
bool new_command = false;
static UnbufferedSerial pc(USBTX, USBRX,9600);
int fields = 0;

void serial_rx_int() {
    char c = 0;
    if (pc.read(&c,1)){
        command[command_count] = c;
        command_count++;

        if (c == '\r'){
            new_command=true;
        }
    }
}

// ehkä Watchdog tuohon kun katsoo onko $GSPPA oikein.
// Note: atof and similar function don't detect overflows and return zero on error, so there's no way to know if it failed.
// you should use strtol for converting strings to int and strtod converting to double
//int parser(char *str){
    //data_msg_t data;
    //double testvariable = 0;
    //double *dubbelstet = NULL; 
    //char delim[] = ",";
    //int fields = 0;

    //if (command[0] == '?')
    //{
        //return 1;
    //}
    //str = strtok(command, delim);
    //while ( str != NULL) {
        //str = strtok(NULL, delim);
        //switch (fields) {
            //case 0:
                ////data.timestamp = atof(str);
                //testvariable = atof(str);ThisThread::sleep_for(50ms);
                ////timestampqueue.try_put(atof(&str));
                ////data_msg_t[1]
                //timestampqueue.try_put(&testvariable);
                //ThisThread::sleep_for(100ms);
                //timestampqueue.try_get(&dubbelstet);
                ////printf("Timestamp= %f\n", *dubbelstet);
                ////printf("Timestamp= %f\n", data.timestamp);
                //break;
            //case 1:
                ////testvariable = atof(str);
                ////longitudequeue.try_put(&testvariable);
                ////printf("Longitude= %f\n", data.longitude);
                //break;
            //case 3:
                ////data.latitude = atof(str);
                ////latitudequeue.try_put(str);
                ////printf("latitude= %f\n", data.latitude);
                //break;
            //default:
                //break;
        //}
        ////debug msg vois tulla tähän cnt, montako käsitelty? 
        //fields++;
    //}

    //// Palauttaa montako eriteltyä stringiä oli itse stringin sisällä eroteltu pilkulla ( , )
    //return fields;
//}
char gps[] = "$GPGGA";
char n[] = "N";
char e[] = "E";
int runcount = 0;

int parser(char *str) {
	data_msg_t data;
	char *token = NULL;
	char delim[] = ",";
	token = strtok(str, delim);
	int fields = 0;	
    double asdf = 1;
	if (strncmp(str, gps, 6) < 0 || strncmp(str,gps,6) > 0){ return 1;} 
	while ( token != NULL) {
		token = strtok(NULL, delim);
		switch(fields){
			case 0: // timestamp
				data.timestamp = atof(token);
				if ( data.timestamp < 0 ) { return 2;}
				break;
			case 1: // latitude
				data.latitude = atof(token);
				if ( (data.latitude / 100) < 0 || (data.latitude / 100) > 90 ) { return 3;}  
				
				break;
			case 2: // North hemisphere
				if ( strncmp(token, n, 1) == 0 ){
					break;
				}
				else {
					return 4;
				}
				
			case 3: // longitude
				data.longitude = atof(token);
                
                timestampqueue.try_put(&asdf);
				if ( (data.longitude / 100) < 0 || (data.longitude / 100) > 90 ) { return 5;}  
		
				break;
			case 4: // East hemisphere
				if (strncmp(token, e, 1) == 0){
					break;
				}
				else {
					return 6;
				}
			default:
				break;
		}	
		fields++;
	}
    data_a.push(data);
    runcount++;

	return fields;
}

/*T1: GPS Parse - Parse (simplified) NMEA messages from serial port (UART) to get timestamp and GPS coordinates of a data point. 
 Create separate parser function/library as in Exercise 4. More instructions further below.
 Example message is "$GPGGA,134732.000,5540.3244,N,01231.2941,E", where the fields are:
 $GPGGA - message identifier                menee pois
 34732.000 - timestamp in format hhmmss.000 case 0  
 5540.3244 - Latitude                       case 1
 N - North hemisphere                       case 2 # ei käyttöä
 01231.2941 - Longitude                     case 3
 E - East hemisphere                        case 4 # ei käyttöä
 */
void thread1() {
    // $GPGGA,134731.361,5540.3252,N,01231.2946,E
    //Pitää check että $GPGGA eikä esim $GPGGS
    // 6 eri tataa timestamp latitude longitude sensorvalue data_msg_t
    while (true) {  // exercise5 parsing input from serial port
        ThisThread::sleep_for(10ms);
        if (new_command == true){
            fields = parser(command);
            //printf("%d\n", fields);
            new_command = false;
            command_count = 0;
        }
    }
}

// T2: Air Quality Sensor - Sensor values (data points) as randomized number between 0 and 100.
void thread2() 
{
    double aqsensorValue = 0;
    while (true)
    {
        aqsensorValue = std::rand() % 100;
        sensorqueue.try_put(&aqsensorValue);
        //printf("sensorValue: %f\n",aqsensorValue);
        //ThisThread::sleep_for(50ms);
        ThisThread::yield();
    }
}

//T3: Data Aggregator - Create data point that has combined GPS information and air quality measurement.
// $GPGGA,134731.361,5540.3252,N,01231.2946,E
// $GPGGA,666666.361,5540.3252,N,01231.2946,E
// = Timestamp= 134731.361 Longitude= 5540.3252 Latitude= 01231.2946 Airquality= 666 ==> thread4
void thread3() {
    data_msg_t data;
    double *perhe = NULL;
    uint32_t flags_read = NULL;
    while (true) {
        flags_read = event_flags.wait_any(FLAG1);
        if (timestampqueue.empty() == false)
        {
            event_flags.clear(FLAG1);
            
            data_a.pop(data);

            //timestampqueue.try_get(&perse);
            //dm.timestamp = *perse;
            //longitudequeue.try_get(&perse);
            //dm.longitude = *perse;
            //latitudequeue.try_get(&perse);
            //dm.latitude = *perse;
            sensorqueue.try_get(&perhe);
            data.sensorvalue = *perhe;
            //data_a.push(dm);

            muteksi.lock();
            printf("Timestamp: %f\n", data.timestamp);
            printf("Longitude: %f\n", data.longitude);
            printf("Latitude: %f\n", data.latitude);
            printf("Sensorvalue: %f\n", data.sensorvalue);
            printf("\n");
            muteksi.unlock();
            //ThisThread::sleep_for(50ms);
            
            if (maxSensorValue < data.sensorvalue)
            {
                maxSensorValue = data.sensorvalue;
            }
            if (minSensorValue > data.sensorvalue)
            {
                minSensorValue = data.sensorvalue;
            }

            *perhe = NULL;
            flags_read = NULL;
            ThisThread::yield();
        }
    }
}

// Thread4 
void thread4()
{
    data_msg_t dm;
    while(true)
    {
        if (command[0] == '?')
        {
            if (command[1] == 'c')
            {
                printf("Run count: %d\n", runcount);
            }
            if (command[2] == 'i')
            {
                printf("Min sensor value: %f\n", minSensorValue);
            }
            if (command[3] == 'x')
            {
                printf("Max sensor value: %f\n", maxSensorValue);
            }
            command[0] = NULL;
        }
        if (new_command == true && command[0] != '\r')
        {
            ThisThread::sleep_for(50ms);
            event_flags.set(FLAG1);
        }
    }
}

int main() {
    
    std::srand(1);
    pc.format(8, SerialBase::None,1);
    pc.attach(serial_rx_int,SerialBase::RxIrq);

    t1.start(thread1);
    t2.start(thread2);
    t3.start(thread3);
    t4.start(thread4);

}
 
