#include "mbed.h"
#include "MQTTEthernet.h"
#include "MQTTClient.h"
#include "picojson.h"
#include <iostream>
#include <string>
#include <fstream>
#include "WeatherMeters.h"
#include "DHT.h"
#include "SRF05.h"

BusOut leds(LED4, LED3, LED2, LED1);
void sendMQTT(char* topicName, MQTT::Client<MQTTEthernet, Countdown> client, MQTT::Message message);

/********* Variables ***********/
int numSensors = 0;
char server[20], user[20], pass[20];
int port;
char *cs;
int update=0;
Timeout timeWind;

float humedad,temperatura;

void **objectsSensors;
char **topicsMQTT;
char **typesObjects;

MQTT::Message message;

void errorConfig(int led){
	int aux=1;

	while(1){
		if(aux){
			leds=led;
		}else{
			leds=0;
		}
		aux = !aux;
		wait(0.25);
	}
}
void ReadFile (void){
	LocalFileSystem local("local");
	char A[1024];
	string s;
	const char *sChar;

	printf("Reading files1  "__TIME__"\r\n");
	FILE *set = fopen("/local/config.txt", "r");  // Open "config.txt" on the local file system for read
	if(set==NULL){
		errorConfig(1);
	}
	while(fgets(A,sizeof(A),set)!=NULL){
		s+=A;
	}
	printf("%s \n",s.c_str());
	sChar=s.c_str();
	cs = (char*)malloc(strlen(sChar)+1);
	strcpy(cs, sChar);
	fclose(set);
}
void parse() {
	PinName pines[35] = {LED1,LED2,LED3,LED4,p5, p6, p7, p8, p9, p10, p11, p12, p13, p14, p15, p16, p17, p18, p19, p20, p21, p22, p23, p24, p25, p26, p27, p28, p29, p30};
	int cont=0,p;
	const char *sensor;
	const char *sChar;

	picojson::value v;
	string err = picojson::parse(v, cs, cs + strlen(cs));
	printf("res error? %s\r\n", err.c_str());
	if(!err.empty()){
		errorConfig(2);
		//PwmOut led(p20);
	}
	picojson::array list = v.get("configSensors").get<picojson::array>();
	numSensors=list.size();
	printf("Numero de objetos en JSON %d \r\n", numSensors);
	objectsSensors = (void** )calloc(numSensors,sizeof(void *));
	topicsMQTT = (char** )calloc(numSensors,sizeof(char*));
	typesObjects = (char** )calloc(numSensors,sizeof(char*));

	if(v.get("server").get<string>().size()==0 || v.get("server") == NULL
			|| (int)v.get("port").get<double>()==0 || (int)v.get("port")== NULL
			|| v.get("user").get<string>().size()==0 || v.get("user") == NULL
			|| v.get("pass").get<string>().size()==0 || v.get("pass") == NULL){

		printf("Server, port, user, pass fican mal no ficheiro JSON \r\n");
		errorConfig(3);
		//PwmOut led(p20);
	}
	strcpy(server,v.get("server").get<string>().c_str());
	printf("Server %s\r\n", server);
	port = (int)v.get("port").get<double>();
	printf("Puerto %d\r\n", port);
	strcpy(user,v.get("user").get<string>().c_str());
	printf("User %s\r\n", user);
	strcpy(pass,v.get("pass").get<string>().c_str());
	printf("Pass %s\r\n", pass);
	printf("Numero de objetos en JSON %d \r\n", list.size());

	for (picojson::array::iterator iter = list.begin(); iter != list.end(); ++iter){
		if((*iter).get("sensor").get<string>().size()==0 || (*iter).get("topic").get<string>().size()==0 || (int)(*iter).get("pin").get<double>()==0
				|| (*iter).get("sensor")==NULL || (*iter).get("topic")==NULL || (int)(*iter).get("pin")==NULL){

			printf("Sensor, topic, or pin fican mal no ficheiro JSON \r\n");
			errorConfig(4);
			//PwmOut led(p20);
		}
		sensor= (*iter).get("sensor").get<string>().c_str();
		sChar = (*iter).get("topic").get<string>().c_str();
		p = (int)(*iter).get("pin").get<double>();
		if(strcmp(sensor,"windspeed")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			objectsSensors[cont] = new CAnemometer(pines[p-1]);
			strcpy(typesObjects[cont], sensor);
			printf("Dentro de windSpeed %s \r\n", topicsMQTT[cont]);

			cont++;
		}else if(strcmp(sensor,"winddirection")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			objectsSensors[cont] = new CWindVane(pines[p-1],220);
			strcpy(typesObjects[cont], sensor);
			printf("%s \r\n", topicsMQTT[cont]);

			cont++;
		}else if(strcmp(sensor,"raingauge")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			objectsSensors[cont] = new CRainGauge(pines[p-1]);
			strcpy(typesObjects[cont], sensor);
			printf("%s \r\n", topicsMQTT[cont]);

			cont++;
		}else if(strcmp(sensor,"temperature")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			objectsSensors[cont] = new DHT(pines[p-1],DHT22);
			strcpy(typesObjects[cont], sensor);
			printf("%s \r\n", topicsMQTT[cont]);

			cont++;
		}else if(strcmp(sensor,"humidity")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			objectsSensors[cont] = new DHT(pines[p-1],DHT22);
			strcpy(typesObjects[cont], sensor);
			printf("%s \r\n", topicsMQTT[cont]);

			cont++;
		}else if(strcmp(sensor,"soilHumidity")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			objectsSensors[cont] = new AnalogIn(pines[p-1]);
			strcpy(typesObjects[cont], sensor);
			printf("%s \r\n", topicsMQTT[cont]);

			cont++;
		}else if(strcmp(sensor,"aIn")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			objectsSensors[cont] = new AnalogIn(pines[p-1]);
			strcpy(typesObjects[cont], sensor);
			printf("%s \r\n", topicsMQTT[cont]);

			cont++;
		}else if(strcmp(sensor,"dOut")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			objectsSensors[cont] = new DigitalOut(pines[p-1]);
			strcpy(typesObjects[cont], sensor);
			printf("%s \r\n", topicsMQTT[cont]);

			cont++;
		}else if(strcmp(sensor,"sonar")==0){
			topicsMQTT[cont] = (char*)malloc(strlen(sChar)+1);
			typesObjects[cont] = (char*)malloc(strlen(sensor)+1);
			strcpy(topicsMQTT[cont], sChar);
			int pAux = (int)(*iter).get("pin2").get<double>();
			objectsSensors[cont] = new SRF05(pines[p-1],pines[pAux-1]);
			strcpy(typesObjects[cont], sensor);
			printf("%s \r\n", topicsMQTT[cont]);

			cont++;
		}else{
			errorConfig(5);
		}
	}
	printf("Llega Ultimo for \r\n");

	if(cont!=numSensors){
		printf("Error a la lectura de sensores %d <--------> %d\r\n",numSensors,cont);
		numSensors=cont;

	}

	int auxiliar;
	for(auxiliar=0;auxiliar<numSensors;auxiliar++){
		printf("Sensor: %s ->%s \r\n",typesObjects[auxiliar], topicsMQTT[auxiliar]);
	}
}
/*
 * Funcion que es llamada cada 3 segundos para activar variable update,
 * esta variable es un semaforo para actualicar datos de sensores.
 * */
void timer_sensor(){
	update=1;
	timeWind.attach(&timer_sensor,3);
}
void updateSensorData(MQTT::Client<MQTTEthernet, Countdown> client){

	int state;
	char intStr[30];

	float dataFloat/*,humedad,temperatura*/;
	int dataInt;
	const char* direcction;
	float rain;

	for (int i = 0; i < numSensors; ++i) {

		if(strcmp(typesObjects[i],"windspeed")==0){
			CAnemometer* anemometer = (CAnemometer*)objectsSensors[i];
			dataFloat=anemometer->GetCurrentWindSpeed();
			printf("Tomando dato %s \r\n", typesObjects[i]);
			printf("Velocidad: %d.%01d \r\n", (uint32_t)dataFloat,(uint32_t)((dataFloat * 1000) - (uint32_t)(dataFloat) * 1000));
			sprintf (intStr, "%d.%01d", (uint32_t)dataFloat,(uint32_t)((dataFloat * 1000) - (uint32_t)(dataFloat) * 1000));
			message.payload = (void*)intStr;
			message.payloadlen = strlen(intStr)+1;

			sendMQTT(topicsMQTT[i],client,message);
		}
		if(strcmp(typesObjects[i],"winddirection")==0){
			CWindVane* windVane = (CWindVane*)objectsSensors[i];
			direcction=windVane->GetWindDirectionAsString();
			printf("Tomando dato %s \r\n", typesObjects[i]);
			printf("Direccion: %s \r\n",direcction);
			message.payload = (void*)direcction;
			message.payloadlen = strlen(direcction)+1;

			sendMQTT(topicsMQTT[i],client,message);
		}
		if(strcmp(typesObjects[i],"raingauge")==0){
			CRainGauge* raingauge = (CRainGauge*)objectsSensors[i];
			rain=raingauge->GetRainfall();
			raingauge->Reset();
			printf("Tomando dato %s \r\n", typesObjects[i]);
			printf("Lluvia: %d.%01d \r\n", (uint32_t)rain,(uint32_t)((rain * 1000) - (uint32_t)(rain) * 1000));
			sprintf (intStr, "%d.%03d", (uint32_t)rain,(uint32_t)((rain * 1000) - (uint32_t)(rain) * 1000));
			message.payload = (void*)intStr;
			message.payloadlen = strlen(intStr)+1;

			sendMQTT(topicsMQTT[i],client,message);

		}
		if((strcmp(typesObjects[i],"temperature")==0) || (strcmp(typesObjects[i],"humidity")==0)){
			DHT* sensorTemp = (DHT*)objectsSensors[i];
			printf("Tomando dato %s \r\n", typesObjects[i]);
			/* Read the measured results */
			state = sensorTemp->readData();
			/* Show the data, otherwise error */
			if (state!=0) {
				printf("Error: %d \r\n", state);
			} else {
				temperatura = sensorTemp->ReadTemperature(CELCIUS);
				humedad=sensorTemp->ReadHumidity();
			}
			if (strcmp(typesObjects[i],"temperature")==0) {
				sprintf (intStr, "%d.%01d", (uint32_t)temperatura,(uint32_t)((temperatura * 1000) - (uint32_t)(temperatura) * 1000));
				message.payload = (void*)intStr;
				message.payloadlen = strlen(intStr)+1;

				sendMQTT(topicsMQTT[i],client,message);
			}
			if(strcmp(typesObjects[i],"humidity")==0){
				sprintf(intStr, "%d",(int)humedad);
				message.payload = (void*)intStr;
				message.payloadlen = strlen(intStr)+1;

				sendMQTT(topicsMQTT[i],client,message);
			}
		}
		if(strcmp(typesObjects[i],"soilHumidity")==0){
			AnalogIn* moistureSensor = (AnalogIn*)objectsSensors[i];
			dataInt = moistureSensor->read()*1000;
			//dataFloat = moistureSensor->read();
			//dataFloat = moistureSensor->read();
			printf("Tomando dato %s \r\n", typesObjects[i]);
			printf("H_suelo: %d \r\n", dataInt);
			//sprintf (intStr, "%d.%03d", (uint32_t)dataFloat,(uint32_t)((dataFloat * 1000) - (uint32_t)(dataFloat) * 1000));
			sprintf(intStr,"%d", dataInt);
			message.payload = (void*)intStr;
			message.payloadlen = strlen(intStr)+1;

			sendMQTT(topicsMQTT[i],client,message);
		}
		if(strcmp(typesObjects[i],"aIn")==0){
			AnalogIn* moistureSensor = (AnalogIn*)objectsSensors[i];
			dataInt = moistureSensor->read_u16();
			printf("Tomando dato %s \r\n", typesObjects[i]);
			printf("Claridad luz: %d \r\n", dataInt);
			sprintf(intStr,"%d", dataInt);
			//dataFloat = moistureSensor->operator float();
			//sprintf (intStr, "%d.%03d", (uint32_t)dataFloat,(uint32_t)((dataFloat * 1000) - (uint32_t)(dataFloat) * 1000));
			message.payload = (void*)intStr;
			message.payloadlen = strlen(intStr)+1;

			sendMQTT(topicsMQTT[i],client,message);
		}
		if(strcmp(typesObjects[i],"dOut")==0){
			DigitalOut* pinDigital = (DigitalOut*)objectsSensors[i];
			dataInt = pinDigital->read();
			//	printf("Tomando dato %s \r\n", typesObjects[i]);
			//	printf("Claridad luz: %d \r\n", moisture);
			sprintf(intStr,"%d", dataInt);
			message.payload = (void*)intStr;
			message.payloadlen = strlen(intStr)+1;

			sendMQTT(topicsMQTT[i],client,message);
		}
		if(strcmp(typesObjects[i],"sonar")==0){
			SRF05* sonar = (SRF05*)objectsSensors[i];
			float sonar1 =sonar->read();
			sprintf (intStr, "%d.%03d", (uint32_t)sonar1,(uint32_t)((sonar1 * 1000) - (uint32_t)(sonar1) * 1000));
			message.payload = (void*)intStr;
			message.payloadlen = strlen(intStr)+1;

			sendMQTT(topicsMQTT[i],client,message);
		}
	}
}
void sendMQTT(char* topicName, MQTT::Client<MQTTEthernet, Countdown> client, MQTT::Message message){
	client.publish(topicName,message);
}
void messageArrived(MQTT::MessageData& md)
{
	MQTT::Message &message = md.message;
	for (int rc = 0; rc < numSensors; ++rc) {
		if(strcmp(typesObjects[rc],"dOut")==0){
			if(strncmp(md.topicName.lenstring.data,topicsMQTT[rc],strlen(topicsMQTT[rc]))==0){
				DigitalOut* pinDigital = (DigitalOut*)objectsSensors[rc];
				if(atoi((char*)message.payload)==1)
					pinDigital->write(1);
				if(atoi((char*)message.payload)==0)
					pinDigital->write(0);
			}
		}
	}
}
void subscribeTopics(MQTT::Client<MQTTEthernet, Countdown> *client){
	for (int rc = 0; rc < numSensors; ++rc) {
		if((strcmp(typesObjects[rc],"aOut")==0) || (strcmp(typesObjects[rc],"dOut")==0)){
			printf("----------------------------------- Subscripcion------------------------- \r\n");
			client->subscribe(topicsMQTT[rc], MQTT::QOS1, messageArrived);
		}
	}
}
int main(){
	/************ Read and parse file ********************/
	printf("Reading files  "__TIME__"\r\n");
	ReadFile();
	printf("Starting PicoJSON "__TIME__"\r\n");
	printf(">>> parsing \r\n");

	parse();
	/***************************************************/
	MQTTEthernet ipstack = MQTTEthernet();
	MQTT::Client<MQTTEthernet, Countdown> client = MQTT::Client<MQTTEthernet, Countdown>(ipstack);
	printf("Connecting to %s:%d\r\n", server, port);
	int rc = ipstack.connect(server, port);

	if (rc != 0){
		printf("rc from TCP connect is %d\r\n", rc);
		errorConfig(15);
	}
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
	data.MQTTVersion = 3;
	data.clientID.cstring = "mbed-sample";
	data.username.cstring = "testuser";
	data.password.cstring = "testpassword";

	if ((rc = client.connect(data)) != 0){
		printf("rc from MQTT connect is %d\n", rc);
		errorConfig(15);
	}

	//MQTT::Message message;
	// QoS 0
	message.qos = MQTT::QOS0;
	message.retained = false;
	message.dup = false;

	//Thread t(temp_thread);
	subscribeTopics(&client);
	timeWind.attach(&timer_sensor,3);

	while(true){
		int i=0;
		if(update==1){
			update=0;
			updateSensorData(client);
			leds=leds+1;
		}
		client.yield(100);
		wait(1);
	}
}
