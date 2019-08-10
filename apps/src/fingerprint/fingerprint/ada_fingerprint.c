/**
 * Description: This file is migrated from https://raw.githubusercontent.com/SeeedDocument/Grove-Fingerprint_Sensor/master/res/Fingerprint_library.rar
 * 		Some functions have been renamed or changed their format to make them suitable for Legato code
 * 
 **/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/io.h>
#include <stdio.h>

#include <termios.h>
#include <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <sys/signal.h>

#include <legato.h>
#include <interfaces.h>

int __thePassword = 0;
uint32_t __theAddress = 0xFFFFFFFF;

int serial_fd = 0;
struct sigaction saio;
struct termios oldtio, newtio;

static bool data_available = false;
void signal_handler_IO(int status)
{
	data_available = true;
}

int serial_open(const char *serial_bus)
{
	int fd;

	fd = open(serial_bus, O_RDWR | O_NOCTTY | O_NONBLOCK);

	/* install the signal handler before making the device asynchronous */
	saio.sa_handler = signal_handler_IO;
	// saio.sa_mask = 0;
	saio.sa_flags = 0;
	saio.sa_restorer = NULL;
	sigaction(SIGIO, &saio, NULL);

	/* allow the process to receive SIGIO */
	fcntl(fd, F_SETOWN, getpid());
	fcntl(fd, F_SETFL, FASYNC);

	// fcntl (fd, F_SETFL, 0);

	// get the parameters
	tcgetattr(fd, &oldtio);
	tcgetattr(fd, &newtio);

	// Set the baud rates to 57600...
	cfsetispeed(&newtio, B57600);
	cfsetospeed(&newtio, B57600);
	// Enable the receiver and set local mode...
	newtio.c_cflag |= (CLOCAL | CREAD);
	// No parity (8N1):
	newtio.c_cflag &= ~PARENB;
	newtio.c_cflag &= ~CSTOPB;
	newtio.c_cflag &= ~CSIZE;
	newtio.c_cflag |= CS8;
	// enable hardware flow control (CNEW_RTCCTS)
	// newtio.c_cflag |= CRTSCTS;
	// if(hw_handshake)
	// Disable the hardware flow control for use with mangOH RED
	newtio.c_cflag &= ~CRTSCTS;

	// set raw input
	newtio.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	newtio.c_iflag &= ~(INLCR | ICRNL | IGNCR);

	// set raw output
	newtio.c_oflag &= ~OPOST;
	newtio.c_oflag &= ~OLCUC;
	newtio.c_oflag &= ~ONLRET;
	newtio.c_oflag &= ~ONOCR;
	newtio.c_oflag &= ~OCRNL;

	newtio.c_cc[VMIN] = 0;
	newtio.c_cc[VTIME] = 0;

	// Set the new newtio for the port...
	tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &newtio);

	return fd;
}
int serial_close(int serial_fd)
{
	// restore old serial configuration
	tcsetattr(serial_fd, TCSANOW, &oldtio);
	close(serial_fd);
	return 0;
}
int serial_read(int serial_fd, char *buffer, int buffer_size)
{
	int bytes_read = 0;
	bytes_read = read(serial_fd, buffer, buffer_size);
	return bytes_read;
}
int serial_write(int serial_fd, const char *data, int data_len)
{
	int wrote_bytes = 0;
	wrote_bytes = write(serial_fd, data, data_len);
	return wrote_bytes;
}
void serial_flush(int serial_fd)
{
	tcflush(serial_fd, TCIOFLUSH);
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Perform password verification. (handshaking when communicate via UART)
 * 	This function uses the default password (0) to do the verification
 * Parameters:
 * 	None
 * Return:
 * 	FINGERPRINT_OK: Password verification is success. (Handshake is OK)
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 * 	FINGERPRINT_PASSFAIL: Password verification failed
 */
//------------------------------------------------------------------------------
bool fingerprint_verifyPassword(void)
{
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_VERIFYPASSWORD,
						   (__thePassword >> 24), (__thePassword >> 16),
						   (__thePassword >> 8), __thePassword};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 7);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len == 1) && (packet[0] == FINGERPRINT_ACKPACKET) && (packet[1] == FINGERPRINT_OK))
		return true;

	return false;
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Get fingerprint of detected finger
 * Parameters:
 * 	None
 * Return:
 * 	FINGERPRINT_OK: Fingerprint generation is OK
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 * 	FINGERPRINT_NOFINGER: No finger was detected
 * 	FINGERPRINT_IMAGEFAIL: Image generation failed
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_getImage(void)
{
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_GETIMAGE};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 3);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
		return -1;
	return packet[1];
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Generate character file from fingerprint
 * Parameters:
 * 	slot: integer value represents CharBuffer slot (1 or 2)
 * Return:
 * 	FINGERPRINT_OK: Image converted
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 * 	FINGERPRINT_IMAGEMESS: Image too messy
 * 	FINGERPRINT_FEATUREFAIL: Could not find fingerprint features
 * 	FINGERPRINT_INVALIDIMAGE: Image cannot be generated
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_image2Tz(uint8_t slot)
{
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_IMAGE2TZ, slot};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 4);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
		return -1;
	return packet[1];
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Create model from character file CharBuffer1 and CharBuffer2
 * Parameters:
 * 	None
 * Return:
 * 	FINGERPRINT_OK: Model created
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 * 	FINGERPRINT_INVALIDREG: Fingerprints did not match
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_createModel(void)
{
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_REGMODEL};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 3);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
		return -1;
	return packet[1];
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Delete all stored model in module
 * Parameters:
 * 	None
 * Return:
 * 	FINGERPRINT_OK: Model created
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 * 	FINGERPRINT_INVALIDREG: Failed to empty the database (stored model)
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_emptyDatabase(void)
{
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_EMPTY};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 3);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
		return -1;
	return packet[1];
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Store created model to flash memory
 * Parameters:
 * 	id: integer value represents id of stored model, depending on flash size
 * Return:
 * 	FINGERPRINT_OK: Stored
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 * 	FINGERPRINT_BADLOCATION: Could not store in that location
 * 	FINGERPRINT_FLASHERR: Error writing to flash
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_storeModel(uint16_t id)
{
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_STORE, 0x01, id >> 8, id & 0xFF};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 6);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
		return -1;
	return packet[1];
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Delete stored model
 * Parameters:
 * 	id: integer value represents id of stored model, depending on flash size
 * Return:
 * 	FINGERPRINT_OK: Stored
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 * 	FINGERPRINT_DELETEFAIL: Failed to delete stored model
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_deleteModel(uint16_t id)
{
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_DELETE, id >> 8, id & 0xFF, 0x00, 0x01};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 7);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
		return -1;
	return packet[1];
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Perform fast search for character file at slot 1 in stored fingers
 * Parameters:
 * 	fingerIDPtr: output the id of found finger
 * 	confidencePtr: output the confidence value of found finger
 * Return:
 * 	FINGERPRINT_OK: Stored
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 * 	FINGERPRINT_NOTFOUND: Did not find a match
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_fingerFastSearch(uint16_t *fingerIDPtr, uint16_t *confidencePtr)
{
	*fingerIDPtr = 0xFFFF;
	*confidencePtr = 0xFFFF;
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	// high speed search of slot #1 starting at page 0x0000 and page #0x00A3
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_HISPEEDSEARCH, 0x01, 0x00, 0x00, 0x00, 0xA3};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 8);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
		return -1;

	*fingerIDPtr = packet[2];
	*fingerIDPtr <<= 8;
	*fingerIDPtr |= packet[3];

	*confidencePtr = packet[4];
	*confidencePtr <<= 8;
	*confidencePtr |= packet[5];

	return packet[1];
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Get the number of stored models
 * Parameters:
 * 	templateCountPtr: output the number of stored models
 * Return:
 * 	FINGERPRINT_OK: Read successfully
 * 	FINGERPRINT_PACKETRECIEVEERR: Communication error
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_getTemplateCount(uint16_t *templateCountPtr)
{
	*templateCountPtr = 0xFFFF;
	size_t packet_size = FINGERPRINT_PACKET_SIZE;
	// get number of templates in memory
	uint8_t packet[FINGERPRINT_PACKET_SIZE] = {FINGERPRINT_TEMPLATECOUNT};
	fingerprint_writePacket(__theAddress, FINGERPRINT_COMMANDPACKET, packet, 3);
	uint8_t len = fingerprint_getReply(packet, &packet_size, FINGERPRINT_DEFAULTTIMEOUT);

	if ((len != 1) && (packet[0] != FINGERPRINT_ACKPACKET))
		return -1;

	*templateCountPtr = packet[2];
	*templateCountPtr <<= 8;
	*templateCountPtr |= packet[3];

	return packet[1];
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Format, do checksum and write packet to module
 * Parameters:
 * 	addr: module address (4 bytes)
 * 	packettype: packet identifier
 * 	packetPtr: input packet pointer to its content
 * 	packetSize: size of passed packet content
 * Return:
 * 	None
 */
//------------------------------------------------------------------------------
void fingerprint_writePacket(uint32_t addr,
			     uint8_t packettype,
			     const uint8_t *packetPtr,
			     size_t packetSize)
{
	uint8_t i = 0;
	char *tmp_buff = malloc(9 + packetSize);

	if (tmp_buff == NULL)
	{
		LE_ERROR("Failed to allocate memory");
		return;
	}

	tmp_buff[0] = ((uint8_t)(FINGERPRINT_STARTCODE >> 8));
	tmp_buff[1] = ((uint8_t)FINGERPRINT_STARTCODE);
	tmp_buff[2] = ((uint8_t)(addr >> 24));
	tmp_buff[3] = ((uint8_t)(addr >> 16));
	tmp_buff[4] = ((uint8_t)(addr >> 8));
	tmp_buff[5] = ((uint8_t)(addr));
	tmp_buff[6] = ((uint8_t)packettype);
	tmp_buff[7] = ((uint8_t)(packetSize >> 8));
	tmp_buff[8] = ((uint8_t)(packetSize));

	uint16_t sum = (packetSize >> 8) + (packetSize & 0xFF) + packettype;
	for (i = 0; i < packetSize - 2; i++)
	{
		tmp_buff[9 + i] = ((uint8_t)(packetPtr[i]));
		sum += packetPtr[i];
	}
	tmp_buff[9 + i] = ((uint8_t)(sum >> 8));
	tmp_buff[10 + i] = ((uint8_t)sum);

	serial_write(serial_fd, tmp_buff, 9 + packetSize);
	serial_flush(serial_fd);

	free(tmp_buff);
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Get and verify the received packet
 * Parameters:
 * 	packetPtr: output buffer to store received packet
 * 	packetSize: putput the received packet length
 * 	timeout: specify waiting timeout in ms
 * Return:
 * 	Length of packet content
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_getReply(uint8_t *packetPtr,
			     size_t *packetSize,
			     uint16_t timeout)
{
	uint8_t reply[FINGERPRINT_PACKET_SIZE], idx;
	uint16_t timer = 0;
	char tmp_buff[FINGERPRINT_PACKET_SIZE];
	int read_bytes = 0;

	idx = 0;

	while (true)
	{
		while (data_available == false)
		{
			sleep(0.001);
			timer++;
			if (timer >= timeout)
				return FINGERPRINT_TIMEOUT;
		}

		read_bytes = serial_read(serial_fd, tmp_buff, FINGERPRINT_PACKET_SIZE);
		if (read_bytes <= 0)
		{
			data_available = false;
		}
		for (int i = 0; i < read_bytes; i++)
		{
			reply[idx] = tmp_buff[i];
			if ((idx == 0) && (reply[0] != (FINGERPRINT_STARTCODE >> 8)))
				continue;
			idx++;

			// check packet!
			if (idx >= 9)
			{
				if ((reply[0] != (FINGERPRINT_STARTCODE >> 8)) ||
				    (reply[1] != (FINGERPRINT_STARTCODE & 0xFF)))
					return FINGERPRINT_BADPACKET;
				uint8_t packettype = reply[6];
				//LE_INFO("Packet type"); LE_INFO(packettype);
				uint16_t len = reply[7];
				len <<= 8;
				len |= reply[8];
				len -= 2;
				//LE_INFO("Packet len"); LE_INFO(len);
				if (idx <= (len + 10))
					continue;
				packetPtr[0] = packettype;
				if (*packetSize < len + 1)
				{
					LE_ERROR("Outing buffer didn't have enough size");
					return -1;
				}
				for (uint8_t j = 0; j < len; j++)
				{
					packetPtr[1 + j] = reply[9 + j];
				}
				return len;
			}
		}
	}
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Enroll a finger to database (store to flash memory)
 * Parameters:
 * 	id: id where store the finger
 * Return:
 * 	FINGERPRINT_OK: Finger enrolled
 * 	other: error occured
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_enroll(uint8_t id)
{
	uint8_t p = -1;
	LE_INFO("Waiting for valid finger to enroll");
	while (p != FINGERPRINT_OK)
	{
		p = fingerprint_getImage();
		switch (p)
		{
		case FINGERPRINT_OK:
			LE_INFO("Image taken");
			break;
		case FINGERPRINT_NOFINGER:
			LE_INFO(".");
			break;
		case FINGERPRINT_PACKETRECIEVEERR:
			LE_INFO("Communication error");
			break;
		case FINGERPRINT_IMAGEFAIL:
			LE_INFO("Imaging error");
			break;
		default:
			LE_INFO("Unknown error");
			break;
		}
	}

	// OK success!

	p = fingerprint_image2Tz(1);
	switch (p)
	{
	case FINGERPRINT_OK:
		LE_INFO("Image converted");
		break;
	case FINGERPRINT_IMAGEMESS:
		LE_INFO("Image too messy");
		return p;
	case FINGERPRINT_PACKETRECIEVEERR:
		LE_INFO("Communication error");
		return p;
	case FINGERPRINT_FEATUREFAIL:
		LE_INFO("Could not find fingerprint features");
		return p;
	case FINGERPRINT_INVALIDIMAGE:
		LE_INFO("Could not find fingerprint features");
		return p;
	default:
		LE_INFO("Unknown error");
		return p;
	}

	LE_INFO("Remove finger");
	sleep(2);
	p = 0;
	while (p != FINGERPRINT_NOFINGER)
	{
		p = fingerprint_getImage();
	}

	p = -1;
	LE_INFO("Place same finger again");
	while (p != FINGERPRINT_OK)
	{
		p = fingerprint_getImage();
		switch (p)
		{
		case FINGERPRINT_OK:
			LE_INFO("Image taken");
			break;
		case FINGERPRINT_NOFINGER:
			LE_INFO(".");
			break;
		case FINGERPRINT_PACKETRECIEVEERR:
			LE_INFO("Communication error");
			break;
		case FINGERPRINT_IMAGEFAIL:
			LE_INFO("Imaging error");
			break;
		default:
			LE_INFO("Unknown error");
			break;
		}
	}

	// OK success!

	p = fingerprint_image2Tz(2);
	switch (p)
	{
	case FINGERPRINT_OK:
		LE_INFO("Image converted");
		break;
	case FINGERPRINT_IMAGEMESS:
		LE_INFO("Image too messy");
		return p;
	case FINGERPRINT_PACKETRECIEVEERR:
		LE_INFO("Communication error");
		return p;
	case FINGERPRINT_FEATUREFAIL:
		LE_INFO("Could not find fingerprint features");
		return p;
	case FINGERPRINT_INVALIDIMAGE:
		LE_INFO("Could not find fingerprint features");
		return p;
	default:
		LE_INFO("Unknown error");
		return p;
	}

	// OK converted!
	p = fingerprint_createModel();
	if (p == FINGERPRINT_OK)
	{
		LE_INFO("Prints matched!");
	}
	else if (p == FINGERPRINT_PACKETRECIEVEERR)
	{
		LE_INFO("Communication error");
		return p;
	}
	else if (p == FINGERPRINT_ENROLLMISMATCH)
	{
		LE_INFO("Fingerprints did not match");
		return p;
	}
	else
	{
		LE_INFO("Unknown error");
		return p;
	}

	p = fingerprint_storeModel(id);
	if (p == FINGERPRINT_OK)
	{
		LE_INFO("Stored!");
	}
	else if (p == FINGERPRINT_PACKETRECIEVEERR)
	{
		LE_INFO("Communication error");
		return p;
	}
	else if (p == FINGERPRINT_BADLOCATION)
	{
		LE_INFO("Could not store in that location");
		return p;
	}
	else if (p == FINGERPRINT_FLASHERR)
	{
		LE_INFO("Error writing to flash");
		return p;
	}
	else
	{
		LE_INFO("Unknown error");
		return p;
	}
	return FINGERPRINT_OK;
}

//------------------------------------------------------------------------------
/**
 * Descrition:
 * 	Perform procedure to check a finger with database
 * Parameters:
 * 	None
 * Return:
 * 	FINGERPRINT_OK: Finger found
 * 	other: error occured
 */
//------------------------------------------------------------------------------
uint8_t fingerprint_check()
{
	uint16_t fingerprint_fingerID;
	uint16_t fingerprint_confidence;

	uint8_t p = fingerprint_getImage();
	switch (p)
	{
	case FINGERPRINT_OK:
		LE_INFO("Image taken");
		break;
	case FINGERPRINT_NOFINGER:
		LE_INFO("No finger detected");
		return p;
	case FINGERPRINT_PACKETRECIEVEERR:
		LE_INFO("Communication error");
		return p;
	case FINGERPRINT_IMAGEFAIL:
		LE_INFO("Imaging error");
		return p;
	default:
		LE_INFO("Unknown error");
		return p;
	}

	// OK success!

	p = fingerprint_image2Tz(1);
	switch (p)
	{
	case FINGERPRINT_OK:
		LE_INFO("Image converted");
		break;
	case FINGERPRINT_IMAGEMESS:
		LE_INFO("Image too messy");
		return p;
	case FINGERPRINT_PACKETRECIEVEERR:
		LE_INFO("Communication error");
		return p;
	case FINGERPRINT_FEATUREFAIL:
		LE_INFO("Could not find fingerprint features");
		return p;
	case FINGERPRINT_INVALIDIMAGE:
		LE_INFO("Could not find fingerprint features");
		return p;
	default:
		LE_INFO("Unknown error");
		return p;
	}

	// OK converted!
	p = fingerprint_fingerFastSearch(&fingerprint_fingerID, &fingerprint_confidence);
	if (p == FINGERPRINT_OK)
	{
		LE_INFO("Found a print match!");
	}
	else if (p == FINGERPRINT_PACKETRECIEVEERR)
	{
		LE_INFO("Communication error");
		return p;
	}
	else if (p == FINGERPRINT_NOTFOUND)
	{
		LE_INFO("Did not find a match");
		return p;
	}
	else
	{
		LE_INFO("Unknown error");
		return p;
	}

	// found a match!
	LE_INFO("Found ID #%d", fingerprint_fingerID);
	LE_INFO(" with confidence of %d", fingerprint_confidence);

	return FINGERPRINT_OK;
}

static int get_finger_image(void)
{
	uint8_t p = -1;
	// puts("Put your finger to sensor");
	while (p != FINGERPRINT_OK)
	{
		p = fingerprint_getImage();
		switch (p)
		{
		case FINGERPRINT_OK:
			puts("Image taken");
			break;
		case FINGERPRINT_NOFINGER:
			puts(".");
			break;
		case FINGERPRINT_PACKETRECIEVEERR:
			puts("Communication error");
			return -1;
			break;
		case FINGERPRINT_IMAGEFAIL:
			puts("Imaging error");
			break;
		default:
			puts("Unknown error");
			return -1;
			break;
		}
	}
	return 0;
}
COMPONENT_INIT
{
	serial_fd = serial_open("/dev/ttyHS0");

	if (fingerprint_verifyPassword())
	{
		puts("Found fingerprint sensor!");
	}
	else
	{
		fprintf(stderr, "Did not find fingerprint sensor :(\n");
		exit(EXIT_FAILURE);
	}
	if (get_finger_image() != 0) {
		// fprintf(stderr, "Did not find fingerprint sensor :(\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}