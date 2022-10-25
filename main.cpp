#include "EventFlags.h"
#include "Mutex.h"
#include "ThisThread.h"
#include "mbed.h"
#include "mstd_iterator"
#include <cstdint>
#include <string>

//#define DEBUG

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

Queue<double,20> timestampqueue;
Queue<double,20> longitudequeue;
Queue<double,20> latitudequeue;
Queue<double,20> sensorqueue;

Mutex muteksi;

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
char maxValue[80];
char minValue[80];

void serial_rx_int() {

    char c = 0;
    if (pc.read(&c,1)){
        command[command_count] = c;
        command_count++;

        if (command[command_count-1] == '\r'){
            new_command=true;
        }
    }
}

// ?cix

char gps[] = "$GPGGA";
char n[] = "N";
char e[] = "E";
int runcount = 0;

int parser(char *str) {
    ThisThread::sleep_for(20ms);
	data_msg_t data;
	char *token = NULL;
	char delim[] = ",";
    if (new_command == true){
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
                    //timestampqueue.try_put(&asdf);
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
    }
	return fields;
}

void thread1() {
    while (true) {
        ThisThread::sleep_for(10ms);
        if (new_command == true){
            fields = parser(command);
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
        ThisThread::yield();
    }
}

void thread3() {
    data_msg_t data;
    double *perhe = NULL;
    //uint32_t flags_read = NULL;
    while (true) {
        event_flags.wait_all(FLAG1);
        if (timestampqueue.empty() == false)
        {
            event_flags.clear(FLAG1);
            
            data_a.pop(data);

            sensorqueue.try_get(&perhe);
            data.sensorvalue = *perhe;
            
            if (maxSensorValue < data.sensorvalue)
            {
                maxSensorValue = data.sensorvalue;
                sprintf(maxValue,"%f : %f",data.timestamp,maxSensorValue);
            }
            if (minSensorValue > data.sensorvalue)
            {
                minSensorValue = data.sensorvalue;
                sprintf(minValue,"%f : %f",data.timestamp,minSensorValue);
            }

            *perhe = NULL;
            //flags_read = NULL;
            ThisThread::yield();
        }
    }
}
 // ?cix
void thread4()
{     
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
                printf("Min sensor value: %s\n", minValue);   
            }
            if (command[3] == 'x')
            {
                printf("Max sensor value: %s\n", maxValue);
            }
            command[0] = NULL;
        }
    ThisThread::sleep_for(100ms);
    event_flags.set(FLAG1);


        // if (new_command == true && command[0] != '\r' && command[0] == '$')
        //if (new_command == true)
        //{
        //    ThisThread::sleep_for(50ms);
        //    event_flags.set(FLAG1);
        //}
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
 
