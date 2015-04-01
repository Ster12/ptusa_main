#include "w_mem.h"
#include "stdlib.h"

#pragma warning( disable : 4244 ) //conversion from '__time64_t' to 'u_long', possible loss of data
FILE* SRAM::file = 0;
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
SRAM::SRAM( const char *file_name,
    u_int total_size,
    u_int available_start_pos,
    u_int available_end_pos ): NV_memory( total_size,
    available_start_pos,
    available_end_pos )
    {
    if ( strlen( file_name ) > C_MAX_FILE_NAME - 1 )
        {
        printf( "SRAM:: error - max file name length (%s).\n",
            file_name );
        }
    else
        {
        strcpy( this->file_name, file_name );

        if ( 0 == file )
            {
            if( ( file = fopen( file_name, "r+b" ) ) <= 0 )
                {
#ifdef DEBUG
                printf( "SRAM() - ERROR: Can't open device (%s) : %s.\n",
                    file_name, strerror( errno ) );
#endif // DEBUG
                file = 0;
                }
            }
        }
    }
//-----------------------------------------------------------------------------
SRAM::~SRAM()
    {
    if ( file )
        {
        fclose( file );
        file = 0;
        }
    }
//-----------------------------------------------------------------------------
int SRAM::read( void *buff, u_int count, u_int start_pos )
    {
    int res = 0;

    if ( file )
        {
        fseek( file, get_available_start_pos() + start_pos, SEEK_SET );
        res = fread( buff, sizeof( char ), count, file );

#ifdef DEBUG
        if ( res <= 0 )
            {
            printf( "Error reading device (%s) : %s.\n",
                file_name, strerror( errno )  );
            }
#endif // DEBUG

        }

    return res;
    }
//-----------------------------------------------------------------------------
int SRAM::write( void *buff, u_int count, u_int start_pos )
    {
    int res = 0;

    if ( file )
        {
        fseek( file, get_available_start_pos() + start_pos, SEEK_SET );
        res = fwrite( buff, sizeof( char ), count, file );

#ifdef DEBUG
        if ( res <= 0 )
            {
            printf( "Error writing device (%s) : %s.\n",
                file_name, strerror( errno )  );
            }
#endif // DEBUG

        fflush( file );
        }

    return res;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
data_file::data_file() : f( 0 )
    {
    }
//-----------------------------------------------------------------------------
int data_file::file_open( const char* file_name )
    {
    f = fopen( file_name, "r" );
    if ( 0 == f )
        {
        Print( "Error open file \"%s\".\n", file_name );
        return 0;
        }

    return 1;
    }
//-----------------------------------------------------------------------------
int data_file::file_read( void *buffer, int count )
    {
    int res = 0;
    if ( f > 0 )
        {
        res = fread( buffer, sizeof( char ), count, f );
        }

    return res;
    }
//-----------------------------------------------------------------------------
char* data_file::fget_line()
    {
    buf[ 0 ] = 0;

    char *tmp_buff = buf;
    int pos = 1;
    int res = fread( tmp_buff, sizeof( char ), 1, f );
    if ( res != 1 )
        {
#ifdef DEBUG
        Print( "Error reading file - can\'t read!\n" );
#endif // DEBUG
        return buf;
        }

    while ( tmp_buff[ 0 ] != '\n' )
        {
        tmp_buff++;
        res = fread( tmp_buff, sizeof( char ), 1, f );

        if ( res != 1 )
            {
#ifdef DEBUG
            Print( "Error reading file - can\'t read more!\n" );
#endif // DEBUG
            break;
            }

        pos++;
        if ( pos >= C_MAX_BUFFER_SIZE )
            {
#ifdef DEBUG
            Print( "Error reading file - line is too long!\n" );
#endif // DEBUG
            break;
            }
        }

    buf[ pos ] = 0;
    return buf;
    }
//-----------------------------------------------------------------------------
char* data_file::pfget_line()
    {
    fget_line();
    int str_len = strlen( buf );
    fseek( f, -str_len - 1, SEEK_CUR );
    return buf;
    }
//-----------------------------------------------------------------------------
void data_file::file_close()
    {
    if ( f > 0 )
        {
        fclose( f );
        f = 0;
        }
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
