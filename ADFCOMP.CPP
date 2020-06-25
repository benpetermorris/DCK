#include "dck.h"
#pragma hdrstop

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Globals begin here                                                      *
 *                                                                         *
 *-------------------------------------------------------------------------*/

short so_docommand( void );
short motif_docommand( void );

SO_LINETABLE    so_table[] =
{
    { "Floor",      SOP_ADJTYPE,    SOP_INTEGER,    SC_FLOOR },
    { "FloorTex",   SOP_STRING,     SOP_NONE,       SC_FLOORTEX },
    { "Light",      SOP_ADJTYPE,    SOP_INTEGER,    SC_LIGHT },
    { "BLineType",  SOP_INTEGER,    SOP_NONE,       SC_BLTYPE },
    { "ObjInsert",  SOP_INTEGER,    SOP_NONE,       SC_OBJINSERT },
    { "Ceil",       SOP_ADJTYPE,    SOP_INTEGER,    SC_CEIL },
    { "CeilTex",    SOP_STRING,     SOP_NONE,       SC_CEILTEX },
    { "SecType",    SOP_INTEGER,    SOP_NONE,       SC_SECTYPE },
    { "BLineLTex",  SOP_STRING,     SOP_NONE,       SC_BLLTEX },
    { "BLineUTex",  SOP_STRING,     SOP_NONE,       SC_BLUTEX },
    { "BLineFTex",  SOP_STRING,     SOP_NONE,       SC_BLFTEX },
    { "SecFindTag", SOP_NONE,       SOP_NONE,       SC_SECTAG },
    { "BLineSetTag",SOP_NONE,       SOP_NONE,       SC_BLSETTAG },
    { NULL,         0,              0,              0 }
};

struct
{
    char    *name;
    char    *desc;
    short   code;
    short   (*fp)(void);

} sectypes[] =
{
    { "SectObject",     "sector object",    SEC_SOBJECT,        so_docommand },
    { "Motif",          "motif",            SEC_MOTIF,          motif_docommand },
    { NULL,             NULL,               -1,                 NULL }
};

uchar *st_adj[] =
{
    "Ceiling",
    "Floor",
    "Real",
    "Adjac",
    NULL
};

static  FILE    *in_fp;
static  short   outhandle;
static  short   nerror, lineno;
static  uchar   *tokptr;
static  uchar   tokbuf[128];

/*-------------------------------------------------------------------------*
 *                                                                         *
 * Code begins here                                                        *
 *                                                                         *
 *-------------------------------------------------------------------------*/

static void error( char *fmt, ... )
/*-------------------------------------------------------------------------*
 * Displays an error and sets the error variable.                          *
 *-------------------------------------------------------------------------*/
{
    char    buf[128];
    va_list vl;

    va_start( vl, fmt );
    vsprintf( buf, fmt, vl );
    va_end( vl );

    if( !nerror )
        printf( "\n" );

    nerror ++;      /* more errors!  eek */

    clreol();
    printf( "  Error (line %d): %s\n", lineno, buf );
}

static short getinputline( char *buf )
{
    char    *bptr = buf;

    while( 1 )
    {
        bptr = buf;

        if( fgets( buf, 128, in_fp ) == NULL )
            return 0;

        ++lineno;

        /* Strip preceding whitespace */
        while( *bptr == 32 || *bptr == 9 )
            ++bptr;

        /* Strip \r\n too.. how convenient! */
        strccpy( buf, bptr, ";\r\n" );

        /* And FINALLY, strip trailing whitespace */
        bptr = buf + strlen( buf ) - 1;
        while( *bptr == 32 || *bptr == 9 )
            bptr--;
        *(bptr+1) = 0;

        if( *buf )
            break;
    }

    return 1;
}

#define issepch( ch )   (strchr( tokstr, ch ) != NULL)

static short gettoken( short *type, char *data )
/*-------------------------------------------------------------------------*
 * Gets a token from the file and returns its type and data.               *
 * Data is the proper type of the parameter, ie: integer or string.        *
 *-------------------------------------------------------------------------*/
{
    uchar   tmpbuf[128], *tbptr = tmpbuf;
    short     i;
    uchar   tokstr[] = ";:=()[]!*/|&%,<>^ \x09\r\n";

    /* Skip whitespace to go to the next token */
    if( tokptr != NULL )
        while( issepch( *tokptr ) && *tokptr )
            ++tokptr;

    /* Get a token from the token pointer */
    if( tokptr == NULL || !(*tokptr) || *tokptr == '\r' || *tokptr == '\n' )
    {
        if( !getinputline( tokbuf ) )
            return 0;       /* no more tokens */
        tokptr = tokbuf;
    }

    /* Skip whitespace to go to the next token */
    if( tokptr != NULL )
        while( issepch( *tokptr ) && *tokptr )
            ++tokptr;

    /* tokptr is the start of the next token. */
    if( *tokptr == '+' || *tokptr == '-' || isdigit( *tokptr ) )
    {
        /* Number. */
        while( *tokptr == '+' || *tokptr == '-' || isdigit( *tokptr ) )
            *tbptr++ = *tokptr++;
        *tbptr = 0;

        *((short *) data) = atoi( tmpbuf );
        *type = SOP_INTEGER;
        return 1;
    }

    if( *(tokptr-1) == '[' )
        *type = SOP_BSTRING;
    else
        *type = SOP_STRING;

    /* Name?  Copy the contents.. */
    i = 0;
    while( !issepch( *tokptr ) )
    {
        *tbptr++ = *tokptr++;
        ++i;
        if( i == TOKLEN-1 )
        {
            error( "name too long (max %d characters)", TOKLEN-1 );
            break;
        }
    }
    *tbptr = 0;

    strcpy( data, tmpbuf );
    return 1;
}

static void fillparmstruc( SO_PARM *p, short expected )
/*-------------------------------------------------------------------------*
 * p = parameter structure to fill                                         *
 * expected = the expected type here.                                      *
 *-------------------------------------------------------------------------*/
{
    short     type, i;
    char    buf[TOKLEN], *bptr = buf;

    memset( p, 0, sizeof( SO_PARM ) );

    p->type = expected;

    if( expected == SOP_NONE )
        return;

    if( !gettoken( &type, buf ) )
        return;

    switch( expected )
    {
    case SOP_INTEGER:
        if( type != SOP_INTEGER )
        {
            error( "number parameter expected" );
            return;
        }
        p->idata = *((short *) bptr);
        break;
    case SOP_STRING:
        if( type != SOP_STRING )
        {
            error( "string parameter expected" );
            return;
        }
        strcpy( p->sdata, bptr );
        break;
    case SOP_ADJTYPE:
        if( type != SOP_STRING )
        {
atexp:
            error( "adjustment type string expected" );
            return;
        }
        for( i = 0; st_adj[i]; i++ )
        {
            if( !strcmpi( buf, st_adj[i] ) )
                break;
        }

        /* this means not found */
        if( !st_adj[i] )
            goto atexp;

        p->idata = i;
        break;
    }
}

static void so_writecommand( SO_COMMAND *cmd )
{
    short  i;

    /* Write out the command number */
    z_write( outhandle, &(cmd->cmd), sizeof( cmd->cmd ) );

    /* And the parameters */
    for( i = 0; i < SO_MAXPARM; i++ )
    {
        z_write( outhandle, &(cmd->p[i].type), 1 );
        switch( cmd->p[i].type )
        {
        case SOP_NONE:
            i = SO_MAXPARM; /* outta here */
            break;
        case SOP_INTEGER:
        case SOP_ADJTYPE:
            z_write( outhandle, &(cmd->p[i].idata), 2 );
            break;
        case SOP_STRING:
            strupr( cmd->p[i].sdata );
            z_write( outhandle, cmd->p[i].sdata, TOKLEN );
            break;
        }
    }
}

static short so_docommand( void )
/*-------------------------------------------------------------------------*
 * Reads the next command from the file and parses it and writes it to     *
 * an output file.  woo woo                                                *
 *-------------------------------------------------------------------------*/
{
    char        name[32];
    short         i, type, cmd;
    SO_COMMAND  soc_struc;

    /* Get the command name */
    if( !gettoken( &type, name ) )
    {
        error( "end-of-file in sector object" );
        return -1;
    }

    /* e-o-section */
    if( type == SOP_BSTRING )
        return -1;

    /* Search for the command */
    for( i = 0; ; i++ )
    {
        if( so_table[i].name == NULL )
        {
            /* error! */
            error( "command '%s' not recognised", name );
            return 0;
        }
        if( !strcmp( name, so_table[i].name ) )
            break;
    }

    /* Found the command.  Now get the parameters and write it to
     * the output file. */
    cmd = i;

    soc_struc.cmd = so_table[cmd].cmdno;

    /* Fill the parameters */
    fillparmstruc( &soc_struc.p[0], so_table[cmd].p1type );
    fillparmstruc( &soc_struc.p[1], so_table[cmd].p2type );

    /* Write the thing out now */
    so_writecommand( &soc_struc );

    return 0;
}

static short motif_docommand( void )
/*-------------------------------------------------------------------------*
 * Parse a motif command.  The first token is expected to be a type        *
 * of name .. the second, a string or integer.                             *
 *-------------------------------------------------------------------------*/
{
    short     type;
    char    name[TOKLEN];

    /* Get the command name */
    memset( name, 0, TOKLEN );
    if( !gettoken( &type, name ) )
    {
        error( "end-of-file in motif" );
        return -1;
    }

    /* e-o-section */
    if( type == SOP_BSTRING )
        return -1;

    if( type != SOP_STRING )
    {
        error( "motif command invalid (not a name!)" );
        return -1;
    }

    strupr( name );
    z_write( outhandle, name, TOKLEN );

    /* Get the parameter */
    memset( name, 0, TOKLEN );
    if( !gettoken( &type, name ) )
    {
        error( "end-of-file in motif" );
        return -1;
    }

    /* Write out the type, and the data. */
    z_write( outhandle, &type, sizeof type );

    switch( type )
    {
    case SOP_STRING:
        strupr( name );
        z_write( outhandle, name, TOKLEN );
        break;
    case SOP_INTEGER:
        z_write( outhandle, name, sizeof( short ) );
        break;
    }

    return 0;
}

static short do_section( char *secbuf )
/*-------------------------------------------------------------------------*
 * "Does" a section.  secbuf contains [sectype secname] or something.      *
 *-------------------------------------------------------------------------*/
{
    char    sectype[TOKLEN], secname[TOKLEN];
    char    tmpbuf[128];
    short     i, type;
    long    ncmdofs;
    ADF_SECTION sec;

    ++secbuf;       /* go past '[' */

    tokptr = secbuf;

    memset( secname, 0, TOKLEN );
    memset( sectype, 0, TOKLEN );

    /* Get the type of the section */
    gettoken( &type, sectype );
    gettoken( &type, secname );

    for( i = 0; ; i++ )
    {
        if( !strcmpi( sectypes[i].name, sectype ) )
        {
            strchangechar( secname, '_', ' ' );

            /* Yah! */
            sec.type = sectypes[i].code;
            sec.ncmd = 0;
            strcpy( sec.name, secname );

            ncmdofs = tell( outhandle );
            z_write( outhandle, &sec, sizeof sec );

            sprintf( tmpbuf, "  Parsing %s '%s'", sectypes[i].desc, secname );
            printf( "%-79s\r", tmpbuf );

            while( 1 )
            {
                if( (*(sectypes[i].fp))() < 0 )
                    break;
                ++sec.ncmd;
            }

            lseek( outhandle, ncmdofs, SEEK_SET );
            z_write( outhandle, &sec, sizeof sec );
            lseek( outhandle, 0L, SEEK_END );
            return 0;
        }

        if( sectypes[i].name == NULL )
        {
            error( "unrecognised section type '%s'", sectype );
            return 0;
        }
    }

    return 0;
}

short ad_compile( char *inf )
/*-------------------------------------------------------------------------*
 * Returns !0 on success.
 *-------------------------------------------------------------------------*/
{
    char    buf[128];
    short   nsection = 0;       /* number of sections.  first two bytes. */

    /* reset globals */
    nerror = lineno = 0;
    tokptr = NULL;
    *tokbuf = 0;

    _setcursortype( _NOCURSOR );

    printf( "þ Compiling ADF file: %s\n", inf );

    in_fp = fopen( inf, "rt" );

    if( in_fp == NULL )
    {
        printf( "  Error: I couldn't open the file '%s'.\n"
                "  Each game data (GD) file must have an ADF file to go with it.\n", inf );
        return 0;
    }

    fnchop( buf, inf, FNC_FILE | FNC_DRIVE | FNC_DIRECTORY );
    strcat( buf, ".AD" );

    outhandle = z_creat( buf, O_BINARY | S_IWRITE );

    if( outhandle == -1 )
    {
        printf( "  Error: Can't open %s for write access.\n", buf );
        return 0;
    }

    z_write( outhandle, "ADF", 3 ); /* signature */
    z_write( outhandle, &nsection, sizeof nsection );

    /* Compile all the sections */
    while( 1 )
    {
        if( fgets( buf, 128, in_fp ) == NULL )
            break;

        ++lineno;

        if( *buf == '[' )   /* Found a section? */
        {
            if( do_section( buf ) != -1 )
                ++nsection;
            else
            {
                while( 1 )
                {
                    /* Ignore 'til [end] */
                    if( fgets( buf, 128, in_fp ) == NULL )
                        break;

                    ++lineno;

                    if( *buf == '[' )   /* End */
                        break;
                }
            }
        }
    }

    /* Write out the number of sections at the beginning of the file */
    lseek( outhandle, 3L, SEEK_SET );
    z_write( outhandle, &nsection, sizeof nsection );
    z_close( outhandle );
    fclose( in_fp );

    if( !nerror )
        clreol();   /* get rid of the "parsing.." line */

    _setcursortype( _NORMALCURSOR );

    return !nerror;
}
