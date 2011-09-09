
int check_user_valid(FILE *fp, char *username, char *password)
{
	char	line[1024], *lasts, *tok;
	int	match;

	rewind(fp);

	match = 0;

	while(fgets(line, sizeof(line), fp)){
		tok = strtok_r(line, ":", &lasts);
		if(strcasecmp(tok, username)){
			continue;
		}

		tok = strtok_r(NULL, ":", &lasts);
		if(!strcasecmp(tok, (char *)crypt(password, password))){
			match = 1;
		}
	}

	if(match)
		return 1;
	else
		return 0;
}

int interact_w(char *ivhome)
{

	int	i;
	FILE	*fp;
	char	username[50], password[50];
	char	passfile[100];

	sprintf(passfile, "%s/%s", ivhome, PASSWORD_FILE);
	fp = fopen(passfile, "r");
	if(fp == NULL){
		fprintf(stderr, "Cannot open the password file\n");
		exit(1);
	}
	
	for(i = 0; i < RETRY_COUNT; i++){
		printf("user id :");
		fflush(stdout);
		scanf("%s", username);
		if(username[strlen(username)-1] == '\n')
			username[strlen(username)-1] = 0x00;
		printf("password :");
		fflush(stdout);
		printf("\n\n\n");	
		system("stty -echo");
		scanf("%s", password);
		system("stty echo");
		//printf("\n");
		//fflush(stdout);
		fprintf(stderr, "%sTT",&password);	
		if(password[strlen(password)-1] == '\n')
			password[strlen(password)-1] = 0x00;
	
		if(check_user_valid(fp, username, password))
			break;


	}

	if(i >= RETRY_COUNT)
		exit(1);

	printf("user confirm\n");
	
}
