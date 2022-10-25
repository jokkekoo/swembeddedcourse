# Only checking run count and test should be run 
*** Settings ***

Library  SerialLibrary    encoding=ascii
Library  String

*** Variables ***

${com}   /dev/ttyACM0
${board}   NUCLEO
${count}   Run count:${SPACE} 

*** Test Cases ***
Connect Serial
	Log To Console  Connecting to ${board}
	Add Port  ${com}  baudrate=9600   encoding=ascii
	Port Should Be Open  ${com}
	Reset Input Buffer
	Reset Output Buffer

Check Starting point
	Log To Console 	Checking current count
	Write Data  ?cix\r
	Sleep  1s
	${read} =  Read Until  \n
	@{list_string} =  split string  ${read}  ${SPACE}
	Log To Console  ${list_string}[2]
	Should Be Equal As Strings  ${read.replace('\n','').strip()}  ${count}${list_string}[2]  strip_spaces=True 
	Log To Console  ${read}
	Reset Input Buffer
	Reset Output Buffer

Write Wrong GPS Identifier Data To Serial
	Write Data  $GPAAA,134731.361,5540.3252,N,01231.2946,E\r
	Reset Input Buffer
	Reset Output Buffer
	Sleep  1s
	Write Data  $GPGGA,-5554731.361,5540.3252,N,01231.2946,E\r
	Reset Input Buffer
	Reset Output Buffer
	Sleep  1s
	Write Data  $GPGGA,134731.361,12151540.3252,N,01231.2946,E\r
	Reset Input Buffer
	Reset Output Buffer
	Sleep  1s
	Write Data  $GPGGA,134731.361,5540.3252,XXXX,01231.2946,E\r
	Reset Input Buffer
	Reset Output Buffer
	Sleep  1s
	Write Data  $GPGGA,134731.361,5540.3252,N,5115511231.2946,E\r
	Reset Input Buffer
	Reset Output Buffer
	Sleep  1s
	Write Data  $GPGGA,134731.361,5540.3252,N,01231.2946,XXXX\r
	Reset Input Buffer
	Reset Output Buffer
	Sleep  1s

Check if Wrong GPS Data Went Through 
	Log To Console  Count should remain same as in starting point
	Write Data  ?cix\r
	Sleep  1s
	${read} =  Read Until  \n
	@{list_string} =  split string  ${read}  ${SPACE}
	Should Be Equal As Strings  ${read.replace('\n','').strip()}  ${count}${list_string}[2]  strip_spaces=True
	Log To Console  ${read}
	Reset Input Buffer
	Reset Output Buffer

Write Correct GPS Data To Serial
	Write Data  $GPGGA,134731.631,5540.3244,N,01231.2941,E\r
	Sleep  1s
	Reset Input Buffer
	Reset Output Buffer

Check if Correct Data Went Through
	Log To Console  Count should increment by 1
	Write Data  ?cix\r
	Sleep  1s
	${read} =  Read Until  \n
	@{list_string} =  split string  ${read}  ${SPACE}
	Should Be Equal As Strings  ${read.replace('\n','').strip()}  ${count}${list_string}[2]  strip_spaces=True
	Log To Console  ${read}
	Reset Input Buffer
	Reset Output Buffer

Disconnect Serial
	Log To Console  Disconnecting ${board}
	[TearDown]  Delete Port  ${com}

*** Keywords ***

