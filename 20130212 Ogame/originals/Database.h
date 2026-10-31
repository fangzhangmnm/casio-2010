//define
#define MAX_STRING_LENGTH 17

#define MAX_BUILDING_TYPE 19
#define MAX_RESEARCH_TYPE 19
#define MAX_FLEET_TYPE 19
#define MAX_DEFENCE_TYPE 13

#define MAX_GALAXY 5
#define MAX_SOLAR 10
#define MAX_ORBIT 15

#define MAX_PLANET_OWN 9
#define MAX_FLEET_OWN 20

#define MAX_PLAYER 50
#define MAX_PLANET 500
#define MAX_FLEET 1000

//player

typedef struct{

	short int id;
	short int avilable;

	//account

	short int picture;
	char username[MAX_STRING_LENGTH+1];
	char nickname[MAX_STRING_LENGTH+1];
	char password[MAX_STRING_LENGTH+1];

	//setting

	//management

	char operator;
	char banned;
	char vacation;

	//vip

	long int darkmatter;

	//message

	//game

	short int planetid[MAX_PLANET_OWN];
	short int research[MAX_RESEARCH_TYPE];
	short int fleetid[MAX_FLEET_OWN];

} PLAYER;
typedef struct{
	short int id;
	char username[MAX_STRING_LENGTH+1];

	short int position;
} PLAYERKEY;
PLAYERKEY playerkey[MAX_PLAYER];

//planet

typedef struct{

	short int id;
	short int avilable;

	//basic property

	short int owner;
	short int picture;
	char name[MAX_STRING_LENGTH+1];
	short int coord[3];
	short int space;
	short int radius;

	//resource

	long int metal;
	long int crystal;
	long int heavyhydrogen;

	//building

	short int building[MAX_BUILDING_TYPE];

	//fleet

	long int fleet[MAX_FLEET_TYPE];

	//defence

	long int defence[MAX_DEFENCE_TYPE];

} PLANET;

//fleet

typedef struct{

	short int id;
	short int avilable;


} FLEET;

//global

typedef struct{

	//system

	int version;

	//memory

	int playernum;
	int planetnum;
	int fleetnum;
	int nextplayerid;
	int nextplanetid;
	int nextfleetid;

	//game

	time refreshtime;
} GLOBAL;
GLOBAL global;

//search



//file

int db_reset_game()
{
	int fp;
	int error;
	PLAYER tmpplayer;
	PLANET tmpplanet;

	global.version=VERSION;
	global.nextplayerid=1;
	global.nextplanetid=1;
	global.nextfleetid=0;
	global.playernum=1;
	global.planetnum=1;
	global.fleetnum=0;
	readtime(&global.refreshtime);
	tmpplayer.id=0;
	tmpplayer.avilable=1;
	strcpy(tmpplayer.username,"ADMIN");
	strcpy(tmpplayer.password,"ADMIN");


	Bfile_DeleteMainMemory("GLOBAL"); if(Bfile_CreateMainMemory("GLOBAL")) return 1;
	Bfile_DeleteMainMemory("PLAYER"); if(Bfile_CreateMainMemory("PLAYER")) return 1;
	Bfile_DeleteMainMemory("PLANET"); if(Bfile_CreateMainMemory("PLANET")) return 1;
	Bfile_DeleteMainMemory("FLEET"); if(Bfile_CreateMainMemory("FLEET")) return 1;

	if((fp=Bfile_OpenMainMemory("GLOBAL"))<0) return 1;
	if(Bfile_WriteFile(fp,&global,sizeof(global))<0) error=1;
	Bfile_CloseFile(fp);
	if(error) return 1;
	if((fp=Bfile_OpenMainMemory("PLAYER"))<0) return 1;
	if(Bfile_WriteFile(fp,&tmpplayer,sizeof(tmpplayer))<0) error=1;
	Bfile_CloseFile(fp);
	if(error) return 1;
	if((fp=Bfile_OpenMainMemory("PLANET"))<0) return 1;
	if(Bfile_WriteFile(fp,&tmpplanet,sizeof(tmpplanet))<0) error=1;
	Bfile_CloseFile(fp);
	if(error) return 1;
	
	return error;
}
int db_init_player()
{
	int fp;
	PLAYER player;
	int i,p,n;

	if((fp=Bfile_OpenMainMemory("PLAYER"))<0) return 1;
	i=0;p=0;n=0;
	while(Bfile_ReadFile(fp,&player,sizeof(PLAYER),-1)>=0 && i<MAX_PLAYER){
		if(player.avilable){
			playerkey[i].id=player.id;
			strcpy(playerkey[i].username,player.username);
			playerkey[i].position=p;
			n++;
		}
		p++;
	}
	Bfile_CloseFile(fp);
	if(global.playernum!=n) return 1;//alert
	return 0;
}
int db_search_player(char* username)
{
	int i;
	for(i=0;i<global.playernum;i++)
		if(strcmp(username,playerkey[i].username)) return playerkey[i].id;
	return -1;
}
int db_load_player(int id,PLAYER* buff)
{
	int i,p;
	int fp;
	p=-1;
	for(i=0;i<global.playernum;i++)
		if(playerkey[i].id==id) p=i;
	if(p==-1) return 1;
	if((fp=Bfile_OpenMainMemory("PLAYER"))<0) return 1;
	Bfile_SeekFile(fp,p*sizeof(PLAYER));
	if(Bfile_ReadFile(fp,buff,sizeof(PLAYER),-1)<0) return 1;
	return 0;
}
int db_init_game()
{
	int fp;
	int error;

	//load global
	if((fp=Bfile_OpenMainMemory("GLOBAL"))<0) return 1;
	if(Bfile_ReadFile(fp,&global,sizeof(GLOBAL),-1)<0) error=1;
	Bfile_CloseFile(fp);
	if(error) return 1;

	if(global.version!=VERSION) return 1;

	if(db_init_player()) return 1;

	return error;
}
int db_backup_game()
{

}
int db_restore_game()
{

}






