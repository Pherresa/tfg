%{
#include <glib.h>
#include <stdlib.h>
#include <string.h>

#include <process.h>
#include <CPU/simulation.h>

#include "parser.h"
void yyerror (char *s);
int yylex (void);

static simul_data_t *data = NULL;
static gint n_io_events;
static gint n_pages;
static GScanner *scanner;
static gboolean error;
#define YYSTYPE gint
%}
  
%token PROC	
%token START	
%token END	
%token IO	
%token BLOCK	
%token TIME	
%token NUM	
%token MEM
%token PAGE
%token WRITE

/* Grammar follows */
%%
input:	  PROC '{' start end ios mems '}' {return 0;}
;
start:
	| START '=' NUM		{data->start_time=$3;}
;
end:
	| END '=' NUM		{data->end_time=$3;}
;
ios:	  IO '{' io_lines '}' {}
;
io_lines:
	| io_lines io_line
;
io_line:  BLOCK '=' NUM TIME '=' NUM {
				data->io_events = g_renew(simul_io_event_t,
							data->io_events,
							++n_io_events);
				data->io_events[n_io_events-1].time=$6;
				data->io_events[n_io_events-1].block=$3;
				}
;
mems:	  MEM '{' mem_lines '}' {}
;
mem_lines:
	| mem_lines mem_line
;
mem_line:  PAGE '=' NUM WRITE '=' NUM {
				data->pages = g_renew(simul_mem_t,
							data->pages,
							++n_pages);
				data->pages[n_pages-1].page=$3;
				data->pages[n_pages-1].write=$6;
				}
;
%%

#include <ctype.h>

void parse_proc_init(void)
{
	scanner = g_scanner_new (NULL);
	g_scanner_freeze_symbol_table (scanner);
	g_scanner_add_symbol (scanner, "proc", GINT_TO_POINTER (PROC));
	g_scanner_add_symbol (scanner, "start_time", GINT_TO_POINTER (START));
	g_scanner_add_symbol (scanner, "end_time", GINT_TO_POINTER (END));
	g_scanner_add_symbol (scanner, "io", GINT_TO_POINTER (IO));
	g_scanner_add_symbol (scanner, "block", GINT_TO_POINTER (BLOCK));
	g_scanner_add_symbol (scanner, "time", GINT_TO_POINTER (TIME));
	g_scanner_add_symbol (scanner, "mem", GINT_TO_POINTER (MEM));
	g_scanner_add_symbol (scanner, "page", GINT_TO_POINTER (PAGE));
	g_scanner_add_symbol (scanner, "write", GINT_TO_POINTER (WRITE));
	g_scanner_thaw_symbol_table (scanner);
}

gchar *get_simulation_from_string (simul_data_t *data_p, gchar * str)
{
	data = data_p;

	g_free(data->io_events);
	data->io_events = NULL;

	g_free(data->pages);
	data->pages = NULL;

	n_io_events=0;
	n_pages=0;
	error=FALSE;

	g_scanner_input_text (scanner, str, strlen (str));
	yyparse ();

	if (error){
		g_free(data->io_events);
		data->io_events = NULL;
		g_free(data->pages);
		data->pages = NULL;
		return NULL;
	}

	data->next_io_event = data->io_events;
	data->last_io_event = data->io_events + n_io_events -1;
	data->n_pages = n_pages;
	data->cur_access = 0;

	/* FIXME: scanner->text should be consicerded a private value
	 * and not be used directly */
	return (gchar *)scanner->text;
}
int yylex (void)
{
	GTokenType type = g_scanner_get_next_token(scanner);
	switch (type){
	case G_TOKEN_INT:
		yylval = scanner->value.v_int;
		return NUM;
	case G_TOKEN_SYMBOL:
		return GPOINTER_TO_INT(scanner->value.v_symbol);
	default:
		return type;
	}
}
#include <stdio.h>

void yyerror (char *s)  /* Called by yyparse on error */
{
  printf ("%s in line %d pos %d\n", s, scanner->line, scanner->position);
  error = TRUE;
}
