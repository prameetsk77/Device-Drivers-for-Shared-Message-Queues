struct message 
{
	int mess_id;
	int sender_id;
	int receiver_id;
	char *data;
	int char_length;
	
	unsigned long long int tsc;	
	
};
