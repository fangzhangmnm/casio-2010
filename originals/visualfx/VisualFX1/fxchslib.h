/*
   header of font file
*/

typedef struct
{
	char			author[16]		;
	unsigned int	width,height	;
	unsigned int	asc_offset		;
	unsigned int	chs_offset		;
	int				file_handle		;
	int				font_size		;
}
FONTFILE;


FONTFILE *	open_font(const char * cfname);
void		close_font (FONTFILE * ff);
void		select_font (const FONTFILE * font);
void		print_chs_char (int x,int y,int sel,unsigned char c1,unsigned char c2);
void		print_asc_char (int x,int y,int sel,unsigned char c);
void		print_chs_str (int x,int y,int sel,const unsigned char * str);



#define Chs_OpenFont		open_font
#define Chs_CloseFont		close_font
#define Chs_SelectFont		select_font
#define Chs_Print			print_chs_str
#define Chs_PrintChsChar	print_chs_char
#define Chs_PrintAscChar	print_asc_char



