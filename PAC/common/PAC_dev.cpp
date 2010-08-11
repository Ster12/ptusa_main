#include "PAC_dev.h"
#include "tech_def.h"

auto_smart_ptr < device_manager > device_manager::instance;

const char device::DEV_NAMES[][ 5 ] = { "V", "N", "M", "LS", "TE", "FE", "FS",
    "CTR", "AO", "LE", "FB", "UPR", "QE", "AI" };

const char device::DEV_TYPES[] =        { 1,    1,   1,   1,    4,    4,    1,
     2,      4,    4,    1,    1,     4,    4 };

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
char single_state::get_val( int idx )
    {
    return ( ( unsigned long )( state [ idx / 32 ] >> idx % 32 ) & 1 );
    }  
//-----------------------------------------------------------------------------
int single_state::parse_cmd( char *buff  )   
    { 
#ifdef USE_NO_TANK_COMB_DEVICE
    return 0;
#else // USE_NO_TANK_COMB_DEVICE

    switch ( owner_type )
        {
        case T_TECH_OBJECT:
            ( ( tech_object* ) owner_object )->set_mode( ( ( u_int_4* ) buff )[ 1 ] - 1,
                ( ( u_int_4* ) buff )[ 2 ] );                      
            break;
        }
    return 12;
#endif // USE_NO_TANK_COMB_DEVICE     
    } 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
complex_state::complex_state( const char *name, int n, u_int_4 *state,
                             void *owner_object, char owner_type, int size ):
device_state < u_int_4 >( n, 
                         name, 
                         size, 
                         i_complex_device::ARRAY_DEV_LONG, 
                         state,
                         owner_object, 
                         owner_type )

    {                     
    } 
//-----------------------------------------------------------------------------
u_int_4 complex_state::get_val( int idx )
    {    
    return state[ idx ];
    }     
//-----------------------------------------------------------------------------
int complex_state::parse_cmd( char *buff  )   
    {    
#if !defined USE_NO_TANK_COMB_DEVICE 
#define SIMPLE_PROJECT
#endif

#ifdef SIMPLE_PROJECT
    if (	( strcmp( name, "CMD" ) == 0 ) 
        ||	( strcmp( name, "Cmd" ) == 0 ) 
        ||	( strcmp( name, "cmd" ) == 0 ) )
        {
 
        switch ( owner_type )
            {
            case T_TECH_OBJECT:
                u_int_4 new_mode = 0;
                memcpy( &new_mode, buff + sizeof( u_int_4 ), sizeof( new_mode ) );
                int new_state = 0;
                if ( 0 == new_mode )
                    {
                    state[ 0 ] = 0;
                    return 0;
                    }

                if ( new_mode >= 1000 && new_mode < 2000 )      // On mode.
                    {
                    new_mode -= 1000;
                    new_state = 1;
                    }
                else
                    {
                    if ( new_mode >= 2000 && new_mode < 3000 )  // Off mode.
                        {
                        new_mode -= 2000;
                        new_state = 0;
                        }
                    else
                        {
#ifdef DEBUG
                        Print( "Error complex_state::parse_cmd - new_mode = %lu\n",
                            ( unsigned long int ) new_mode );
#endif // DEBUG
                        return -1;
                        }
                    }

                if ( new_mode >
                    ( ( tech_object* ) owner_object )->get_modes_count() )
                    {
                    // Command.
                    state[ 0 ] = ( ( tech_object* ) owner_object )->exec_cmd(
                    new_mode );
                    }
                else
                    {
                    // On/off mode.
                    state[ 0 ] = ( ( tech_object* ) owner_object )->set_mode(
                        new_mode, new_state );
                    }
                break;
            }

        return 0; 
        }	
    else // if ( strcmp( name, "CMD" ) != 0 )
        {
        return -1; //��������� ������ ���� ������.	
        }
#else // SIMPLE_PROJECT

    return 0;
#endif // SIMPLE_PROJECT    
    } 
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
string_device::string_device( u_int_4 n, const char *new_name,
                             char* str, int max_str_len ): n( n ),
                             str( str ),
                             max_str_len( max_str_len )
    {
    name = new char[ strlen( new_name ) + 1 ];
    strcpy( name, new_name );
    }
//-----------------------------------------------------------------------------
// ������ ������ (buff) � ��������� ����:
//    1 ����  - ���;                                  (1)
//    4 ����� - �����;                                (2)
//    1 ����  - ����� ����� ������ ����������;        (3)
//    � ����  - ��� ������ ����������;                (4)
//    4 ����� - ���������� ������������;              (5)
int string_device::save_device( char *buff )
    {
    u_int_2 idx = 0;

    buff[ idx++ ] = complex_device::ARRAY_DEV_STR;    //(1)                            
    ( ( u_int_4* ) ( buff + idx ) )[ 0 ] = n;         //(2)
    idx += 4;
    buff[ idx++ ] = strlen( name );                   //(3)              
    strcpy( buff + idx, name );                       //(4)
    idx += strlen( name ) + 1;
    ( ( u_int_4* ) ( buff + idx ) )[ 0 ] = 1;         //(5)
    idx += 4;           

    return idx;
    }
//-----------------------------------------------------------------------------
// ������ ������ (buff) � ��������� ����:
//  4 ����� - ����� ����������;                       (1)
//  4 ����� - ���������� ������������;                (2)
//  �����   - ������ ������� �������������.
int string_device::save_state( char *buff )
    {
    ( ( u_int_4* ) buff )[ 0 ] = n;         //(1)
    ( ( u_int_4* ) buff )[ 1 ] = 1;	        //(2)
    u_int_2 answer_size = 8;

    int str_len = strlen( str );
#ifdef DEBUG
    if ( str_len > 200 )
        {
        Print( "string_device::save_state(...) - strlen [ %d ] > 200!\n", 
            str_len );
        get_char();
        }
#endif // DEBUG

    strcpy( buff + answer_size, str );

    
    answer_size += str_len + 1;      

    return answer_size;
    }
//-----------------------------------------------------------------------------
int string_device::save_changed_state( char *buff )
    {
    return save_state( buff );
    }
//-----------------------------------------------------------------------------
void string_device::print() const
    {
    char tmp_str[ 100 ];    
    sprintf( tmp_str, "\"%s\", \t[ %3lu ]", name, 1UL );

    print_str( tmp_str );
    }
//-----------------------------------------------------------------------------
u_int_4 string_device::get_n() const
    {
    return 0; 
    }
//-----------------------------------------------------------------------------
int string_device::parse_cmd( char *buff )
    {
	buff += 4; //������� ������ �������
    int new_str_len = strlen( buff );
    if ( new_str_len > max_str_len - 1 )
    	{
#ifdef DEBUG
        Print( "string_device::parse_cmd(...) - str_len[ %d ] > max_str_len[ %d ]!\n",
            new_str_len, max_str_len );
        get_char();
#endif // DEBUG
    	}
    else
        {
        strcpy( str, buff );
        }

    return new_str_len + 1;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
device::device() : number( 0 ),
    type( DT_NONE ),
    sub_type( DST_NONE )
    { 
    }
//-----------------------------------------------------------------------------
int device::save_device( char *buff )
    {
    memcpy( buff, &number, sizeof( number ) );
    return sizeof( number );
    }
//-----------------------------------------------------------------------------
void device::print() const
    {
#ifdef DEBUG    
    switch ( type )
        {
        case DT_V:
            Print( "V  " );
            break;

        case DT_N:
            Print( "N  " );
            break;

        case DT_M:
            Print( "M  " );
            break;

        case DT_LS:
            Print( "LS " );
            break;

        case DT_TE:
            Print( "TE " );
            break;

        case DT_FE:
            Print( "FE " );
            break;

        case DT_FS:
            Print( "FS " );
            break;

        case DT_CTR:
            Print( "CTR" );
            break;

        case DT_AO:
            Print( "AO " );
            break;

        case DT_LE:
            Print( "LE " );
            break;

        case DT_FB:
            Print( "FB " );
            break;

        case DT_UPR:
            Print( "UPR" );
            break;

        case DT_QE:
            Print( "QE " );
            break;

        case DT_AI:
            Print( "AI " );
            break;

        default:
            Print( "Uknown" );
            break;
        }
    Print( "%5lu\t", ( u_long ) number );

#endif // DEBUG
    }
//-----------------------------------------------------------------------------
int device::load( file *cfg_file )
    {    
    sscanf( cfg_file->fget_line(), "%u %u %u", ( u_int* ) &type,
        ( u_int* ) &sub_type, &number );

    return 0;
    }
//-----------------------------------------------------------------------------
u_int_4 device::get_n() const
    {
    return number;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int char_state_device::save_changed_state( char *buff )
    {
    if ( prev_state != get_state() )
        {
        return save_state( buff );
        }
    return 0;
    }
//-----------------------------------------------------------------------------
int char_state_device::save_state( char *buff )
    {
    buff[ 0 ] = get_state();
    prev_state = get_state();
    return sizeof( char );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int u_int_4_state_device::save_changed_state( char *buff )
    {
    if ( prev_state != get_u_int_4_state() )
        {
        return save_state( buff );
        }
    return 0;
    }
//-----------------------------------------------------------------------------
int u_int_4_state_device::save_state( char *buff )
    {
    prev_state = ( u_int_4 ) get_u_int_4_state();
    memcpy( buff, &prev_state, sizeof( prev_state ) );
    return sizeof( prev_state );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

int DO_1::get_state_now()
    {
    return get_DO( DO_INDEX );
    }
//-----------------------------------------------------------------------------
void DO_1::on()
    {
    set_DO( DO_INDEX, 1 );
    }
//-----------------------------------------------------------------------------
void DO_1::off()
    {
    set_DO( DO_INDEX, 0 );
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
i_DO_device* device_manager::get_V( int number )
    {
    return get_device( device::DT_V, number, "V" );
    }
//-----------------------------------------------------------------------------
device_manager* device_manager::get_instance()
    {
    return instance;
    }
//-----------------------------------------------------------------------------
void device_manager::set_instance( device_manager* new_instance )
    {
    instance = new_instance;
    }
//-----------------------------------------------------------------------------
int device_manager::get_device_n( device::DEVICE_TYPE dev_type, 
    u_int dev_number )
    {
    int l = dev_types_ranges[ dev_type ].start_pos;
    int u = dev_types_ranges[ dev_type ].end_pos;

    if ( -1 == l ) return -1; // ��� ���������.

    while ( l <= u ) 
        {
        int i = ( l + u ) / 2;

        if ( dev_number == project_devices[ i ]->get_n() ) 
            {
            return i;
            }
        
        if ( dev_number > project_devices[ i ]->get_n() )
            {
            l = i + 1;
            } 
        else 
            {
            u = i - 1;
            }        
        }
    
    return -1;
    }
//-----------------------------------------------------------------------------
device* device_manager::get_device( device::DEVICE_TYPE dev_type,
        u_int dev_number, char const * dev_name )
    {
    int dev_n = get_device_n( dev_type, dev_number );

    if ( dev_n >= 0 )
        {
        return project_devices[ dev_n ];
        }
    else
        {
#ifdef DEBUG
    Print( "%s[ %d ] not found!\n", dev_name, dev_number );
#endif // DEBUG
        }

    return &stub;
    }
//-----------------------------------------------------------------------------
int device_manager::load_from_cfg_file( file *cfg_file )
    {
    cfg_file->fget_line();                      // ���������� ���������.
    sscanf( cfg_file->fget_line(), "%d", &devices_count );
    cfg_file->fget_line();                      // ���������� ������ ������.

#ifdef DEBUG
    Print( "Total devices count %d.\n", devices_count );
#endif // DEBUG

    if ( devices_count )
        {
        char is_first_device[ device::C_DEVICE_TYPE_CNT ] = { 0 };

        project_devices = new device* [ devices_count ];

        for ( int i = 0; i < devices_count; i++ )
            {
            int dev_type = 0;
            int dev_sub_type = 0;
            cfg_file->fget_line();              // ���������� �����������.
            sscanf( cfg_file->pfget_line(), "%d %d", &dev_type, &dev_sub_type );
            
            switch ( dev_type )
                {                
                case device::DT_V:
                    {
                    switch ( dev_sub_type )
                        {
                        case device::DST_V_DO_1:
                            project_devices[ i ] = new valve_DO_1();
                            break;

                        case device::DST_V_DO_2:
                            project_devices[ i ] = new valve_DO_2();
                            break;

                        case device::DST_V_DO_1_DI_1:
                            project_devices[ i ] = new valve_DO_1_DI_1();
                            break;

                        case device::DST_V_DO_1_DI_2:
                            project_devices[ i ] = new valve_DO_1_DI_2();
                            break;

                        case device::DST_V_DO_2_DI_2:
                            project_devices[ i ] = new valve_DO_2_DI_2();
                            break;

                        case device::DST_V_MIXPROOF:
                            project_devices[ i ] = new valve_mix_proof();
                            break;

                        default:
#ifdef DEBUG
                            Print( "Unknown V device subtype %d!\n", dev_sub_type );
                            get_char();
#endif // DEBUG
                            project_devices[ i ] = new dev_stub();
                            break;
                        }
                    break;
                    }

                case device::DT_N:
                    project_devices[ i ] = new pump();
                    break;

                case device::DT_M:
                    project_devices[ i ] = new mixer();
                    break;

                case device::DT_LS:
                    project_devices[ i ] = new level_s();
                    break;

                case device::DT_TE:
                    project_devices[ i ] = new temperature_e();
                    break;

                case device::DT_FE:
                    project_devices[ i ] = new flow_e();
                    break;

                case device::DT_FS:                    
                    project_devices[ i ] = new flow_s();
                    break;

                case device::DT_CTR:
                    project_devices[ i ] = new counter();
                    break;

                case device::DT_AO:
                    project_devices[ i ] = new AO_0_100();
                    break;

                case device::DT_LE:
                    project_devices[ i ] = new level_e();
                    break;

                case device::DT_FB:
                    project_devices[ i ] = new feedback();
                    break;

                case device::DT_UPR:
                    project_devices[ i ] = new control_s();
                    break;

                case device::DT_QE:
                    project_devices[ i ] = new concentration_e();
                    break;

                case device::DT_AI:
                    project_devices[ i ] = new analog_input_4_20();
                    break;

                default:
#ifdef DEBUG
                    Print( "Unknown device type %d!\n", dev_type );
#endif // DEBUG
                    project_devices[ i ] = new dev_stub();
                    break;
                }

            if ( dev_type < device::C_DEVICE_TYPE_CNT )
                {
                if ( 0 == is_first_device[ dev_type ] )
                    {
                    dev_types_ranges[ dev_type ].start_pos = i;
                    is_first_device[ dev_type ] = 1;
                    }
                dev_types_ranges[ dev_type ].end_pos = i;
                }

            project_devices[ i ]->load( cfg_file );
            }
        }

        devices = new complex_device( 0, "GLB", device::C_DEVICE_TYPE_CNT, 0 );

        for ( int i = 0; i < device::C_DEVICE_TYPE_CNT; i++ )
            {
            int dev_cnt = dev_types_ranges[ i ].end_pos -
                dev_types_ranges[ i ].start_pos + 1;

            if ( dev_types_ranges[ i ].start_pos == -1 )
                {
                dev_cnt = 0;
                }

            devices->sub_dev[ i ] =
                new complex_device( 0, device::DEV_NAMES[ i ], dev_cnt,
                device::DEV_TYPES[ i ] );

            if ( dev_cnt )
                {
                int pos = 0;
                for ( int j = dev_types_ranges[ i ].start_pos;
                    j <= dev_types_ranges[ i ].end_pos; j++ )
                    {
                    ( ( complex_device* ) ( devices->sub_dev[ i ] ) )->sub_dev[ pos++ ] =
                        project_devices[ j ];
                    }
                }
            }

    return 0;
    }
//-----------------------------------------------------------------------------
void device_manager::print() const
    {
    for ( int i = 0; i < devices_count; i++ )
        {
        Print( "    " );
        project_devices[ i ]->print();
        }
    Print( "\n" );
    }
//-----------------------------------------------------------------------------
device_manager::device_manager():project_devices( 0 )
    {
    for ( int i = 0; i < device::C_DEVICE_TYPE_CNT; i++ )
        {
        dev_types_ranges[ i ].start_pos = -1;
        dev_types_ranges[ i ].end_pos = -1;
        }
    }
//-----------------------------------------------------------------------------
device_manager::~device_manager()
    {
    if ( project_devices )
        {
        for ( int i = 0; i < devices_count; i++ )
            {
            delete project_devices[ i ];
            }
        delete [] project_devices;
        project_devices = 0;        
        }

   for ( int i = 0; i < device::C_DEVICE_TYPE_CNT; i++ )
        {
        if ( devices->sub_dev[ i ] )
            {
            delete devices->sub_dev[ i ];
            devices->sub_dev[ i ] = 0;
            }
        }

    delete devices;
    devices = 0;
    }
//-----------------------------------------------------------------------------
i_DO_device* device_manager::get_N( int number )
    {
    return get_device( device::DT_N, number, "N" );
    }
//-----------------------------------------------------------------------------
i_DO_device* device_manager::get_M( int number )
    {
    return get_device( device::DT_M, number, "M" );
    }
//-----------------------------------------------------------------------------
i_DI_device* device_manager::get_LS( int number )
    {
    return get_device( device::DT_LS, number, "LS" );
    }
//-----------------------------------------------------------------------------
i_DI_device* device_manager::get_FS( int number )
    {
    return get_device( device::DT_FS, number, "FS" );
    }
//-----------------------------------------------------------------------------
i_AI_device* device_manager::get_AI( int number )
    {
    return get_device( device::DT_AI, number, "AI" );
    }
//-----------------------------------------------------------------------------
i_AO_device* device_manager::get_AO( int number )
    {
    return get_device( device::DT_AO, number, "AO" );
    }
//-----------------------------------------------------------------------------
i_counter* device_manager::get_CTR( int number )
    {
    int res = get_device_n( device::DT_CTR, number );

    if ( res >= 0 ) return ( counter* ) project_devices[ res ];

    return &stub;
    }
//-----------------------------------------------------------------------------
i_AI_device* device_manager::get_TE( int number )
    {
    return get_device( device::DT_TE, number, "TE" );
    }
//-----------------------------------------------------------------------------
i_AI_device* device_manager::get_FE( int number )
    {
    return get_device( device::DT_FE, number, "FE" );
    }
//-----------------------------------------------------------------------------
i_AI_device* device_manager::get_LE( int number )
    {
    return get_device( device::DT_LE, number, "LE" );
    }
//-----------------------------------------------------------------------------
i_DI_device* device_manager::get_FB( int number )
    {
    return get_device( device::DT_FB, number, "FB" );
    }
//-----------------------------------------------------------------------------
i_DO_device* device_manager::get_UPR( int number )
    {
    return get_device( device::DT_UPR, number, "UPR" );
    }
//-----------------------------------------------------------------------------
i_AI_device* device_manager::get_QE( int number )
    {
    return get_device( device::DT_QE, number, "QE" );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
i_DI_device::i_DI_device() :last_check_time( get_millisec() ),
    state( 0 ),
    dt( 0 )
    {
    }
//-----------------------------------------------------------------------------
void i_DI_device::set_dt( u_int time )
    {
    dt = time;
    }
//-----------------------------------------------------------------------------
void i_DI_device::set_st_state( int new_state )
    {
    state = new_state;
    }
//-----------------------------------------------------------------------------
int i_DI_device::get_state()
    {
    if ( dt > 0 )
        {
        if ( state != get_state_now() )
            {
            if ( get_delta_millisec( last_check_time ) > dt  )
                {
                state = get_state_now();
                }
            }
        else
            {
            last_check_time = get_millisec();
            }
        }
    else state = get_state_now();

    return state;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float dev_stub::get_value()
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int dev_stub::set_value( float new_value )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int dev_stub::get_state_now()
    {
    return 0;
    }
//-----------------------------------------------------------------------------
void dev_stub::on()
    {
    }
//-----------------------------------------------------------------------------
void dev_stub::off()
    {
    }
//-----------------------------------------------------------------------------
int dev_stub::set_state( int new_state )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int dev_stub::parse_cmd( char *buff )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int dev_stub::load( file *cfg_file )
    {
    return device::load( cfg_file );    
    }
//-----------------------------------------------------------------------------
int dev_stub::save_state( char *buff )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int dev_stub::save_changed_state( char *buff )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int dev_stub::save_device( char *buff )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
u_int_4 dev_stub::get_n() const
    {
    return 0;
    }
//-----------------------------------------------------------------------------
void dev_stub::print() const
    {
#ifdef DEBUG
    Print( "virtual device" );
#endif // DEBUG
    }
//-----------------------------------------------------------------------------
void dev_stub::pause()
    {
    }
//-----------------------------------------------------------------------------
void dev_stub::start()
    {
    }
//-----------------------------------------------------------------------------
void dev_stub::reset()
    {
    }
//-----------------------------------------------------------------------------
u_int dev_stub::get_quantity()
    {
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
counter::counter() : value( 0 ), last_read_value( 0 ), state( S_WORK )
    {
    }
//-----------------------------------------------------------------------------
float counter::get_value()
    {
    return get_quantity();
    }
//-----------------------------------------------------------------------------
int counter::set_value( float new_value )
    {
    value = ( u_int ) new_value;

    return 0;
    }
//-----------------------------------------------------------------------------
int counter::get_state_now()
    {
    return state;
    }
//-----------------------------------------------------------------------------
void counter::on()
    {
    start();
    }
//-----------------------------------------------------------------------------
void counter::off()
    {
    reset();
    }
//-----------------------------------------------------------------------------
int counter::set_state( int new_state )
    {
    switch ( new_state )
        {
        case S_STOP:
            reset();
            break;

        case S_WORK:
            start();
            break;

        case S_PAUSE:
            pause();
            break;
        }

    return 0;
    }
//-----------------------------------------------------------------------------
int counter::parse_cmd( char *buff )
    {
    memcpy( &value, buff, sizeof( value ) );

    return sizeof( value );
    }
//-----------------------------------------------------------------------------
int counter::load( file *cfg_file )
    {
    device::load( cfg_file );
    wago_device::load( cfg_file );

    return 0;
    }
//-----------------------------------------------------------------------------
void counter::print() const
    {
    device::print();
    wago_device::print();
    }
//-----------------------------------------------------------------------------
u_int_4 counter::get_u_int_4_state()
    {
    return get_quantity();
    }
//-----------------------------------------------------------------------------
int counter::save_changed_state( char *buff )
    {
    return u_int_4_state_device::save_changed_state( buff );
    }
//-----------------------------------------------------------------------------
int counter::save_state( char *buff )
    {
    return u_int_4_state_device::save_state( buff );
    }
//-----------------------------------------------------------------------------
void counter::pause()
    {
    get_quantity(); // ������������� �������� ��������.

    state = S_PAUSE;
    }
//-----------------------------------------------------------------------------
void counter::start()
    {
    if ( S_STOP == state || S_PAUSE == state )
        {
        if ( S_STOP == state )
            {
            value = 0;
            }
        
        state = S_WORK;
        last_read_value = ( u_int ) get_AI( AI_INDEX );
        }
    }
//-----------------------------------------------------------------------------
void counter::reset()
    {
    state = S_STOP;
    value = 0;
    }
//-----------------------------------------------------------------------------
u_int counter::get_quantity()
    {
    if ( S_WORK == state )
        {
        u_int delta;
        u_int current = ( u_int ) get_AI( AI_INDEX );

        if ( current < last_read_value )
            {
            delta = MAX_VAL - last_read_value + current;
            }
        else
            {
            delta = current - last_read_value;
            }
            if ( delta > 0 )
                {
                value += delta;
                last_read_value = current;
                }
        }

    return value;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float digital_device::get_value()
    {
    return get_state_now();
    }
//-----------------------------------------------------------------------------
int digital_device::set_value( float new_value )
    {
    return set_state( ( int ) new_value );
    }
//-----------------------------------------------------------------------------
int digital_device::set_state( int new_state )
    {
    if ( new_state ) on();
    else off();

    return 0;
    }
//-----------------------------------------------------------------------------
int digital_device::parse_cmd( char *buff )
    {
    set_state( buff[ 0 ] );
    return sizeof( char );
    }
//-----------------------------------------------------------------------------
int digital_device::load( file *cfg_file )
    {
    device::load( cfg_file );
    wago_device::load( cfg_file );

    return 0;
    }
//-----------------------------------------------------------------------------
void digital_device::print() const
    {
    device::print();
    wago_device::print();
    }
//-----------------------------------------------------------------------------
int digital_device::save_changed_state( char *buff )
    {
    return char_state_device::save_changed_state( buff );
    }
//-----------------------------------------------------------------------------
int digital_device::save_state( char *buff )
    {
    return char_state_device::save_state( buff );
    }
//-----------------------------------------------------------------------------
int digital_device::get_state()
    {
    return i_DI_device::get_state();
    }
//-----------------------------------------------------------------------------
#ifdef DEBUG_NO_WAGO_MODULES

int digital_device::get_state_now()
    {
    return state;
    }
//-----------------------------------------------------------------------------
void digital_device::on()
    {
    state = 1;
    }
//-----------------------------------------------------------------------------
void digital_device::off()
    {
    state = 0;
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

int DO_2::get_state_now()
    {
    int b1 = get_DO( DO_INDEX_1 );
    int b2 = get_DO( DO_INDEX_2 );
    if ( b1 == b2 ) return -1;
    return b2;
    }
//-----------------------------------------------------------------------------
void DO_2::on()
    {
    set_DO( DO_INDEX_1, 0 );
    set_DO( DO_INDEX_2, 1 );
    }
//-----------------------------------------------------------------------------
void DO_2::off()
    {
    set_DO( DO_INDEX_1, 1 );
    set_DO( DO_INDEX_2, 0 );
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

int DO_1_DI_1::get_state_now()
    {
    int o = get_DO( DO_INDEX );
    int i = get_DI( DI_INDEX );

    if ( get_par( PAR_FB_STATE ) == 0 )
        {
        if ( ( o == 0 && i == 1 ) || ( o == 1 && i == 0 ) )
            {
            start_switch_time = get_sec();
            return o;
            }
        }
    else
        {
        if ( o == i )
            {
            start_switch_time = get_sec();
            return i;
            }
        }

    if ( get_sec() - start_switch_time > C_SWITCH_TIME )
        {
        return -1;
        }
    else
        {
        if ( get_par( PAR_FB_STATE ) == 0 ) return !i;
        else return i;
        }
    }
//-----------------------------------------------------------------------------
void DO_1_DI_1::on()
    {
    int o = get_DO( DO_INDEX );
    if ( 0 == o )
        {
        start_switch_time = get_sec();
        set_DO( DO_INDEX, 1 );
        }
    }
//-----------------------------------------------------------------------------
void DO_1_DI_1::off()
    {
    int o = get_DO( DO_INDEX );
    if ( o != 0 )
        {
        start_switch_time = get_sec();
        set_DO( DO_INDEX, 0 );
        }
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

int DO_1_DI_2::get_state_now()
    {
    int o = get_DO( DO_INDEX );
    int i0 = get_DI( DI_INDEX_1 );
    int i1 = get_DI( DI_INDEX_2 );

    if ( ( o == 0 && i0 == 1 && i1 == 0 ) ||
        ( o == 1 && i1 == 1 && i0 ==0 ) )
        {
        start_switch_time = get_sec();
        return o;
        }

    if ( get_sec() - start_switch_time > C_SWITCH_TIME )
        {
        return -1;
        }
    else
        {
        return o;
        }
    }
//-----------------------------------------------------------------------------
void DO_1_DI_2::on()
    {
    int o = get_DO( DO_INDEX );
    if ( 0 == o )
        {
        start_switch_time = get_sec();
        set_DO( DO_INDEX, 1 );
        }
    }
//-----------------------------------------------------------------------------
void DO_1_DI_2::off()
    {
    int o = get_DO( DO_INDEX );
    if ( o != 0 )
        {
        start_switch_time = get_sec();
        set_DO( DO_INDEX, 0 );
        }
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

int DO_2_DI_2::get_state_now()
    {
    int o0 = get_DO( DO_INDEX_1 );
    int o1 = get_DO( DO_INDEX_2 );
    int i0 = get_DI( DI_INDEX_1 );
    int i1 = get_DI( DI_INDEX_2 );

    if ( ( o1 == i1 ) && ( o0 == i0 ) )
        {
        start_switch_time = get_sec();
        return o1;
        };

    if ( get_sec() - start_switch_time > C_SWITCH_TIME )
        {
        return -1;
        }
    else
        {
        return o1;
        }
    }
//-----------------------------------------------------------------------------
void DO_2_DI_2::on()
    {
    int o = get_DO( DO_INDEX_1 );
    if ( 0 == o )
        {
        start_switch_time = get_sec();
        set_DO( DO_INDEX_1, 1 );
        set_DO( DO_INDEX_2, 0 );
        }
    }
//-----------------------------------------------------------------------------
void DO_2_DI_2::off()
    {
    int o = get_DO( DO_INDEX_2 );
    if ( 0 == o )
        {
        start_switch_time = get_sec();
        set_DO( DO_INDEX_1, 0 );
        set_DO( DO_INDEX_2, 1 );
        }
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void valve_mix_proof::open_upper_seat()
    {
    set_state( ST_UPPER_SEAT );
    }
//-----------------------------------------------------------------------------
void valve_mix_proof::open_low_seat()
    {
    set_state( ST_LOW_SEAT );
    }
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

int valve_mix_proof::get_state_now()
    {
    int o = get_DO( DO_INDEX );            
    int i0 = get_DI( DI_INDEX_U );
    int i1 = get_DI( DI_INDEX_L );

    if ( ( o == 0 && i0 == 1 && i1 == 0 ) ||
        ( o == 1 && i1 == 1 && i0 == 0 ) )
        {
        start_switch_time = get_sec();
        if ( o == 0 && get_DO( DO_INDEX_U ) == 1 ) return ST_UPPER_SEAT;
        if ( o == 0 && get_DO( DO_INDEX_L ) == 1 ) return ST_LOW_SEAT;
        return o;
        }

    if ( get_sec() - start_switch_time > C_SWITCH_TIME )
        {
        return -1;
        }
    else
        {
        return o;
        }
    }
//-----------------------------------------------------------------------------
void valve_mix_proof::on()
    {
    set_DO( DO_INDEX_U, 0 );
    set_DO( DO_INDEX_L, 0 );
    int o = get_DO( DO_INDEX );

    if ( 0 == o )
        {
        start_switch_time = get_sec();
        set_DO( DO_INDEX, 1 );
        }
    }
//-----------------------------------------------------------------------------
void valve_mix_proof::off()
    {
    set_DO( DO_INDEX_U, 0 );
    set_DO( DO_INDEX_L, 0 );
    int o = get_DO( DO_INDEX );

    if ( o != 0 )
        {
        start_switch_time = get_sec();
        set_DO( DO_INDEX, 0 );
        }
    }
//-----------------------------------------------------------------------------
int valve_mix_proof::set_state( int new_state )
    {
    switch ( new_state )
        {
        case ST_CLOSE:
            off();
            break;

        case ST_OPEN:
            on();
            break;

        case ST_UPPER_SEAT:
            off();
            set_DO( DO_INDEX_U, 1 );
            break;

        case ST_LOW_SEAT:
            off();
            set_DO( DO_INDEX_L, 1 );
            break;

        default:
            on();
            break;
        }

    return 0;
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

DI_1::DI_1( u_int dt )
    {
    set_dt( dt );
    }
//-----------------------------------------------------------------------------
int DI_1::get_state_now()
    {
    return get_DI( DI_INDEX );
    }
//-----------------------------------------------------------------------------
void DI_1::on()
    {
    }
//-----------------------------------------------------------------------------
void DI_1::off()
    {
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int float_state_device::save_changed_state( char *buff )
    {
    if ( prev_state != get_value() )
        {
        return save_state( buff );
        }
    return 0;
    }
//-----------------------------------------------------------------------------
int float_state_device::save_state( char *buff )
    {
    prev_state = get_value();
    memcpy( buff, &prev_state, sizeof( prev_state ) );
    return sizeof( prev_state );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

float AI_1::get_value()
    {
    return get_AI( AI_INDEX, get_min_val(), get_max_val() );
    }
//-----------------------------------------------------------------------------
int AI_1::set_value( float new_value )
    {
    return 0;
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
#ifndef DEBUG_NO_WAGO_MODULES

float AO_1::get_value()
    {
    return get_AO( AO_INDEX, get_min_val(), get_max_val() );
    }
//-----------------------------------------------------------------------------
int AO_1::set_value( float new_value )
    {
    return set_AO( AO_INDEX, new_value, get_min_val(), get_max_val() );
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float temperature_e::get_max_val()
    {
    return 0;
    }
//-----------------------------------------------------------------------------
float temperature_e::get_min_val()
    {
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float level_e::get_max_val()
    {
    return 100;
    }
//-----------------------------------------------------------------------------
float level_e::get_min_val()
    {
    return 0;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float flow_e::get_max_val()
    {
    return get_par( C_MAX_PAR_NUMBER );
    }
//-----------------------------------------------------------------------------
float flow_e::get_min_val()
    {
    return get_par( C_MIN_PAR_NUMBER );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float concentration_e::get_max_val()
    {
    return get_par( C_MAX_PAR_NUMBER );
    }
//-----------------------------------------------------------------------------
float concentration_e::get_min_val()
    {
    return get_par( C_MIN_PAR_NUMBER );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float analog_input_4_20::get_max_val()
    {
    return 20;
    }
//-----------------------------------------------------------------------------
float analog_input_4_20::get_min_val()
    {
    return 4;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int analog_device::set_state( int new_state )
    {
    return set_value( new_state );
    }
//-----------------------------------------------------------------------------
int analog_device::get_state_now()
    {
    return ( int ) get_value();
    }
//-----------------------------------------------------------------------------
int analog_device::parse_cmd( char *buff )
    {
    float val;
    memcpy( &val, buff, sizeof( float ) );

    Print( "val=%f\n", val );

    set_value( val );
    return sizeof( float );
    }
//-----------------------------------------------------------------------------
int analog_device::load( file *cfg_file )
    {
    device::load( cfg_file );
    wago_device::load( cfg_file );

    return 0;
    }
//-----------------------------------------------------------------------------
void analog_device::print() const
    {
    device::print();
    wago_device::print();
    }
//-----------------------------------------------------------------------------
void analog_device::on()
    {
    }
//-----------------------------------------------------------------------------
void analog_device::off()
    {
    set_value( 0 );
    }
//-----------------------------------------------------------------------------
#ifdef DEBUG_NO_WAGO_MODULES

float analog_device::get_value()
    {
    return value;
    }
//-----------------------------------------------------------------------------
int analog_device::set_value( float new_value )
    {
    value = new_value;
    return 0;
    }

#endif // DEBUG_NO_WAGO_MODULES
//-----------------------------------------------------------------------------
int analog_device::save_changed_state( char *buff )
    {
    return float_state_device::save_changed_state( buff );
    }
//-----------------------------------------------------------------------------
int analog_device::save_state( char *buff )
    {
    return float_state_device::save_state( buff );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float AO_0_100::get_max_val()
    {
    return C_AO_MIN_VALUE;
    }
//-----------------------------------------------------------------------------
float AO_0_100::get_min_val()
    {
    return C_AO_MAX_VALUE;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
int timer::save( char *buff )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int timer::load( char *buff )
    {
    return 0;
    }
//-----------------------------------------------------------------------------
int timer::get_saved_size() const
    {
    return 0;
    }
//-----------------------------------------------------------------------------
timer::timer(): last_time( 0 ),
    work_time( 0 ),
    state( S_STOP ),
    countdown_time( 0 )
    {
    }
//-----------------------------------------------------------------------------
void timer::start()
    {
    if ( S_STOP == state )
        {        
        work_time = 0;
        }

    if ( S_PAUSE == state || S_STOP == state )
        {
        state = S_WORK;
        last_time = get_millisec();
        }
    }
//-----------------------------------------------------------------------------
void timer::reset()
    {
    state = S_STOP;
    work_time = 0;
    }
//-----------------------------------------------------------------------------
void timer::pause()
    {
    if ( S_WORK == state )
        {
        work_time += get_delta_millisec( last_time );
        }
    state = S_STOP;
    }
//-----------------------------------------------------------------------------
bool timer::is_time_up() const
    {
    if ( state != S_STOP )
        {
        u_int time = work_time + get_delta_millisec( last_time );
        if (  time <= countdown_time )
            {
            return 0;
            }
        else
            {
            return 1;
            }
        }

    return 0;
    }
//-----------------------------------------------------------------------------
u_long timer::get_work_time() const
    {
    return work_time + get_delta_millisec( last_time );
    }
//-----------------------------------------------------------------------------
void timer::set_countdown_time( u_long new_countdown_time )
    {
#ifdef DEBUG
    if ( 0 == new_countdown_time )
        {
        Print( "Error void timer::set_countdown_time( u_long time ), time = %lu!\n",
                new_countdown_time );
        }
#endif

    countdown_time = new_countdown_time;
    }
//-----------------------------------------------------------------------------
u_long timer::get_countdown_time() const
    {
    return countdown_time;
    }
//-----------------------------------------------------------------------------
timer::STATE timer::get_state() const
    {
    return state;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
level_s::level_s( u_int dt /*= 1000 */ )
    {
    set_dt( dt );
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
timer_manager::timer_manager( u_int timers_count ) : timers_cnt( timers_count ),
    timers( 0 )
    {
    if ( timers_cnt )
        {
        timers = new timer[ timers_cnt ];
        }
    }
//-----------------------------------------------------------------------------
timer_manager::~timer_manager()
    {
    if ( timers )
        {
        delete [] timers;
        timers     = 0;
        timers_cnt = 0;
        }
    }
//-----------------------------------------------------------------------------
timer& timer_manager::operator[]( unsigned int index )
    {
    if ( index < timers_cnt )
        {
        return timers[ index ];
        }
#ifdef DEBUG
    else
        {
        Print( "timer_manager[] - error: index[ %u ] > count [ %u ]\n",
            index, timers_cnt );
        }
#endif // DEBUG

    return stub;
    }
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
