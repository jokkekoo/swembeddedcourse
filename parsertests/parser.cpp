#include <iostream>
#include "parser.h"
#include <cstring>
using namespace std;

typedef struct {
	double timestamp;
	double longitude;
	double latitude;
	double sensorvalue;
} data_msg_t;

char gps[] = "$GPGGA";
char n[] = "N";
char e[] = "E";

int parser(char *str) {
	data_msg_t data;
	char *token = NULL;
	char delim[] = ",";
	token = strtok(str, delim);
	int fields = 0;	
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
	return fields;
}


