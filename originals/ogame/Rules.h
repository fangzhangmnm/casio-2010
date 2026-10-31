 int ru_login(char* username,char* password)
{
	int id;
	PLAYER player;
	if((id=db_search_player(username))==-1) return 0;
	if(db_load_player(id,&player)) return 0;
	if(strcmp(player.password,password)!=0) return 0;
	return 1;
}